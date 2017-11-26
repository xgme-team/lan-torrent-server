#include <cerrno>

#include <boost/current_function.hpp>
#include <boost/exception/all.hpp>

#include <gtest/gtest.h>

#include <errorhandling.hpp>


using boost::get_error_info;


struct some_exception : basic_error {
    some_exception() : basic_error("") {}
    some_exception(const char *what) : basic_error(what) {}
};

static int identity_function(int return_value) {
    return return_value;
}


TEST(ErrorHandlingTest, ThrowMacroThrows) {
    EXPECT_THROW(THROW(some_exception()), some_exception);
}

TEST(ErrorHandlingTest, ThrowMacroSetsSourceFunction) {
    try {
        THROW(some_exception());
    } catch (const some_exception &e) {
        const auto *info = get_error_info<errinfo::srcfunc>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_STREQ(BOOST_CURRENT_FUNCTION, *info);
    }
}

TEST(ErrorHandlingTest, ThrowMacroSetsSourceFile) {
    try {
        THROW(some_exception());
    } catch (const some_exception &e) {
        const auto *info = get_error_info<errinfo::srcfile>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_STREQ(__FILE__, *info);
    }
}

TEST(ErrorHandlingTest, ThrowMacroSetsSourceLine) {
    int line = 0;
    try {
        line = (int)__LINE__; THROW(some_exception());
    } catch (const some_exception &e) {
        const auto *info = get_error_info<errinfo::srcline>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_EQ(line, *info);
    }
}


TEST(ErrorHandlingTest, OSErrorMacroThrows) {
    EXPECT_THROW(OSERROR(read, ""), os_error);
}

TEST(ErrorHandlingTest, OSErrorMacroHasMessage) {
    try {
        OSERROR(read, "42 /\\");
    } catch (const os_error &e) {
        EXPECT_STREQ("42 /\\", e.what());
    }
}

TEST(ErrorHandlingTest, OSErrorMacroSetsApiFunction) {
    try {
        OSERROR(read, "");
    } catch (const os_error &e) {
        const auto *info = get_error_info<errinfo::function>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_STREQ("read", *info);
    }
}

TEST(ErrorHandlingTest, OSErrorMacroSetsErrno) {
    try {
        errno = 42;
        OSERROR(read, "");
    } catch (const os_error &e) {
        const auto *info = get_error_info<errinfo::errnum>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_EQ(42, *info);
    }
}

TEST(ErrorHandlingTest, OSErrorMacroSetsSourceFunction) {
    try {
        OSERROR(read, "");
    } catch (const os_error &e) {
        const auto *info = get_error_info<errinfo::srcfunc>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_STREQ(BOOST_CURRENT_FUNCTION, *info);
    }
}

TEST(ErrorHandlingTest, OSErrorMacroSetsSourceFile) {
    try {
        OSERROR(read, "");
    } catch (const os_error &e) {
        const auto *info = get_error_info<errinfo::srcfile>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_STREQ(__FILE__, *info);
    }
}

TEST(ErrorHandlingTest, OSErrorMacroSetsSourceLine) {
    int line = 0;
    try {
        line = (int)__LINE__; OSERROR(read, "");
    } catch (const os_error &e) {
        const auto *info = get_error_info<errinfo::srcline>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_EQ(line, *info);
    }
}


TEST(ErrorHandlingTest, OSCheckMacroThrowsOnError) {
    EXPECT_THROW(OSCHECK(identity_function,(-1), >= 0), os_error);
}

TEST(ErrorHandlingTest, OSCheckMacroDosntThrowOnSuccess) {
    EXPECT_NO_THROW(OSCHECK(identity_function,(1), >= 0));
}

TEST(ErrorHandlingTest, OSCheckMacroReturnsValueOnSuccess) {
    EXPECT_EQ(42, OSCHECK(identity_function,(42), >= 0));
}

TEST(ErrorHandlingTest, OSCheckMacroSetsApiFunction) {
    try {
        OSCHECK(identity_function,(-1), >= 0);
    } catch (const os_error &e) {
        const auto *info = get_error_info<errinfo::function>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_STREQ("identity_function", *info);
    }
}

TEST(ErrorHandlingTest, OSCheckMacroSetsErrno) {
    try {
        errno = 84;
        OSCHECK(identity_function,(-1), >= 0);
    } catch (const os_error &e) {
        const auto *info = get_error_info<errinfo::errnum>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_EQ(84, *info);
    }
}

TEST(ErrorHandlingTest, OSCheckMacroSetsSourceFunction) {
    try {
        OSCHECK(identity_function,(-1), >= 0);
    } catch (const os_error &e) {
        const auto *info = get_error_info<errinfo::srcfunc>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_STREQ(BOOST_CURRENT_FUNCTION, *info);
    }
}

TEST(ErrorHandlingTest, OSCheckMacroSetsSourceFile) {
    try {
        OSCHECK(identity_function,(-1), >= 0);
    } catch (const os_error &e) {
        const auto *info = get_error_info<errinfo::srcfile>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_STREQ(__FILE__, *info);
    }
}

TEST(ErrorHandlingTest, OSCheckMacroSetsSourceLine) {
    int line = 0;
    try {
        line = (int)__LINE__; OSCHECK(identity_function,(-1), >= 0);
    } catch (const os_error &e) {
        const auto *info = get_error_info<errinfo::srcline>(e);
        ASSERT_TRUE(info != nullptr);
        EXPECT_EQ(line, *info);
    }
}
