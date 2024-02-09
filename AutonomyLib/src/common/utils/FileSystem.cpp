// in header only mode, control library is not available
#ifndef AUTONOMYLIB_HEADER_ONLY

#include "common/utils/FileSystem.hpp"
#include "common/utils/Utils.hpp"
#include <codecvt>
#include <cstdio>
#include <fstream>
#include <string>

#if defined _WIN32 || defined _WIN64

#include "common/utils/MinWinDefines.hpp"
#include "common/utils/WindowsApisCommonPre.hpp"

#undef NOWINMESSAGES // WM_*, EM_*, LB_*, CB_*
#undef NOCTLMGR      // Control and Dialog routines
#undef NOGDI         // All GDI #undefs and routines
#undef NOKERNEL      // All KERNEL #undefs and routines
#undef NOUSER        // All USER #undefs and routines
#undef NOMSG         // typedef MSG and associated routines

// #include <fileapi.h>
#include <Shlobj.h> // Windows shell object
#include <direct.h> // Windows file system directory manipulation
#include <stdlib.h> // C standard library

#include "common/utils/WindowsApisCommonPost.hpp"

#else
#include <errno.h>
#include <sys/param.h> // C POSIX library, parameters (MAXPATHLEN definition)
#include <sys/stat.h>  // C POSIX library, file attributes (get mkdir)
#include <sys/types.h> // C POSIX library, types
#include <unistd.h>    // C POSIX universal operating system API
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h> // MacOS Mach kernel, dynamic loader
#endif

namespace common_utils {

// File names are unicode (std::wstring), because users can create folders containing unicode characters on both
// Windows, OSX and Linux.
std::string FileSystem::createDirectory(const std::string &fullPath) {

#ifdef _WIN32
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::wstring wide_path = converter.from_bytes(fullPath);
    int hr = CreateDirectoryW(wide_path.c_str(), NULL);
    if (hr == 0) {
        hr = GetLastError();
        if (hr != ERROR_ALREADY_EXISTS) {
            throw std::invalid_argument(Utils::stringf("Error creating directory, hr=%d", hr));
        }
    }
#else
    int success = mkdir(fullPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (success != 0 && errno != EEXIST)
        throw std::ios_base::failure(Utils::stringf("mkdir failed for path %s with errorno %i and message %s",
                                                    fullPath.c_str(), errno, strerror(errno))
                                         .c_str());
#endif
    return fullPath;
}

std::string FileSystem::getUserDocumentsFolder() {
    std::string path;
#ifdef _WIN32
    // Windows users can move the Documents folder to any location they want
    // SHGetFolderPath knows how to find it.
    wchar_t szPath[MAX_PATH];

    if (0 == SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS | CSIDL_FLAG_CREATE, NULL, 0, szPath)) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        path = converter.to_bytes(szPath);
    }

// fall back in case SHGetFolderPath failed for some reason.
#endif
    if (path == "") {
        path = combine(getUserHomeFolder(), "Documents");
    }
    return ensureFolder(path);
}

std::string FileSystem::getExecutableFolder() {
    std::string path;
#ifdef _WIN32
    wchar_t szPath[MAX_PATH];

    HMODULE hModule = GetModuleHandle(NULL);

    if (NULL != hModule) {
        if (0 < GetModuleFileName(hModule, szPath, sizeof(szPath))) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            path = converter.to_bytes(szPath);
        } else {
            HRESULT hr = GetLastError();
            throw std::invalid_argument(Utils::stringf("Error getting executable folder, hr = %d", hr));
        }
    } else {
        HRESULT hr = GetLastError();
        throw std::invalid_argument(
            Utils::stringf("Error getting executable folder - hModule is null. Last error = %d", hr));
    }
#elif defined(__APPLE__)
    char szPath[8192];
    uint32_t size = sizeof(szPath);

    if (_NSGetExecutablePath(szPath, &size) != 0) {
        throw std::invalid_argument("Error getting executable folder, path is too long");
    }

    path = std::string(szPath);
#else
    char szPath[8192];
    readlink("/proc/self/exe", szPath, sizeof(szPath));
    path = std::string(szPath);
#endif

    size_t pathSeparatorIndex = path.find_last_of(kPathSeparator);
    path = path.substr(0, pathSeparatorIndex);

    return ensureFolder(path);
}

} // namespace common_utils

#endif