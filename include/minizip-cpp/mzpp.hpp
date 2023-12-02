#pragma once
#include <contrib/minizip/unzip.h>
#include <contrib/minizip/zip.h>

#include <chrono>
#include <filesystem>
#include <memory>


namespace mzpp {
namespace fs = std::filesystem;

struct unzip_file {

    unzip_file() = default;
    unzip_file(fs::path const& path) {
        open(path);
    }

    ~unzip_file() {
        close();
    }

    bool open(fs::path const& path) {
        handle = unzOpen(path.string().c_str());

        if(is_open() && UNZ_OK == unzGetGlobalInfo(handle, &file_info))
            return true;
        close();
        return false;
    }


    bool is_open() const {
        return handle != type{0};
    }

    void close() {
        file_info = { 0 };
        unzClose(handle);
    }

    private:
    using type = decltype(unzOpen(nullptr));
    type handle = {0};
    unz_global_info file_info = { 0 };
};
} // namespace mzpp
