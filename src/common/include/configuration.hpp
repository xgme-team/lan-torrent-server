#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

/**
 * @file configuration.h
 * File contains types and functions related to configuration management.
 *
 * The file declares the global variable {@link config} and it's type called
 * {@link configuration_t}. Other components can access the configuration
 * through this variable. It is initialized by {@link load_configuration()}
 * which is executed first after starting the application.
 */

#include <cstdint>
#include <string>


/**
 * Formats that can be used to store downloaded files.
 */
enum class storage_format_e {
    PLAIN,     //!< Save files as is.
    ZIP,       //!< Save files in one zip archive per torrent.
    ZIP_IF_DIR //!< Use ZIP on directories and PLAIN on single-file-torrents.
};

/**
 * Struct to hold global configuration.
 *
 * Member can be accessed through global variable {@link config}. It is
 * initialized by load_configuration(). Values are provided by command line
 * arguments and configuration file.
 */
struct configuration_t {
    std::string inifile; //!< Path to configuration file.

    struct storage_t {
        std::string downloads;   //!< Directory to save downloaded files.
        std::string resumedata;  //!< Directory to save resume data.
        std::string tmpdir;      //!< Directory to save files while downloading.
        std::string torrents;    //!< Directory to save torrent files.
        storage_format_e format; //!< Used format to store downloaded files.
        bool use_sparse_files;   //!< Whether sparse files will be used or not.
    } storage;

    struct torrent_t {
        int         cachesize; //!< Amount of read/write cache in 16KiB blocks.
        std::string cachefile; //!< A cachefile to use.
        int         read_cacheline_size;
        int         write_cacheline_size;
        bool        read_os_cache;
        bool        write_os_cache;
        bool        lowdiskprio; //!< Use low priority for disk I/O.
        int         file_pool_size;
        bool        suggestions;
    } torrent;

    struct httpd_t {
        //! Prefix for HTTP paths. Begins and ends with '/'.
        std::string   prefix;
        //! Port used by HTTP server.
        std::uint16_t port;
    } httpd;
};

/**
 * Global variable that holds global configuration.
 *
 * It is initialized by load_configuration(). You can find more information
 * about available options at {@link configuration_t}.
 */
extern const configuration_t &config;

/**
 * Load configuration from command line arguments and configuration file.
 *
 * On configuration errors and when using `--help` or `--version`, The function
 * will write to `stderr` and exit the application.
 *
 * @param argc Amount of arguments. First argument of main() should be passed.
 * @param argv Array of arguments. Second argument of main() should be passed.
 */
void load_configuration(int argc, const char *const argv[]);

#endif // CONFIGURATION_HPP
