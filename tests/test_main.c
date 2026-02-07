#include <limits.h>
#include <string.h>

#include "../blakserv/blakserv.h"
#include "crc.h"
#include "md5.h"
#include "test_framework.h"

static int test_crc32_known_value(void)
{
    const char *input = "test";
    size_t length = strlen(input);
    unsigned int result;

    ASSERT_TRUE(length <= INT_MAX);
    result = CRC32(input, (int)length);

    ASSERT_EQ_UINT(0xd87f7e0cU, result);
    return 0;
}

static int test_crc32_incremental(void)
{
    const char *input = "Meridian59";
    unsigned int mask = 0xFFFFFFFFU;
    unsigned int crc = mask;
    int split = 4;
    size_t length = strlen(input);
    int length_int;

    ASSERT_TRUE(length <= INT_MAX);
    length_int = (int)length;

    crc = CRC32Incremental(crc, input, split);
    crc = CRC32Incremental(crc, input + split, length_int - split);
    crc ^= mask;

    ASSERT_EQ_UINT(0x55c1dbd4U, crc);
    return 0;
}

static int test_md5_standard_value(void)
{
    const char *input = "test";
    unsigned char digest[ENCRYPT_LEN];
    unsigned char expected[ENCRYPT_LEN] = {
        0x09, 0x8f, 0x6b, 0xcd, 0x46, 0x21, 0xd3, 0x73,
        0xca, 0xde, 0x4e, 0x83, 0x26, 0x27, 0xb4, 0xf6
    };

    MDString(input, digest);
    ASSERT_EQ_MEM(expected, digest, ENCRYPT_LEN);
    return 0;
}

static int test_md5_zero_byte_replacement(void)
{
    const char *input = "ai";
    unsigned char digest[ENCRYPT_LEN];
    unsigned char expected[ENCRYPT_LEN] = {
        0x49, 0x21, 0xc0, 0xe2, 0xd1, 0xf6, 0x01, 0x5a,
        0xbe, 0x1f, 0x9e, 0xc2, 0xe2, 0x04, 0x19, 0x09
    };

    MDString(input, digest);
    ASSERT_EQ_MEM(expected, digest, ENCRYPT_LEN);
    return 0;
}

static int test_time_strings_zero(void)
{
    ASSERT_TRUE(TimeStr(0) == "Never");
    ASSERT_TRUE(ShortTimeStr(0) == "Never");
    ASSERT_TRUE(FileTimeStr(0) == "Never");
    return 0;
}

static int test_relative_time_format(void)
{
    ASSERT_TRUE(RelativeTimeStr(0) == "0 sec");
    ASSERT_TRUE(RelativeTimeStr(3661) == "1 hour 1 minute 1 second ");
    return 0;
}

static int test_get_milli_count_monotonic(void)
{
    UINT64 first = GetMilliCount();
    UINT64 second = GetMilliCount();

    ASSERT_TRUE(second >= first);
    return 0;
}

int main(void)
{
    int tests_run = 0;
    int failures = 0;

    failures += run_test("test_crc32_known_value", test_crc32_known_value, &tests_run);
    failures += run_test("test_crc32_incremental", test_crc32_incremental, &tests_run);
    failures += run_test("test_md5_standard_value", test_md5_standard_value, &tests_run);
    failures += run_test("test_md5_zero_byte_replacement", test_md5_zero_byte_replacement, &tests_run);
    failures += run_test("test_time_strings_zero", test_time_strings_zero, &tests_run);
    failures += run_test("test_relative_time_format", test_relative_time_format, &tests_run);
    failures += run_test("test_get_milli_count_monotonic", test_get_milli_count_monotonic, &tests_run);

    if (failures != 0)
    {
        fprintf(stderr, "%d test(s) failed.\n", failures);
        return 1;
    }

    printf("All %d tests passed.\n", tests_run);
    return 0;
}
