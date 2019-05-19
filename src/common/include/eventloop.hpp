#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

/**
 * @file eventloop.hpp
 * File contains class {@link eventloop_t} which handles event dispatching.
 */

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <tuple>
#include <vector>

#include <signal.h>

#include <boost/core/noncopyable.hpp>


/**
 * Eventloop.
 */
class eventloop_t : private boost::noncopyable
{
    //! Struct used internally by {@link eventloop_t}.
    struct event_t;
    //! Comparator used internally by {@link eventloop_t}.
    struct event_cmp {
        bool operator ()(const event_t &e1, const event_t &e2) const noexcept;
    };
public:
    /**
     * Handler that is called on every iteration.
     *
     * The handler can be registered with register_handler(). It is called on
     * every iteration after calling `select()`.
     *
     * @note It is not allowed to unregister the handler within this function.
     *
     * @param rs Second argument (*readfds*) as modified by `select()`.
     * @param ws Third argument (*writefds*) as modified by `select()`.
     * @param es Fourth argument (*exceptfds*) as modified by `select()`.
     */
    typedef std::function<
        void(const fd_set &rs, const fd_set &ws, const fd_set &es)>
    select_handler_t;

    /**
     * Handler that is called on every iteration to announce file descriptors.
     *
     * The handler can be registered with register_handler(). It is called on
     * every iteration before calling `select()`. It can be used to modify the
     * arguments for `select()`.
     *
     * @note It is not allowed to unregister the handler within this function.
     *
     * @param rs      Second argument (*readfds*) for `select()`.
     * @param ws      Third argument (*writefds*) for `select()`.
     * @param es      Fourth argument (*exceptfds*) for `select()`.
     * @param max     First argument (*nfds*) for `select()` minus 1. Reducing
     *                the value does not have an effect. It should be set to the
     *                highest-numbered file descriptor in any of the three sets.
     * @param timeout Can be set to specify the maximal amount of time the
     *                eventloop can block.
     */
    typedef std::function<
        void(fd_set &rs, fd_set &ws, fd_set &es, int &max,
             std::chrono::nanoseconds &timeout)
    > select_fdgetter_t;

    /**
     * Handle returned by register_handler().
     */
    class select_handle_t {
        friend class eventloop_t;
    public:
        bool operator <(const select_handle_t &other) const {
            return m_handle < other.m_handle;}
        bool operator >(const select_handle_t &other) const {
            return m_handle > other.m_handle;}
        bool operator <=(const select_handle_t &other) const {
            return m_handle <= other.m_handle;}
        bool operator >=(const select_handle_t &other) const {
            return m_handle >= other.m_handle;}
    private:
        std::uint64_t m_handle;
    };


    eventloop_t();
    ~eventloop_t() noexcept;

    void call(const std::function<void()> &func,
              const std::chrono::nanoseconds &timeout
              = std::chrono::nanoseconds::zero());

    select_handle_t register_handler(const select_handler_t &handler,
                                     const select_fdgetter_t &getter = nullptr);
    void unregister_handler(const select_handle_t &handle);

    void exec(std::function<bool()> until, const sigset_t *sigmask = nullptr);
    void notify();

private:
    std::priority_queue<event_t, std::vector<event_t>, event_cmp> m_events;
    std::mutex m_events_mtx;

    std::uint64_t m_select_handle_max = 0;
    std::map<
        std::uint64_t,
        std::tuple<select_handler_t,select_fdgetter_t>
    > m_select_funcs;

    int m_self_pipe[2];

};

#endif // EVENTLOOP_HPP
