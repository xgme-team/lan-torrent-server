#include <cstring>

#include <microhttpd.h>

#include <configuration.hpp>
#include <errorhandling.hpp>
#include <eventloop.hpp>
#include <httpd.hpp>
#include <logging.hpp>

LOG_MODULE("HttpServer")


struct connection_data_t {
    httpserver_t::access_handler_t access_handler;
};

static MHD_Response *response_404 = nullptr;
static MHD_Response *response_500 = nullptr;


static MHD_Response *create_static_response(const char *json)
{
    MHD_Response *r = MHD_create_response_from_buffer(strlen(json),
                                                      const_cast<char*>(json),
                                                      MHD_RESPMEM_PERSISTENT);

    OSCHECK(MHD_add_response_header,(r, "Content-type", "application/json"),
            != MHD_NO);
    return r;
}

static void init_static_responses()
{
    response_404 = create_static_response("{\"msg\":\"not found\"}");
    response_500 = create_static_response("{\"msg\":\"internal server error\"}");
}


httpserver_t::httpserver_t(eventloop_t *eventloop)
	: m_eventloop(eventloop)
{
    // Initialize static responses if not done already.
    static std::once_flag flag;
    std::call_once(flag, &init_static_responses);

	// Start httpd
    unsigned int flags = MHD_USE_DUAL_STACK | MHD_USE_EPOLL_LINUX_ONLY
            | MHD_USE_SUSPEND_RESUME | MHD_USE_TCP_FASTOPEN;
#   ifndef NDEBUG
    flags |= MHD_USE_DEBUG;
#   endif
    m_deamon = OSCHECK(MHD_start_daemon,(flags,
                                         config.httpd.port, nullptr, nullptr,
                                         &handle_access, this,
                                         MHD_OPTION_NOTIFY_COMPLETED,
                                         &access_completed, this,
                                         MHD_OPTION_NONCE_NC_SIZE, 0u,
                                         MHD_OPTION_LISTENING_ADDRESS_REUSE, 1u,
                                         MHD_OPTION_END),
                       != nullptr);

    // Register at event loop
    m_select_handle = m_eventloop->register_handler(
            [this](auto&... args) {this->io_handler(args...);},
            [this](auto&... args) {this->fdset_getter(args...);}
	);
}

httpserver_t::~httpserver_t() noexcept
{
    // Resume all connections.
    for (MHD_Connection *connection : suspended_connections) {
        MHD_resume_connection(connection);
    }

    // Stop HTTP daemon.
    MHD_stop_daemon(m_deamon);

    // Unregister from eventloop.
    m_eventloop->unregister_handler(m_select_handle);
}

httpserver_t::access_handler_t httpserver_t::route_request(
        MHD_Connection *connection, const char *url, const char *method,
        const char *version)
{
    return {};
}

void httpserver_t::fdset_getter(fd_set &rs, fd_set &ws, fd_set &es,
                                int &max, std::chrono::nanoseconds &timeout)
{
    const union MHD_DaemonInfo *info = OSCHECK(MHD_get_daemon_info,(
                m_deamon, MHD_DAEMON_INFO_EPOLL_FD_LINUX_ONLY), != nullptr);
    FD_SET(info->epoll_fd, &rs);
    max = info->epoll_fd + 1;

    MHD_UNSIGNED_LONG_LONG mhd_timeout;
    if (MHD_get_timeout(m_deamon, &mhd_timeout) == MHD_YES) {
        timeout = std::chrono::milliseconds(mhd_timeout);
    }
}

void httpserver_t::io_handler(const fd_set &rs, const fd_set &ws,
                              const fd_set &es)
{
    //int ret = MHD_run_from_select(deamon, &rs, &ws, &es);
    OSCHECK(MHD_run,(m_deamon), == MHD_YES);
}

int httpserver_t::handle_access(
        void *cls, MHD_Connection *connection,
        const char *url, const char *method, const char *version,
        const char *upload_data, size_t *upload_data_size, void **con_cls) noexcept
{
    httpserver_t *server = static_cast<httpserver_t*>(cls);
    connection_data_t *data = static_cast<connection_data_t*>(*con_cls);

    // TODO handle logging properly (start new procedure for every request)

    if (data == nullptr) {
        // Cut prefix. Return 404 if it is not used by the request.
        if (strncmp(url, config.httpd.prefix.data(), config.httpd.prefix.size())) {
            return MHD_queue_response(connection, 404, response_404);
        }
        url = url + config.httpd.prefix.size();
        // Route request and get handler.
        access_handler_t handler;
        try {
            handler = server->route_request(connection, url, method, version);
        } catch (const std::exception &e) {
            LOG_FAILURE(e) << e.what();
            return MHD_queue_response(connection, 500, response_500);
        }
        // Respond with 404 if no handler has been set.
        if (!handler) {
            return MHD_queue_response(connection, 404, response_404);
        }
        // Save handler
        data = new connection_data_t{
                server->route_request(connection, url, method, version)
        };
        *con_cls = data;
    }

    // Delegate to request handler.
    try {
        data->access_handler(connection, upload_data, upload_data_size);
    } catch (const std::exception &e) {
       LOG_FAILURE(e) << e.what();
       return MHD_queue_response(connection, 500, response_500);
    }

    return MHD_YES;
}

void httpserver_t::access_completed(
        void *cls, MHD_Connection *connection,
        void **con_cls, MHD_RequestTerminationCode toe) noexcept
{
    httpserver_t *server = static_cast<httpserver_t*>(cls);
    connection_data_t *data = static_cast<connection_data_t*>(*con_cls);
    if (data != nullptr) {
        delete data;
    }
}
