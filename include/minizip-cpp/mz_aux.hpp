#ifndef MZ_AUX_HEADER
#define MZ_AUX_HEADER

#include <contrib/minizip/unzip.h>
#include <contrib/minizip/zip.h>

#include <format>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <cerrno>
#include <cstring>

#if __has_include("../../local_defines.hpp")
    #include "../../local_defines.hpp"
#else
    #include <expected>
#endif

namespace mzpp {

struct zip;
struct unzip;

template <typename Action>
struct error {
    static_assert(std::is_same_v<Action, zip> || std::is_same_v<Action, unzip>);
    error(int c, std::string const& msg) : message(msg), code(c) {}
    std::string what() { return fmt("Error ({}): {}", code_to_str<Action>(code), message); }
    int code;
    std::string message;
};

using entry_desc = std::pair<std::string, unz_file_info64>;
using entry_list = std::vector<entry_desc>;
using entry_index = decltype(unz_global_info::number_entry);

template <typename T>
[[nodiscard]] inline std::string code_to_str(int code) {
    static_assert(std::is_same_v<T, unzip> || std::is_same_v<T, zip>);
    std::string msg = "unknown error";
    if constexpr (std::is_same_v<T, unzip>) {
        switch (code) {
            case UNZ_BADZIPFILE:
                msg = "bad zipfile error";
                break;
            case UNZ_CRCERROR:
                msg = "crc error";
                break;
            case UNZ_INTERNALERROR:
                msg = "internal error";
                break;
            case UNZ_PARAMERROR:
                msg = "parameter error";
                break;
            case UNZ_END_OF_LIST_OF_FILE:
                msg = "end of file list error";
                break;
            case UNZ_ERRNO:
                msg = std::strerror(errno);
            default:
                break;
        }
    } else {
    }
    return msg;
}

template<class... Args>
std::string fmt(std::format_string<Args...> str, Args&&... args)
{
    return std::format(str, std::forward<Args>(args)...);
}

template <typename T, typename E>
using ex = std::expected<T, E>;

template <typename T>
using ux = std::unexpected<T>;

template <typename T>
[[nodiscard]] auto make_unexpected(int code) {
    return std::unexpected(error<T>(code, code_to_str<T>(code)));
}

template <typename T>
[[nodiscard]] auto make_unexpected(int code, std::string const& msg) {
    return std::unexpected(error<T>(code, msg));
}

template <typename T>
[[nodiscard]] auto make_unexpected(mzpp::error<T>&& err) {
    return std::unexpected(std::move(err));
}

template <typename U, typename T>
[[nodiscard]] inline auto make_unexpected(std::expected<T, mzpp::error<U>>&& exp) {
    return std::unexpected(std::move(exp).error());
}
} // namespace mzpp
#endif
