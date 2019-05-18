#ifndef LOGGING_HPP
#define LOGGING_HPP

/**
 * @file logging.hpp
 * File containing utilities related to logging.
 */

#include <cstdint>
#include <boost/current_function.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

namespace _logging_internal {
    BOOST_LOG_GLOBAL_LOGGER(logger, boost::log::sources::logger_mt)

    struct This {
        constexpr This() {}
    };

    template <class T>
    static inline constexpr char const *current_module(T) noexcept {
        return "";
    }
}

/**
 * Initializes logging functionality.
 *
 * This function have to be called before writing anything to the log. It is
 * called at the beginning of the `main` function.
 */
void logging_init();

std::uint64_t logging_procedure_get();

std::uint64_t logging_procedure_push();

std::uint64_t logging_procedure_pop();

/**
 * The type of relation from a log record to a procedure.
 */
enum class logrecord_type_e {
    start,      //!< Marks the beginning of a procedure.
    success,    //!< Marks a successful end of a procedure.
    failure,    //!< Marks an unsuccessful end of a procedure.
    info,       //!< Some informational record.
    warning,    //!< Some informational record which indicates a problem.
    debug,      //!< Some informational record for developers.
};

/**
 * Namespace containing types of attributes that might be added to log records.
 */
namespace logattr {
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            line_id,        "LineID",
            unsigned int)
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            timestamp,      "TimeStamp",
            boost::posix_time::ptime)
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            process_id,     "ProcessID",
            boost::log::attributes::current_process_id::value_type)
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            thread_id,      "ThreadID",
            boost::log::attributes::current_thread_id::value_type)
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            module_name,    "ModuleName",
            const char *)
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            record_type,    "RecordType",
            logrecord_type_e)
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            procedure,      "Procedure",
            std::uint64_t)
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            new_procedure,  "NewProcedure",
            std::uint64_t)
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            is_async,       "Async",
            bool)
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            exception,      "Exception",
            boost::exception_ptr)
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            srcfunc,        "SourceFunction",
            const char *)
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            srcfile,        "SourceFile",
            const char *)
    BOOST_LOG_ATTRIBUTE_KEYWORD(
            srcline,        "SourceLine",
            int)
}

#define LOG_MODULE(name)                                                \
    namespace _logging_internal {                                       \
        template<>                                                      \
        constexpr char const *current_module<This>(This) noexcept {     \
            return name;                                                \
        }                                                               \
    }

#define LOG_CURRENT_MODULE ([&]() {                                     \
    namespace internal = _logging_internal;                             \
    return internal::current_module(internal::This());                  \
}())

#define LOG_INTERNAL(type) BOOST_LOG((_logging_internal::logger::get()))   \
    << boost::log::add_value(logattr::record_type, logrecord_type_e::type) \
    << boost::log::add_value(logattr::module_name, LOG_CURRENT_MODULE)     \
    << boost::log::add_value(logattr::srcfunc, BOOST_CURRENT_FUNCTION)     \
    << boost::log::add_value(logattr::srcfile, __FILE__)                   \
    << boost::log::add_value(logattr::srcline, static_cast<int>(__LINE__)) \

#define LOG_START()                                                     \
    for (std::uint64_t _old_procedure = logging_procedure_get(),        \
                       _new_procedure = logging_procedure_push(),       \
                       _i = 0;                                          \
         _i < 1; ++_i)                                                  \
    LOG_INTERNAL(start)                                                 \
    << boost::log::add_value(logattr::procedure, _old_procedure)        \
    << boost::log::add_value(logattr::new_procedure, _new_procedure)    \
    << boost::log::add_value(logattr::is_async, false)

#define LOG_START_ASYNC(out) ([&]() {                                   \
    LOG_INTERNAL(start)                                                 \
    << boost::log::add_value(logattr::procedure, )                      \
    << boost::log::add_value(logattr::new_procedure, )                  \
    << boost::log::add_value(logattr::is_async, true)                   \
}())

#define LOG_SUCCESS()                                                   \
    for (std::uint64_t _procedure = logging_procedure_pop(),            \
                       _i = 0;                                          \
         _i < 1; ++_i)                                                  \
    LOG_INTERNAL(success)                                               \
    << boost::log::add_value(logattr::procedure, _procedure)

#define LOG_FAILURE(exception_obj)                                      \
    for (std::uint64_t _procedure = logging_procedure_pop(),            \
                       _i = 0;                                          \
         _i < 1; ++_i)                                                  \
    LOG_INTERNAL(failure)                                               \
    << boost::log::add_value(logattr::procedure, _procedure)            \
    << boost::log::add_value(logattr::exception,                        \
                             boost::copy_exception(exception_obj))

#define LOG_INFO() LOG_INTERNAL(info)

#define LOG_WARN() LOG_INTERNAL(warning)

#define LOG_DEBUG() LOG_INTERNAL(debug)

#endif // LOGGING_HPP
