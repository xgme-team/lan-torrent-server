#ifndef ERRORHANDLING_HPP
#define ERRORHANDLING_HPP

/**
 * @file errorhandling.hpp
 * File containing primitives for error handling.
 */

#include <exception>

#include <boost/current_function.hpp>
#include <boost/exception/all.hpp>
#include <boost/stacktrace.hpp>


/**
 * Base class for exceptions used by this application.
 */
class basic_error
    : public virtual std::exception
    , public virtual boost::exception {
public:
    const char *what() const noexcept override
    {
        return mWhat;
    }
protected:
    basic_error() = default;
    basic_error(const char *what) : mWhat(what) {}
    basic_error(const basic_error &) = default;
    virtual ~basic_error() = default;
private:
    const char *mWhat = "";
};

/**
 * Exception thrown for unexpected system errors.
 *
 * Unexpected system errors refers to errors that are not expected to occur on
 * proper operating systems. A proper operating system is an operating system
 * where system calls, libraries and other interfaces are working as specified.
 *
 * In some cases, limited resources or unsupported system configurations can
 * also be expressed with this exception. For example, it might be a valid use
 * case when the process could not be forked, although it might be caused by
 * limited resources.
 *
 * Due to the surprising nature of such exceptions, they should be handled as
 * serious error. Applications are advised to shut down itself or the module
 * causing the exception.
 */
struct os_error : virtual basic_error {
    os_error() = default;
    os_error(const char *what) : basic_error(what) {}
    os_error(const os_error &) = default;
};

/**
 * Exception thrown for unexpected errors regarding to system files.
 *
 * As specialization of os_error, this exception should only be used for
 * unexpected problems and handled as serious error.
 *
 * Note that this exception should be used for invalid system files but not for
 * system errors regarding to user files.
 */
struct os_file_error : virtual os_error {
    os_file_error() = default;
    os_file_error(const char *what) : os_error(what) {}
    os_file_error(const os_file_error &) = default;
};

/**
 * Namespace containing types of additional information for exceptions.
 */
namespace errinfo {
    ///@name General
    ///@{
        /**
         * Another exception that has caused this one.
         */
        using cause     = boost::errinfo_nested_exception;
    ///@}
    ///@name API related
    ///@{
        /**
         * The system error code (`errno`) provided by an API.
         */
        using errnum    = boost::errinfo_errno;
        /**
         * The name of the API function that has caused the error.
         */
        using function  = boost::errinfo_api_function;
    ///@}
    ///@name File related
    ///@{
        /**
         * The name of the file which has been processed when the error
         * occurred.
         */
        using filename  = boost::errinfo_file_name;
        /**
         * The line of the file that has been processed when the error occurred.
         */
        using line      = boost::errinfo_at_line;
    ///@}
    ///@name Localization (Automatically added by THROW macro)
    ///@{
        /**
         * The stacktrace of the exception.
         */
        using trace     = boost::error_info<struct tag_trace,
                          boost::stacktrace::stacktrace>;
        /**
         * The function which has thrown the exception.
         */
        using srcfunc   = boost::throw_function;
        /**
         * The file where the exception has been thrown.
         */
        using srcfile   = boost::throw_file;
        /**
         * The line where the exception has been thrown.
         */
        using srcline   = boost::throw_line;
    ///@}
}

/**
 * A small utility function to crop “&” and “std::” from a string.
 */
const char *crop_ampersand_and_stdnamespace(const char *) noexcept;

/**
 * Throws the given exception and adds basic information.
 *
 * This macro should be used to thorw exceptions within this application. In
 * addition to throwing the given exception, the following information is added
 * to it:
 *
 *  -  errinfo::trace
 *  -  errinfo::srcfunc
 *  -  errinfo::srcfile
 *  -  errinfo::srcline
 */
#define THROW(exception)                                                \
        throw (exception)                                               \
            << errinfo::trace(boost::stacktrace::stacktrace())          \
            << errinfo::srcfunc(BOOST_CURRENT_FUNCTION)                 \
            << errinfo::srcfile(__FILE__)                               \
            << errinfo::srcline(static_cast<int>(__LINE__))

/**
 * Throws an os_error with the given message.
 *
 * In addition to the error message, the name of the given function and the
 * current `errno` value is attached to the exception.
 *
 * ```{.cpp}
 * while (read(fd, buf, sizeof(buf)) < 0) {
 *     if (errno != EINTR) {
 *         OSERROR(read, "`read()' failed on internal pipe");
 *     }
 * }
 * ```
 *
 * For some cases, you might want to use ::OSCHECK instead.
 */
#define OSERROR(fun, message)                                           \
        THROW(os_error(message))                                        \
            << errinfo::function(crop_ampersand_and_stdnamespace(#fun)) \
            << errinfo::errnum(errno)

/**
 * Calls the given function and throws os_error on failure.
 *
 * This macro is intended as shorthand for ::OSERROR. It calls the function
 * given as first argument with the argument list given as second argument.
 * Then, the return value is validated against the condition given as third
 * argument. If the validation fails, an os_error is thrown. Otherwise, the
 * macro resolves to the return value.
 *
 * ```{.cpp}
 * pid_t child_pid = OSCHECK(fork,(), >= 0);
 * ```
 */
#define OSCHECK(fun, args, cond) ([&](const char *_oscheck_funcname) {  \
        const auto _oscheck_ret = fun args ;                            \
        if (!(_oscheck_ret cond)) {                                     \
            OSERROR(fun, "`"#fun"()' has surprisingly failed")          \
                << errinfo::srcfunc(_oscheck_funcname);                 \
        }                                                               \
        return _oscheck_ret;                                            \
}(BOOST_CURRENT_FUNCTION))


#endif // ERRORHANDLING_HPP
