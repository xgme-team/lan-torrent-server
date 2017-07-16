#include <vector>

#include <sysexits.h>

#include <gtest/gtest.h>

#include <buildconf.h>
#include <configuration.hpp>


#define INIFILE(name) XLTS_SOURCE_DIR "/test/common/inifiles/" name


TEST(ConfigurationDeathTest, GetVersionByLongOption) {
    std::vector<const char*> argv = {"", "--version"};
    ASSERT_EXIT({
            load_configuration(argv.size(), argv.data());
    }, ::testing::ExitedWithCode(EX_OK), XLTS_VERSION ".*");
}

TEST(ConfigurationDeathTest, GetHelpByShortOption) {
    std::vector<const char*> argv = {"", "-h"};
    ASSERT_EXIT({
            load_configuration(argv.size(), argv.data());
    }, ::testing::ExitedWithCode(EX_OK), ".*");
}

TEST(ConfigurationDeathTest, GetHelpByLongOption) {
    std::vector<const char*> argv = {"", "--help"};
    ASSERT_EXIT({
            load_configuration(argv.size(), argv.data());
    }, ::testing::ExitedWithCode(EX_OK), ".*");
}

TEST(ConfigurationDeathTest, EmptyArgument) {
    std::vector<const char*> argv = {"", ""};
    ASSERT_EXIT({
            load_configuration(argv.size(), argv.data());
    }, ::testing::ExitedWithCode(EX_USAGE), ".*");
}

TEST(ConfigurationDeathTest, InvalidArgument) {
    std::vector<const char*> argv = {"", "no-argument-should-be-accepted"};
    ASSERT_EXIT({
            load_configuration(argv.size(), argv.data());
    }, ::testing::ExitedWithCode(EX_USAGE), ".*");
}

TEST(ConfigurationDeathTest, InvalidLongOptions) {
    std::vector<const char*> argv = {"", "--invalid-option"};
    ASSERT_EXIT({
            load_configuration(argv.size(), argv.data());
    }, ::testing::ExitedWithCode(EX_USAGE), ".*");
}

TEST(ConfigurationDeathTest, ConfigurationFileMissing) {
    std::vector<const char*> argv = {"", "--inifile=some-missing-file-1234567"};
    ASSERT_EXIT({
            load_configuration(argv.size(), argv.data());
    }, ::testing::ExitedWithCode(EX_CONFIG), ".*");
}

TEST(ConfigurationDeathTest, ConfigurationFileIsDirectory) {
    std::vector<const char*> argv = {"", "--inifile", "."};
    ASSERT_EXIT({
            load_configuration(argv.size(), argv.data());
    }, ::testing::ExitedWithCode(EX_CONFIG), ".*");
}

TEST(ConfigurationDeathTest, ConfigurationFileInvalidOption) {
    const auto inifile = INIFILE("invalid-option.ini");
    std::vector<const char*> argv = {"", "--inifile", inifile};
    ASSERT_EXIT({
            load_configuration(argv.size(), argv.data());
    }, ::testing::ExitedWithCode(EX_CONFIG), ".*");
}

TEST(ConfigurationDeathTest, ConfigurationFileInvalidFormat) {
    const auto inifile = INIFILE("invalid-format.ini");
    std::vector<const char*> argv = {"", "--inifile", inifile};
    ASSERT_EXIT({
            load_configuration(argv.size(), argv.data());
    }, ::testing::ExitedWithCode(EX_CONFIG), ".*");
}


TEST(ConfigurationTest, NoArguments) {
    std::vector<const char*> argv = {""};
    load_configuration(argv.size(), argv.data());

    EXPECT_EQ(XLTS_DEFAULT_INIFILE, config.inifile);

    EXPECT_EQ(XLTS_DEFAULT_DOWNLOADDIR   , config.storage.downloads);
    EXPECT_EQ(XLTS_DEFAULT_RESUMEDATADIR , config.storage.resumedata);
    EXPECT_EQ(XLTS_DEFAULT_DOWNLOADDIR   , config.storage.tmpdir);
    EXPECT_EQ(XLTS_DEFAULT_TORRENTDIR    , config.storage.torrents);
    EXPECT_EQ(storage_format_e::PLAIN    , config.storage.format);
    EXPECT_EQ(false                      , config.storage.use_sparse_files);

    EXPECT_EQ( 1024, config.torrent.cachesize);
    EXPECT_EQ(   "", config.torrent.cachefile);
    EXPECT_EQ(   32, config.torrent.read_cacheline_size);
    EXPECT_EQ(   16, config.torrent.write_cacheline_size);
    EXPECT_EQ( true, config.torrent.read_os_cache);
    EXPECT_EQ( true, config.torrent.write_os_cache);
    EXPECT_EQ(false, config.torrent.lowdiskprio);
    EXPECT_EQ(   40, config.torrent.file_pool_size);
    EXPECT_EQ(false, config.torrent.suggestions);

    EXPECT_EQ(  "/", config.httpd.prefix);
    EXPECT_EQ( 8080, config.httpd.port);
}

TEST(ConfigurationTest, HttpdPrefixEnsureSlash) {
    std::vector<const char*> argv = {"", "--httpd.prefix=prefix"};
    load_configuration(argv.size(), argv.data());

    EXPECT_EQ("/prefix/", config.httpd.prefix);
}

TEST(ConfigurationTest, HttpdPrefixEnsureSlashWhenEmpty) {
    std::vector<const char*> argv = {"", "--httpd.prefix", ""};
    load_configuration(argv.size(), argv.data());

    EXPECT_EQ("/", config.httpd.prefix);
}

TEST(ConfigurationTest, HttpdPrefixDontAddRedundantSlash) {
    std::vector<const char*> argv = {"", "--httpd.prefix=/prefix/"};
    load_configuration(argv.size(), argv.data());

    EXPECT_EQ("/prefix/", config.httpd.prefix);
}

TEST(ConfigurationTest, StorageTmpdirDefaultsToDownloads) {
    std::vector<const char*> argv = {"", "--storage.downloads=some-dir"};
    load_configuration(argv.size(), argv.data());

    EXPECT_EQ("some-dir", config.storage.tmpdir);
}

TEST(ConfigurationTest, UseIniFile) {
    const auto inifile = INIFILE("valid.ini");
    std::vector<const char*> argv = {"", "--inifile", inifile};
    load_configuration(argv.size(), argv.data());

    EXPECT_EQ(inifile, config.inifile);

    EXPECT_EQ(".", config.storage.downloads);
    EXPECT_EQ(".", config.storage.tmpdir);
    EXPECT_EQ(storage_format_e::ZIP, config.storage.format);

    EXPECT_EQ(true, config.torrent.lowdiskprio);

    EXPECT_EQ(1234, config.httpd.port);
}
