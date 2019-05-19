#include <map>
#include <mutex>
#include <queue>
#include <tuple>

#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

#include <errorhandling.hpp>
#include <eventloop.hpp>
#include <logging.hpp>


LOG_MODULE("eventloop")


struct eventloop_t::event_t {
	std::function<void()> func;
	std::chrono::time_point<std::chrono::steady_clock> time;
};

bool eventloop_t::event_cmp::operator ()(
		const event_t &e1, const event_t &e2) const noexcept {
    return e1.time > e2.time;
}


eventloop_t::eventloop_t()
{
	int ret;
	// Create self-pipe
	ret = pipe(m_self_pipe);
	if (ret != 0)
        OSERROR(pipe, "Cannot create self-pipe");
	// Make self-pipe non-blocking
	ret = fcntl(m_self_pipe[0], F_GETFL);
	if (ret < 0 || fcntl(m_self_pipe[0], F_SETFL, ret | O_NONBLOCK) < 0)
        OSERROR(fcntl, "Cannot make self-pipe non-blocking");
	ret = fcntl(m_self_pipe[1], F_GETFL);
	if (ret < 0 || fcntl(m_self_pipe[1], F_SETFL, ret | O_NONBLOCK) < 0)
        OSERROR(fcntl, "Cannot make self-pipe non-blocking");
}

eventloop_t::~eventloop_t() noexcept
{
	while (close(m_self_pipe[0]) < 0) {
		if (errno != EINTR) {
            LOG_WARN() << "Error occurred while closing self-pipe: "
                       << strerror(errno);
			break;
		}
	}
	while (close(m_self_pipe[1]) < 0) {
		if (errno != EINTR) {
            LOG_WARN() << "Error occurred while closing self-pipe: "
                       << strerror(errno);
			break;
		}
	}
}

/**
 * Adds event that is called after timeout.
 *
 * The method adds @p func to the eventloop that will be called when @p timeout
 * is exceeded. This function is thread-safe.
 *
 * @param func    Function to call.
 * @param timeout Time to wait before @p func is called. A timeout of zero means
 *                that the function is called within the next iteration.
 */
void eventloop_t::call(const std::function<void ()> &func,
                       const std::chrono::nanoseconds &timeout)
{
	event_t event = {func, std::chrono::steady_clock::now() + timeout};
	{
		std::lock_guard<std::mutex> m(m_events_mtx);
		m_events.push(std::move(event));
	}
	notify();
}

/**
 * Register handlers that are called on every iteration.
 *
 * @param handler Handler that will be called on every iteration.
 * @param getter  Handler that can be used to announce file descriptors.
 *
 * @return A handle that can be used to unregister the handlers registered by
 *         this call.
 *
 * @see select_handler_t     Additional information about @p handler.
 * @see select_fdgetter_t    Additional information about @p getter.
 * @see unregister_handler() Function to unregister handlers.
 */
eventloop_t::select_handle_t eventloop_t::register_handler(
		const eventloop_t::select_handler_t &handler,
		const eventloop_t::select_fdgetter_t &getter)
{
	select_handle_t handle;
	handle.m_handle = ++m_select_handle_max;
	m_select_funcs.emplace(handle.m_handle, std::make_tuple(handler, getter));

	notify();
	return handle;
}

/**
 * Remove handlers which has been registered with register_handler().
 *
 * @param handle The handle which has been returned from register_handler().
 */
void eventloop_t::unregister_handler(const eventloop_t::select_handle_t &handle)
{
	ASSERT(m_select_funcs.find(handle.m_handle) != m_select_funcs.end());
	m_select_funcs.erase(handle.m_handle);
}

/**
 * Run event loop. The function blocks until @p until returnes `true` or an
 * exception is thrown.
 *
 * @param until   The function will be checked from time to time. If it returns
 *                `true`, exec() returns to the caller. Note that when the
 *                condition changes from outside of the event loop, you must
 *                call notify() to ensure that the given function is called in a
 *                timely manner.
 * @param sigmask The given sigmask is applied while waiting for events. This
 *                is usually used to synchronise certain signals with event
 *                dispatching.
 */
void eventloop_t::exec(std::function<bool()> until, const sigset_t *sigmask)
{
    while (!until()) {
		auto now     = std::chrono::steady_clock::now();
		auto timeout = std::chrono::nanoseconds::max();

		// Run timed events and set timeout for future events
		while (true) {
			event_t e;
			{
				std::lock_guard<std::mutex> m(m_events_mtx);
				if (m_events.empty()) {
					break;
				} else if (m_events.top().time > now) {
					timeout = m_events.top().time - now;
					break;
				}
				e = std::move(m_events.top());
				m_events.pop();
			}
			e.func();
		}

		// Handle available IO or wait for timeout
		{
			int max;
			fd_set rs, ws, es;

			max = 0;
			FD_ZERO (&rs);
			FD_ZERO (&ws);
			FD_ZERO (&es);

			// Call registered select_fdgetter_t to set arguments for select().
			for (const auto &entry : m_select_funcs) {
				int m = 0;
				auto to = decltype(timeout)::max();

				std::get<1>(entry.second)(rs, ws, es, m, to);

				if (to < timeout)
					timeout = to;
				if (m > max)
					max = m;
			}

			// Announce self-pipe for select()
			FD_SET(m_self_pipe[0], &rs);
			max = std::max(m_self_pipe[0], max);

			// Call select
			{
				struct timespec tv;
				struct timespec *tvp;

				if (timeout == std::chrono::nanoseconds::max()) {
					tvp = nullptr;
				} else {
					tv.tv_sec  = timeout.count() / 1000000000;
					tv.tv_nsec = timeout.count() % 1000000000;
					tvp = &tv;
				}

                OSCHECK(pselect,(max + 1, &rs, &ws, &es, tvp, sigmask),
                        >= 0 || errno == EINTR);
			}

			// Call registered select_handler_t
			for (const auto &entry : m_select_funcs) {
				std::get<0>(entry.second)(rs, ws, es);
			}

			// Clear self-pipe
			if (FD_ISSET(m_self_pipe[0], &rs)) {
                ssize_t ret;
				char buf[32];

				do {
					ret = read(m_self_pipe[0], buf, sizeof(buf));
				} while (ret == sizeof(buf) || (ret < 0 && errno == EINTR));

				if (ret < 0 && (errno != EAGAIN || errno != EWOULDBLOCK)) {
                    OSERROR(read, "Cannot read from self-pipe");
				}
			}
		}
	}
}

/**
 * Wakes up the event loop.
 *
 * The event loop blocks when there is nothing to do. Calling the method wakes
 * up the event loop. If the method is invoked within the loop, it causes at
 * least one additional iteration before the loop is going back to sleep.
 */
void eventloop_t::notify()
{
    ssize_t ret;
	do {
		ret = write(m_self_pipe[1], "x", 1);
	} while (ret < 0 && errno == EINTR);

    if (ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        OSERROR(write, "Cannot write to self-pipe");
    }
}
