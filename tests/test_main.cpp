#include <climits>
#include <cstring>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include "../blakserv/osd_linux.h"
#include "../blakserv/btime.h"
#include "crc.h"
#include "md5.h"
#include "rscload.h"
#include "test_framework.h"

static std::string TrimTrailingSpaces(std::string value)
{
    while (!value.empty() && value.back() == ' ')
    {
        value.pop_back();
    }
    return value;
}

constexpr int kRscTestResources = 2;
constexpr unsigned char kRscMagicBytes[] = {'R', 'S', 'C', 0x01};
constexpr int kRscFormatVersion = 4;
static int rsc_callback_count = 0;
static int rsc_resource_nums[kRscTestResources];
static std::string rsc_resource_strings[kRscTestResources];

static void ResetRscCallbackState(void)
{
    rsc_callback_count = 0;
    for (int i = 0; i < kRscTestResources; ++i)
    {
        rsc_resource_nums[i] = 0;
        rsc_resource_strings[i].clear();
    }
}

static int CreateTempFileWithPath(const char *template_suffix, std::vector<char> &path_buffer)
{
    std::error_code error;
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path(error);
    if (error)
    {
        return -1;
    }

    std::string temp_dir_string = temp_dir.string();
    if (!temp_dir_string.empty() && temp_dir_string.back() != '/')
    {
        temp_dir_string += '/';
    }
    temp_dir_string += template_suffix;
    path_buffer.assign(temp_dir_string.begin(), temp_dir_string.end());
    path_buffer.push_back('\0');
    return mkstemp(path_buffer.data());
}

static void CleanupTempFile(int fd, const std::vector<char> &path_buffer)
{
    if (fd != -1)
    {
        close(fd);
    }
    if (!path_buffer.empty())
    {
        unlink(path_buffer.data());
    }
}

static bool CollectRscCallback(const char *filename, int resource_num, const char *string)
{
    (void)filename;
    if (rsc_callback_count >= kRscTestResources)
    {
        return false;
    }
    rsc_resource_nums[rsc_callback_count] = resource_num;
    rsc_resource_strings[rsc_callback_count] = string;
    rsc_callback_count++;
    return true;
}

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

static int test_crc32_empty_string(void)
{
    const char *input = "";
    size_t length = strlen(input);
    unsigned int result;

    ASSERT_TRUE(length <= INT_MAX);
    result = CRC32(input, (int)length);

    ASSERT_EQ_UINT(0U, result);
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

static int test_crc32_incremental_matches_full(void)
{
    const char *input = "Meridian 59 unit tests";
    unsigned int mask = 0xFFFFFFFFU;
    size_t length = strlen(input);
    int length_int;
    unsigned int expected;
    unsigned int crc = mask;
    int split = 5;

    ASSERT_TRUE(length <= INT_MAX);
    length_int = (int)length;

    expected = CRC32(input, length_int);

    crc = CRC32Incremental(crc, input, split);
    crc = CRC32Incremental(crc, input + split, length_int - split);
    crc ^= mask;

    ASSERT_EQ_UINT(expected, crc);
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

static int test_md5_abc_value(void)
{
    const char *input = "abc";
    unsigned char digest[ENCRYPT_LEN];
    unsigned char expected[ENCRYPT_LEN] = {
        0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0,
        0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72
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
    const int one_hour_one_min_one_sec = 3600 + 60 + 1;

    ASSERT_TRUE(TrimTrailingSpaces(RelativeTimeStr(0)) == "0 sec");
    ASSERT_TRUE(TrimTrailingSpaces(RelativeTimeStr(one_hour_one_min_one_sec)) ==
        "1 hour 1 minute 1 second");
    return 0;
}

static int test_relative_time_with_days(void)
{
    const int sample_time = (2 * 24 * 60 * 60) + (3 * 60 * 60) + (4 * 60) + 5;

    ASSERT_TRUE(TrimTrailingSpaces(RelativeTimeStr(sample_time)) ==
        "2 days 3 hours 4 minutes 5 seconds");
    return 0;
}

static int test_get_milli_count_monotonic(void)
{
    UINT64 first = GetMilliCount();
    UINT64 second = GetMilliCount();

    ASSERT_TRUE(second >= first);
    return 0;
}

static int test_rscload_reads_resources(void)
{
    std::vector<char> path_buffer;
    int fd = CreateTempFileWithPath("meridian_rscXXXXXX", path_buffer);

    ASSERT_TRUE(fd != -1);

    FILE *file = fdopen(fd, "wb");
    if (file == NULL)
    {
        CleanupTempFile(fd, path_buffer);
        return 1;
    }

    int version = kRscFormatVersion;
    int num_resources = kRscTestResources;
    int resource_num = 10;
    const char *first = "hello";
    const char *second = "world";

    ASSERT_TRUE(fwrite(kRscMagicBytes, 1, sizeof(kRscMagicBytes), file) == sizeof(kRscMagicBytes));
    ASSERT_TRUE(fwrite(&version, 1, sizeof(version), file) == sizeof(version));
    ASSERT_TRUE(fwrite(&num_resources, 1, sizeof(num_resources), file) == sizeof(num_resources));

    ASSERT_TRUE(fwrite(&resource_num, 1, sizeof(resource_num), file) == sizeof(resource_num));
    ASSERT_TRUE(fwrite(first, 1, strlen(first) + 1, file) == strlen(first) + 1);

    resource_num = 20;
    ASSERT_TRUE(fwrite(&resource_num, 1, sizeof(resource_num), file) == sizeof(resource_num));
    ASSERT_TRUE(fwrite(second, 1, strlen(second) + 1, file) == strlen(second) + 1);

    fclose(file);

    ResetRscCallbackState();
    ASSERT_TRUE(RscFileLoad(path_buffer.data(), CollectRscCallback));
    ASSERT_TRUE(rsc_callback_count == kRscTestResources);
    ASSERT_TRUE(rsc_resource_nums[0] == 10);
    ASSERT_TRUE(rsc_resource_strings[0] == "hello");
    ASSERT_TRUE(rsc_resource_nums[1] == 20);
    ASSERT_TRUE(rsc_resource_strings[1] == "world");

    CleanupTempFile(-1, path_buffer);
    return 0;
}

static int test_rscload_rejects_bad_magic(void)
{
    std::vector<char> path_buffer;
    int fd = CreateTempFileWithPath("meridian_rsc_badXXXXXX", path_buffer);

    ASSERT_TRUE(fd != -1);

    FILE *file = fdopen(fd, "wb");
    if (file == NULL)
    {
        CleanupTempFile(fd, path_buffer);
        return 1;
    }

    const unsigned char bad_magic[] = {0x00, 0x00, 0x00, 0x00};
    ASSERT_TRUE(fwrite(bad_magic, 1, sizeof(bad_magic), file) == sizeof(bad_magic));
    fclose(file);

    ResetRscCallbackState();
    ASSERT_TRUE(!RscFileLoad(path_buffer.data(), CollectRscCallback));
    CleanupTempFile(-1, path_buffer);
    return 0;
}

int main(void)
{
    int tests_run = 0;
    int failures = 0;

    failures += run_test("test_crc32_known_value", test_crc32_known_value, &tests_run);
    failures += run_test("test_crc32_empty_string", test_crc32_empty_string, &tests_run);
    failures += run_test("test_crc32_incremental", test_crc32_incremental, &tests_run);
    failures += run_test("test_crc32_incremental_matches_full", test_crc32_incremental_matches_full, &tests_run);
    failures += run_test("test_md5_standard_value", test_md5_standard_value, &tests_run);
    failures += run_test("test_md5_abc_value", test_md5_abc_value, &tests_run);
    failures += run_test("test_md5_zero_byte_replacement", test_md5_zero_byte_replacement, &tests_run);
    failures += run_test("test_time_strings_zero", test_time_strings_zero, &tests_run);
    failures += run_test("test_relative_time_format", test_relative_time_format, &tests_run);
    failures += run_test("test_relative_time_with_days", test_relative_time_with_days, &tests_run);
    failures += run_test("test_get_milli_count_monotonic", test_get_milli_count_monotonic, &tests_run);
    failures += run_test("test_rscload_reads_resources", test_rscload_reads_resources, &tests_run);
    failures += run_test("test_rscload_rejects_bad_magic", test_rscload_rejects_bad_magic, &tests_run);

    if (failures != 0)
    {
        fprintf(stderr, "%d test(s) failed.\n", failures);
        return 1;
    }

    printf("All %d tests passed.\n", tests_run);
    return 0;
}
