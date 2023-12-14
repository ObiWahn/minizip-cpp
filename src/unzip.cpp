#include <minizip-cpp/mzpp.hpp>

namespace mzpp {

ex<void, error<unzip>> unzip::open(fs::path const& path) {
    if (is_open()) {
        if (auto ok = close(); !ok)
            return mux(std::move(ok));
    }

    std::error_code ec;
    if (!fs::exists(path, ec))
        return mux(UNZ_PARAMERROR, fmt("'{}' not found", path.string()));

    if (!fs::is_regular_file(path, ec) || (!fs::is_directory(path, ec) && fs::is_symlink(path, ec)))
        return mux(UNZ_PARAMERROR, fmt("'{}' is not a file", path.string()));

    m_handle = unzOpen(path.string().c_str());

    if (auto err = unzGetGlobalInfo(m_handle, &m_global_info); err != UNZ_OK) {
        return mux(err, "could not get global info");
    }

    if (is_open())
        return {};

    return mux(UNZ_PARAMERROR, fmt("could not open file {}: ", path.string()));
}


ex<void, error<unzip>> unzip::close() {
    m_global_info = {0};
    if (auto err = unzClose(m_handle); err != UNZ_OK) {
        return mux(err, "could not close file");
    }
    return {};
}


ex<entry_list, error<unzip>> unzip::list() {
    entry_list rv;
    rv.reserve(m_global_info.number_entry);

    if (m_global_info.number_entry == 0)
        return rv;

    if (auto err = unzGoToFirstFile(m_handle); err != UNZ_OK)
        return mux(err, fmt("error in unzGoToFirstFile"));

    for (entry_index i = 0; i < m_global_info.number_entry;) {
        if (auto res = get_current_info(i); res)
            rv.push_back(std::move(res).value());
        else
            return mux(std::move(res));

        if (++i >= m_global_info.number_entry)
            break;

        if (auto err = unzGoToNextFile(m_handle); err != UNZ_OK)
            return mux(err, fmt("error in unzGoToNextFile at entry {}", i));
    }
    return rv;
}


// private


ex<void, error<unzip>> unzip::select_entry(entry_index wanted) {
    if (m_global_info.number_entry == 0)
        return mux(UNZ_BADZIPFILE, "error no files in zip");

    if (auto err = unzGoToFirstFile(m_handle); err != UNZ_OK)
        return mux(err, "error in unzGoToFirstFile");

    entry_index current = 0;
    while (current < m_global_info.number_entry) {
        if (wanted == current)
            break;
        if (++current >= m_global_info.number_entry)
            break;
        if (auto err = unzGoToNextFile(m_handle); err != UNZ_OK)
            return mux(err, fmt("error in unzGoToNextFile at entry {}", current));
    }

    if (wanted != current)
        return mux(UNZ_INTERNALERROR, fmt("error index {} not in file", wanted));

    return {};
}


ex<entry_desc, error<unzip>> unzip::get_current_info(entry_index i) {
    unz_file_info64 info = {0};
    char filename_inzip[256];

    if (auto err = unzGetCurrentFileInfo64(m_handle, &info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
        err != UNZ_OK)
        return mux(err, fmt("error in unzGetCurrentFileInfo for entry {}", i));

    filename_inzip[255] = '\0';
    return entry_desc{std::string(filename_inzip), info};
}


ex<entry_desc, error<unzip>> unzip::open_entry(entry_index wanted, char const* password) {
    if (auto res = select_entry(wanted); !res)
        return mux(std::move(res));

    auto rv = get_current_info(wanted);
    if (!rv)
        return rv;

    if (auto err = unzOpenCurrentFilePassword(m_handle, password); UNZ_OK != err)
        return mux(err, fmt("error in unzOpenCurrentFilePassword at entry {}", wanted));

    return rv;
}

} // namespace mzpp
