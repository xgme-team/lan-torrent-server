#include <algorithm>
#include <chrono>
#include <cstdint>
#include <new>

#include <signal.h>
#include <sysexits.h>

#ifdef XLTS_USE_SYSTEMD
#   include <systemd/sd-daemon.h>
#endif

#include <configuration.hpp>
#include <errorhandling.hpp>
#include <eventloop.hpp>
#include <httpd.hpp>
#include <logging.hpp>


static bool should_stop = false;


static void sighandler(int)
{
    should_stop = true;
}

#ifdef XLTS_USE_SYSTEMD
    static std::chrono::microseconds update_interval;
    static void statusupdates(eventloop_t &eventloop)
    {
        if (should_stop) {
            return; // TODO Check if WATCHDOG must still be sent on shutdown.
        }
        sd_notify(0, "STATUS=Application is running ...\n"
                     "READY=1\n" "WATCHDOG=1\n");
        eventloop.call([&]() { statusupdates(eventloop); }, update_interval);
    }
#endif

static void main0(int argc, char *argv[])
{
    using std::chrono::microseconds;
    using namespace std::literals::chrono_literals;

    // Initialize logging
    logging_init();

#   ifdef XLTS_USE_SYSTEMD
        sd_notify(0, "STATUS=Loading configuration ...\n");
#   endif

    // Load configuration (load_configuration() may quit the application)
    load_configuration(argc, argv);

#   ifdef XLTS_USE_SYSTEMD
        sd_notify(0, "STATUS=Initializing ...\n");
#   endif

    // Block all signals. They will be unblocked by event loop when appropriate.
    sigset_t signal_mask;
    OSCHECK(sigfillset,(&signal_mask), == 0);
                                                      // This signals are raised
    OSCHECK(sigdelset,(&signal_mask, SIGBUS),  == 0); // on fatal errors.
    OSCHECK(sigdelset,(&signal_mask, SIGFPE),  == 0); // According to the man
    OSCHECK(sigdelset,(&signal_mask, SIGILL),  == 0); // page, it would cause
    OSCHECK(sigdelset,(&signal_mask, SIGSEGV), == 0); // undefined behavior to
                                                      // mask them.
    OSCHECK(pthread_sigmask,(SIG_SETMASK, &signal_mask, nullptr), == 0);

    // Register signal handlers to quit the application properly.
    struct sigaction act;
    act.sa_handler = &sighandler;
    act.sa_flags   = 0;
    act.sa_mask    = signal_mask;

    if (sigaction(SIGINT, &act, nullptr) < 0) {
        OSERROR(sigaction, "Could not set signal handler for SIGINT");
    }
    if (sigaction(SIGTERM, &act, nullptr) < 0) {
        OSERROR(sigaction, "Could not set signal handler for SIGTERM");
    }

    // Start up application (initialize components)
    LOG_START() << "Initialize components ...";
    eventloop_t eventloop;
    httpserver_t httpserver(&eventloop);
    LOG_SUCCESS() << "Ready";

    // Send status updates when using Systemd
#   ifdef XLTS_USE_SYSTEMD
        std::uint64_t watchdog_usec;
        if (sd_watchdog_enabled(true, &watchdog_usec) > 0)
            update_interval = std::min(
                    microseconds(watchdog_usec) / 2,
                    microseconds(4s)
            );
        else
            update_interval = 4s;
        statusupdates(eventloop);
#   endif

    // Run eventloop
    OSCHECK(sigemptyset,(&signal_mask), == 0);
    eventloop.exec([&] { return should_stop; }, &signal_mask);

    // Notify Systemd about shutdown
#   ifdef XLTS_USE_SYSTEMD
        sd_notify(0, "STATUS=Shutting down ...\n"
                     "STOPPING=1\n");
#   endif

    // Shut down application
    LOG_START() << "Shutting down ...";
}

int main(int argc, char *argv[])
{
    try {
        main0(argc, argv);
        LOG_SUCCESS() << "Bye";
        return EX_OK;
    } catch (const os_file_error &e) {
        LOG_FAILURE(e) << e.what();
        return EX_OSFILE;
    } catch (const os_error &e) {
        LOG_FAILURE(e) << e.what();
        return EX_OSERR;
    } catch (const std::bad_alloc &e) {
        LOG_FAILURE(e) << e.what();
        return EX_OSERR;
    } catch (const std::exception &e) {
        LOG_FAILURE(e) << e.what();
        return EX_SOFTWARE;
    }
}
