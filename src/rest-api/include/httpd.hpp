#ifndef HTTPD_HPP
#define HTTPD_HPP

#include <functional>
#include <unordered_set>

#include <boost/core/noncopyable.hpp>

#include <microhttpd.h>

#include <eventloop.hpp>


class httpserver_t : private boost::noncopyable
{
public:
    using access_handler_t = std::function<void(
        struct MHD_Connection *connection,
        const char *upload_data, size_t *upload_data_size
    )>;

    httpserver_t(eventloop_t *eventloop);
    ~httpserver_t() noexcept;

protected:
    access_handler_t route_request(struct MHD_Connection *connection,
                                   const char *url, const char *method,
                                   const char *version);

private:
    void fdset_getter(fd_set &rs, fd_set &ws, fd_set &es, int &max,
                      std::chrono::nanoseconds &timeout);
    void io_handler(const fd_set &rs, const fd_set &ws, const fd_set &es);

    static int handle_access(
            void *cls, struct MHD_Connection *connection,
            const char *url, const char *method, const char *version,
            const char *upload_data, size_t *upload_data_size,
            void **con_cls) noexcept;
    static void access_completed(
            void *cls, MHD_Connection *connection,
            void **con_cls, MHD_RequestTerminationCode toe) noexcept;

    eventloop_t *m_eventloop;
    eventloop_t::select_handle_t m_select_handle;
    MHD_Daemon *m_deamon = nullptr;
    std::unordered_set<MHD_Connection*> suspended_connections;
};


#endif // HTTPD_HPP
