#include <format>
#include <fstream>
#include <gtest/gtest.h>
#include <minizip-cpp/mzpp.hpp>
#define EXT_STRINGIFY_DETAIL(a) #a
#define EXT_STRINGIFY(a) EXT_STRINGIFY_DETAIL(a)
namespace fs = std::filesystem;
std::filesystem::path test_dir = EXT_STRINGIFY(MZPP_TEST_PATH);


TEST(unzip, open_file_fail) {
    fs::path p = test_dir / "sam";
    std::error_code ec;
    ASSERT_FALSE(fs::exists(p, ec));
    mzpp::unzip zip(p);
    ASSERT_TRUE(!zip.is_open());
    auto result = zip.open(p);
    ASSERT_FALSE(result);
    ASSERT_EQ(result.error().message, std::format("'{}' not found", p.string()));
}

TEST(unzip, open_file_success) {
    fs::path p = test_dir / "unz_sample.zip";
    std::error_code ec;
    ASSERT_TRUE(fs::exists(p, ec)) << ec.message();
    mzpp::unzip zip(p);
    ASSERT_TRUE(zip.is_open()) << "ls -lisah " << p;
}

TEST(unzip, list) {
    mzpp::unzip zip(test_dir / "unz_sample.zip");
    ASSERT_TRUE(zip.is_open());
    auto list_result = zip.list();
    ASSERT_TRUE(list_result.has_value()) << list_result.error().message;
    ASSERT_EQ(list_result.value().size(), 2);
    ASSERT_EQ(list_result.value()[0].first, "unz_sample_short.txt");
    ASSERT_EQ(list_result.value()[1].first, "unz_sample_long.txt");
}

TEST(unzip, get_entry) {
    mzpp::unzip zip(test_dir / "unz_sample.zip");
    ASSERT_TRUE(zip.is_open());

    {
        std::string sink;
        auto ok = zip.read_into(0, sink);
        ASSERT_TRUE(ok) << ok.error().message;
        ASSERT_EQ(sink, "kurzes sample\n");
    }

    {
        std::string sink;
        auto ok = zip.read_into(1, sink);
        ASSERT_TRUE(ok) << ok.error().message;
        ASSERT_EQ(sink.length(), 51764);
    }

    {
        std::ofstream sink(test_dir / "foo", std::ios::out);
        auto ok = zip.read_into(1, sink);
        ASSERT_TRUE(ok) << ok.error().message;
    }
}
