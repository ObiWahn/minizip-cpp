#ifndef MZPP_HEADER
#define MZPP_HEADER

#include "mz_aux.hpp"

#include <filesystem>
#include <format>
#include <type_traits>

namespace mzpp {
namespace fs = std::filesystem;

template <typename T>
concept UnzipSink = std::is_same_v<T, std::string> || std::is_base_of_v<std::ostream, T>;

struct unzip {
    unzip() = default;
    unzip(unzip const&) = delete;
    unzip& operator=(unzip const&) = delete;
    unzip(fs::path const& path) {
        std::ignore = open(path);
    }
    ~unzip() {
        std::ignore = close();
    }

    // error handling
    using error = mzpp::error<unzip>;
    template<class... Args>
    auto mux(Args&&... args)
    {
        return mzpp::make_unexpected<unzip>(std::forward<Args>(args)...);
    }

    auto open(fs::path const& path) -> std::expected<void, error>;
    auto close() -> std::expected<void, error>;
    auto list() -> std::expected<entry_list, error>;
    auto size() const -> entry_index {
        return m_global_info.number_entry;
    }
    auto is_open() const -> bool {
        return m_handle != nullptr;
    }

    template <UnzipSink snk>
    ex<void, error> read_into(entry_index index, snk& sink, const char* password = nullptr) {
        if (auto res = open_entry(index, password); !res)
            return mux(std::move(res));

        std::vector<std::string::value_type> buf(std::numeric_limits<std::uint16_t>::max());
        error loop_err(UNZ_OK, "");
        {
            int read = 0;
            do {
                auto read = unzReadCurrentFile(m_handle, buf.data(), buf.size());
                if (read < 0)
                    break;
                if constexpr (std::is_same_v<snk, std::string>)
                    sink.append(buf.data(), read);
                else if constexpr (std::is_base_of_v<std::ostream, snk>)
                    sink.write(buf.data(), read);
                else {
                    loop_err.message = "COMPILE TIME PROBLEM: PUNCH THE DEV FOR NOT TESTING THE CODE :)";
                    loop_err.code = UNZ_INTERNALERROR;
                }
            } while (read > 0);

            if (read < 0) {
                loop_err.message = fmt("error in unzReadCurrentFile at entry {}", index);
                loop_err.code = UNZ_BADZIPFILE;
            }
        }
        auto close_err = unzCloseCurrentFile(m_handle);
        if (loop_err.code != UNZ_OK)
            return mux(std::move(loop_err));
        if (close_err != UNZ_OK)
            return mux(close_err, fmt("unzCloseCurrentFile at entry {}", index));
        return {};
    }

    private:
    auto select_entry(entry_index wanted) -> std::expected<void, error>;
    auto get_current_info(entry_index) -> std::expected<entry_desc, error>; // the index is informative only
    auto open_entry(entry_index wanted, char const* password = nullptr) -> std::expected<entry_desc, error>;

    unzFile m_handle = nullptr;
    unz_global_info m_global_info = {0};
};

struct zip_opts {
    bool overwrite = true;
    bool append = false;
    bool filename_only = false;
    // this prefix gets removed from the entry file name
    fs::path prefix;
};

template <typename T>
concept ZipSource = std::is_same_v<T, std::string> || std::is_base_of_v<std::ostream, T>;

struct zip {

    zip(zip_opts const& = zip_opts{});
    zip(zip const&) = delete;
    zip& operator=(zip const&) = delete;
    zip(fs::path const& path) {
        std::ignore = open(path);
    }
    ~zip() {
        std::ignore = close();
    }

    // error handling
    using error = mzpp::error<zip>;
    template<class... Args>
    auto mux(Args&&... args)
    {
        return mzpp::make_unexpected<zip>(std::forward<Args>(args)...);
    }


    auto open(fs::path const& path) -> std::expected<void, error>;
    auto close() -> ex<void, error>;
    auto is_open() const -> bool {
        return m_handle != nullptr;
    }

    template <ZipSource Src>
    ex<void, error> add_entry(std::string name_in_zip, Src& source, const char* password = nullptr);

    private:
    zip_opts m_zip_opts;
    int mode = 0;
    zipFile m_handle = nullptr;
};

} // namespace mzpp
#endif // MZPP_HEADER
