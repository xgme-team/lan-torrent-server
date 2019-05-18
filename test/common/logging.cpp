#include <gtest/gtest.h>

#include <logging.hpp>


LOG_MODULE("test module")


TEST(LoggingTest, UsesModuleName) {
    const char *current_module = LOG_CURRENT_MODULE;
    EXPECT_STREQ("test module", current_module);
}
