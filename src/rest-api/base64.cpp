#include <algorithm>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <string>

#include <base64.hpp>


static const char encoding_table_def[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'};
static const char encoding_table_url[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '-', '_'};
static std::int8_t decoding_table[256];


static std::uint_fast32_t from_char(char input) {
    return static_cast<std::uint8_t>(input);
}

static char to_char(std::uint_fast32_t input) {
    assert(input < 256);
    return static_cast<char>(input);
}

static void build_decoding_table()
{
    assert(sizeof (encoding_table_def) == 64);
    assert(sizeof (encoding_table_url) == 64);
    for (std::size_t i = 0; i < 256; ++i) {
        decoding_table[i] = -1;
    }
    for (std::int_fast8_t i = 0; i < 64; ++i) {
        std::uint_fast32_t v = from_char(encoding_table_def[i]);
        assert(decoding_table[v] == -1 || decoding_table[v] == i);
        decoding_table[v] = i;
    }
    for (std::int_fast8_t i = 0; i < 64; ++i) {
        std::uint_fast32_t v = from_char(encoding_table_url[i]);
        assert(decoding_table[v] == -1 || decoding_table[v] == i);
        decoding_table[v] = i;
    }
}

static std::uint_fast32_t decode_char(char input)
{
    // Build decoding table if not done already.
    static std::once_flag flag;
    std::call_once(flag, &build_decoding_table);

    // Decode given char
    std::int8_t decoded = decoding_table[from_char(input)];
    if (decoded < 0) {
        assert(false); // TODO throw exception
    }
    return static_cast<std::uint8_t>(decoded);
}

std::string b64_encode(const std::string &bin, bool url, bool omit_padding)
{
    std::string str;
    std::size_t len = 4 * ((bin.size() + 2) / 3);
    std::size_t padding (bin.size() % 3 == 0 ? 0 : 3 - bin.size() % 3);
    const char *table = encoding_table_def;

    // Change encoding table if base64url shall be used instead
    if (url) {
        table = encoding_table_url;
    }

    // Allocate storage and fill padding
    if (omit_padding) {
        str.resize(len - padding);
    } else {
        str.resize(len);
        for (std::size_t i = len - padding; i < len; ++i) {
            str[i] = '=';
        }
    }

    // Encode data and return when complete
    std::size_t i = 0, j = 0;
    while (true) {
        std::size_t remaining = bin.size() - j;
        std::uint_fast32_t triple = 0;
        switch (remaining) {
            default:triple |= from_char(bin[j + 2]) <<  0;
            case 2: triple |= from_char(bin[j + 1]) <<  8;
            case 1: triple |= from_char(bin[j + 0]) << 16;
                    break;
            case 0: assert(i == len - padding);
                    assert(j == bin.size());
                    return str;
        }
        switch (remaining) {
            default:str[i + 3] = table[(triple >>  0) & 0x3F];
            case 2: str[i + 2] = table[(triple >>  6) & 0x3F];
            case 1: str[i + 1] = table[(triple >> 12) & 0x3F];
                    str[i + 0] = table[(triple >> 18) & 0x3F];
                    break;
            case 0: assert(false);
        }
        i = std::min(i + 4, len - padding);
        j = std::min(j + 3, bin.size());
    }
}

std::string b64_decode(const std::string &str)
{
    std::string bin;

    // Determine the length of input
    std::size_t len_in = str.size();
    while (str[len_in - 1] == '=') {
        --len_in;
    }

    // Determine the length of output
    std::size_t len_out = ((len_in + 3) / 4) * 3;
    switch (len_in % 4) {
    case 1: assert(false); // TODO throw exception
    case 2: --len_out;
    case 3: --len_out;
    }

    // Allocate storage
    bin.resize(len_out);

    // Decode data and return when complete
    std::size_t i = 0, j = 0;
    while (true) {
        std::size_t remaining = len_out - j;
        std::uint_fast32_t triple = 0;
        switch (remaining) {
            default:triple |= decode_char(str[i + 3]) <<  0;
            case 2: triple |= decode_char(str[i + 2]) <<  6;
            case 1: triple |= decode_char(str[i + 1]) << 12;
                    triple |= decode_char(str[i + 0]) << 18;
                    break;
            case 0: assert(i == len_in);
                    assert(j == len_out);
                    return bin;
        }
        switch (remaining) {
            default:bin[j + 2] = to_char(triple >>  0 & 0xFF);
            case 2: bin[j + 1] = to_char(triple >>  8 & 0xFF);
            case 1: bin[j + 0] = to_char(triple >> 16 & 0xFF);
                    break;
            case 0: assert(false);
        }
        i = std::min(i + 4, len_in);
        j = std::min(j + 3, len_out);
    }
}
