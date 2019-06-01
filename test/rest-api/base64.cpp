#include <gtest/gtest.h>

#include <base64.hpp>


static const std::uint8_t bytes[] = {
    0x00, 0x10, 0x83, 0x10, 0x51, 0x87, 0x20, 0x92, 0x8b, 0x30, 0xd3, 0x8f,
    0x41, 0x14, 0x93, 0x51, 0x55, 0x97, 0x61, 0x96, 0x9b, 0x71, 0xd7, 0x9f,
    0x82, 0x18, 0xa3, 0x92, 0x59, 0xa7, 0xa2, 0x9a, 0xab, 0xb2, 0xdb, 0xaf,
    0xc3, 0x1c, 0xb3, 0xd3, 0x5d, 0xb7, 0xe3, 0x9e, 0xbb, 0xf3, 0xdf, 0xbf};
static const std::string bytes_str{reinterpret_cast<const char*>(bytes), 48};


TEST(Base64Test, EmptyStringRepresentsEmptyString) {
    EXPECT_EQ("", b64_encode("", false, false));
    EXPECT_EQ("", b64_encode("", true, true));
    EXPECT_EQ("", b64_decode(""));
}

TEST(Base64Test, PaddingIsIgnoredAtDecoding) {
    EXPECT_EQ(std::string("\0\0\0", 3), b64_decode("AAAA============"));
}

TEST(Base64Test, AddPaddingIfLengthNotMultipleOfThree) {
    EXPECT_EQ("MA==", b64_encode("0"));
    EXPECT_EQ("MDA=", b64_encode("00"));
}

TEST(Base64Test, OmitPaddingIfRequested) {
    EXPECT_EQ("MA", b64_encode("0", false, true));
    EXPECT_EQ("MDA", b64_encode("00", false, true));
}

TEST(Base64Test, CanEncodeBase64) {
    EXPECT_EQ("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
              "abcdefghijklmnopqrstuvwxyz"
              "0123456789+/",
              b64_encode(bytes_str));
}

TEST(Base64Test, CanEncodeBase64url) {
    EXPECT_EQ("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
              "abcdefghijklmnopqrstuvwxyz"
              "0123456789-_",
              b64_encode(bytes_str, true));
}

TEST(Base64Test, CanDecodeBase64) {
    EXPECT_EQ(bytes_str, b64_decode("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "0123456789+/"));
}

TEST(Base64Test, CanDecodeBase64url) {
    EXPECT_EQ(bytes_str, b64_decode("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "0123456789-_"));
}
