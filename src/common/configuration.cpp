#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include <sysexits.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/program_options.hpp>

#include <buildconf.h>
#include <configuration.hpp>

using boost::program_options::bool_switch;
using boost::program_options::command_line_parser;
using boost::program_options::notify;
using boost::program_options::options_description;
using boost::program_options::parse_config_file;
using boost::program_options::store;
using boost::program_options::value;
using boost::program_options::variables_map;
using std::cerr;
using std::endl;
using std::string;


static configuration_t cfg;
const configuration_t &config = cfg;


static void validate(boost::any &v, const std::vector<std::string> &values,
                     storage_format_e* target_type, int)
{
    using boost::program_options::validation_error;
    using boost::program_options::validators::check_first_occurrence;
    using boost::program_options::validators::get_single_string;

    // Make sure no previous assignment was made.
    check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const string& s = get_single_string(values);

    // Check value and assign value.
    if (s == "plain")
        v = boost::any(storage_format_e::PLAIN);
    else if (s == "zip")
        v = boost::any(storage_format_e::ZIP);
    else if (s == "zip_if_dir")
        v = boost::any(storage_format_e::ZIP_IF_DIR);
    else
        throw validation_error(validation_error::invalid_option_value);
}


void load_configuration(int argc, const char *const argv[])
{
    // Define configuration options
    options_description clidesc("Command line only");
    clidesc.add_options()
            ("help,h",    "Show help message.")
            ("verbose,v", "Print log to stderr.")
            ("version",   "Print version string.")
            ("inifile",   value<string>(&cfg.inifile)
                          ->value_name("file")
                          ->default_value(XLTS_DEFAULT_INIFILE),
                          "Path to configuration file.")
            ;
    options_description genericdesc("Configuration");
    genericdesc.add_options()
            ("storage.downloads",
                 value<string>(&cfg.storage.downloads)
                 ->value_name("directory")
                 ->default_value(XLTS_DEFAULT_DOWNLOADDIR),
                 "Directory to store downloaded files.")
            ("storage.resumedata",
                 value<string>(&cfg.storage.resumedata)
                 ->value_name("directory")
                 ->default_value(XLTS_DEFAULT_RESUMEDATADIR),
                 "Directory to store resume data.")
            ("storage.tmpdir",
                 value<string>(&cfg.storage.tmpdir)
                 ->value_name("directory")
                 ->default_value(""),
                 "Directory to store files before fully downloaded.")
            ("storage.torrents",
                 value<string>(&cfg.storage.torrents)
                 ->value_name("directory")
                 ->default_value(XLTS_DEFAULT_TORRENTDIR),
                 "Directory to store torrent files.")
            ("storage.format",
                 value<storage_format_e>(&cfg.storage.format)
                 ->value_name("format")
                 ->default_value(storage_format_e::PLAIN, "plain"),
                 "Used format to store downloaded files.")
            ("storage.use-sparse-files",
                 bool_switch(&cfg.storage.use_sparse_files)
                 ->default_value(false),
                 "Use sparse files to store yet incomplete data.")

            ("torrent.cachesize",
                 value<int>(&cfg.torrent.cachesize)
                 ->value_name("num_blocks")
                 ->default_value(1024),
                 "Size of read/write cache as amount of 16 KiB blocks.")
            ("torrent.cachefile",
                 value<string>(&cfg.torrent.cachefile)
                 ->value_name("file")
                 ->default_value(""),
                 "Specifies a file to be used as read/write cache. The file "
                 "will be mapped to memory through mmap. Can be used to "
                 "provide a much bigger cache on a fast disk as possible on "
                 "RAM. This will disable contiguous_recv_buffer and can impact "
                 "seeding performance.")
            ("torrent.read-cache-line-size",
                 value<int>(&cfg.torrent.read_cacheline_size)
                 ->value_name("num_blocks")
                 ->default_value(32),
                 "Number of blocks to read on cache miss.")
            ("torrent.write-cache-line-size",
                 value<int>(&cfg.torrent.write_cacheline_size)
                 ->value_name("num_blocks")
                 ->default_value(16),
                 "Number of blocks to cache before they are flushed.")
            ("torrent.read-os-cache",
                 value<bool>(&cfg.torrent.read_os_cache)
                 ->default_value(true),
                 "Enable or disable os cache while reading files.")
            ("torrent.write-os-cache",
                 value<bool>(&cfg.torrent.write_os_cache)
                 ->default_value(true),
                 "Enable or disable os cache while writing files.")
            ("torrent.low-disk-priority",
                 value<bool>(&cfg.torrent.lowdiskprio)
                 ->implicit_value(true)->zero_tokens()
                 ->default_value(false),
                 "Use low priority for disk I/O.")
            ("torrent.file-pool-size",
                 value<int>(&cfg.torrent.file_pool_size)
                 ->default_value(40),
                 "Upper limit on the total number of files the torrent session "
                 " will keep open.")
            ("torrent.make-suggestions",
                 bool_switch(&cfg.torrent.suggestions)
                 ->default_value(false),
                 "Make suggestions about pieces that are in cache already.")

            ("httpd.port",
                 value<std::uint16_t>(&cfg.httpd.port)
                 ->value_name("port")
                 ->default_value(8080),
                 "port to listen on for HTTP requests")
            ("httpd.prefix",
                 value<string>(&cfg.httpd.prefix)
                 ->value_name("prefix")
                 ->default_value("/"),
                 "prefix for the pathes used by the HTTP server")
            ;

    variables_map vm;
    // Parse command line options
    try {
        options_description desc;
        desc.add(clidesc).add(genericdesc);
        store(command_line_parser(argc, argv)
                .options(desc)
                .positional({}).run(),
              vm);
        notify(vm);

        // Setup syslog
        if (vm.count("verbose")) {
            openlog(nullptr, LOG_PID | LOG_PERROR, LOG_USER);
        } else {
            openlog(nullptr, LOG_PID, LOG_USER);
        }

        // Check for --help and --version
        if (vm.count("help")) {
            cerr << desc;
            std::exit(EX_OK);
        }
        if (vm.count("version")) {
            cerr << XLTS_VERSION;
            std::exit(EX_OK);
        }
    } catch (boost::program_options::too_many_positional_options_error &e) {
        cerr << e.what() << std::endl;
        std::exit(EX_USAGE);
    } catch (boost::program_options::error_with_option_name &e) {
        cerr << e.what() << std::endl;
        std::exit(EX_USAGE);
    }

    // Parse configuration file
    try {
        if (!cfg.inifile.empty()) {
            options_description desc;
            desc.add(genericdesc);
            store(parse_config_file<char>(cfg.inifile.c_str(), desc), vm);
            notify(vm);
        }
    } catch (boost::program_options::reading_file &e) {
        cerr << "Could not read configuration file: " << cfg.inifile << "\n"
             << "    " << e.what() << std::endl;
        std::exit(EX_CONFIG);
    } catch (boost::program_options::error_with_option_name &e) {
        cerr << "Invalid configuration file: " << cfg.inifile << "\n"
             << "    " << e.what() << std::endl;
        std::exit(EX_CONFIG);
    }

    // Set `cfg.storage.tmpdir` to `cfg.storage.downloads` if not set.
    // Otherwise, ensure that both pathes are on the same filesystem.
    if (cfg.storage.tmpdir.empty()) {
        cfg.storage.tmpdir = cfg.storage.downloads;
    } else {
        struct stat tmpdirStat, downloadsStat;
        if (stat(cfg.storage.tmpdir.c_str(), &tmpdirStat)) {
            std::perror("stat() on storage.tmpdir failed");
            std::exit(EX_OSERR);
        } else if (stat(cfg.storage.downloads.c_str(), &downloadsStat)) {
            std::perror("stat() on storage.downloads failed");
            std::exit(EX_OSERR);
        } else if (tmpdirStat.st_dev != downloadsStat.st_dev) {
            std::cerr << "storage.tmpdir and storage.downloads have to be on "
                      << "the same filesystem." << std::endl;
            std::exit(EX_CONFIG);
        }
    }

    // Ensure that `cfg.httpd.prefix` starts and ends with '/'.
    if (cfg.httpd.prefix.back() != '/')
        cfg.httpd.prefix = cfg.httpd.prefix + "/";
    if (cfg.httpd.prefix.front() != '/')
        cfg.httpd.prefix = "/" + cfg.httpd.prefix;
}
