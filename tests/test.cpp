#include <gtest/gtest.h>
#include <minizip-cpp/mzpp.hpp>

#define EXT_STRINGIFY_DETAIL(a) #a
#define EXT_STRINGIFY(a) EXT_STRINGIFY_DETAIL(a)
namespace fs = std::filesystem;
std::filesystem::path test_dir = EXT_STRINGIFY(MZPP_TEST_PATH);

TEST(mzpp, zip) {
    fs::path p = test_dir / "sample.zip";
    std::error_code ec;
    ASSERT_TRUE(fs::exists(p,ec));
    mzpp::unzip_file zip(p);
    ASSERT_TRUE(zip.is_open()) << p ;
}
