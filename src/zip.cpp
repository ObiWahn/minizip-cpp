#include <minizip-cpp/mzpp.hpp>

namespace mzpp {

zip::zip(zip_opts const& opts) : m_zip_opts(opts) {}

ex<void, error<zip>> zip::open(fs::path const& path) {
    if (is_open()) {
        if (auto ok = close(); !ok) {
            return mux(std::move(ok));
        }
    }

    std::error_code ec;
    auto mode = APPEND_STATUS_CREATE;
    if (fs::exists(path, ec)) {
        if (!fs::is_regular_file(path, ec) || (!fs::is_directory(path, ec) && fs::is_symlink(path, ec)))
            return mux(ZIP_PARAMERROR, fmt("'{}' is not a file", path.string()));
        if (m_zip_opts.append)
            mode = APPEND_STATUS_ADDINZIP;
        else if (!m_zip_opts.overwrite)
            return mux(ZIP_PARAMERROR,
                fmt("file {} exists and neither overwrite nor append are configured", path.string()));
    }

    m_handle = zipOpen64(path.string().c_str(), mode);

    if (is_open())
        return {};

    return mux(ZIP_INTERNALERROR, fmt("could not open file {}: ", path.string()));
}


ex<void, error<zip>> zip::close() {
    if (auto res = zipClose(m_handle, nullptr); res != ZIP_OK)
        return mux(res, "could not close zip-file ({})");
    return {};
}

} // namespace mzpp
