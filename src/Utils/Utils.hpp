#ifndef UTILS_HPP
#define UTILS_HPP

// MIT License
// Copyright (c) 2024-2025 Tomáš Mark

#include "Logger/Logger.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// fullfilled from ../cmake/tmplt-assets.cmake)
#ifndef UTILS_ASSET_PATH
  #define UTILS_ASSET_PATH ""
#endif
// fullfilled from ../cmake/tmplt-assets.cmake)
#ifndef UTILS_FIRST_ASSET_FILE
  #define UTILS_FIRST_ASSET_FILE ""
#endif
// fullfilled from ../cmake/tmplt-assets.cmake)
#ifndef UTILS_ASSET_FILES_DIVIDED_BY_COMMAS
  #define UTILS_ASSET_FILES_DIVIDED_BY_COMMAS ""
#endif

// We need to avoid conflicts with other libraries
#ifdef _WIN32
  #include <windows.h>
#elif defined(__APPLE__)
  #include <limits.h>
  #include <mach-o/dyld.h>
#else // Linux
  #include <limits.h>
  #include <unistd.h>
#endif

namespace Utils {
  namespace FSManager {

    inline std::string read (const std::string& filename) {
      std::ifstream file;
      file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
      std::stringstream file_stream;
      try {
        file.open (filename.c_str ());
        file_stream << file.rdbuf ();
        file.close ();
      } catch (const std::ifstream::failure& e) {
        LOG_E_STREAM << e.what () << std::endl;
      }
      return file_stream.str ();
    }

    // overload read to use std::filesystem::path (Windows requires this)
    inline std::string read (const std::filesystem::path& filePath) {
      return read (filePath.string ());
    }

    inline std::string getExecutePath () {
      std::string path;
#ifdef _WIN32
      char buffer[MAX_PATH];
      GetModuleFileNameA (NULL, buffer, MAX_PATH);
      path = buffer;
      size_t pos = path.find_last_of ("\\/");
      if (pos != std::string::npos) {
        path = path.substr (0, pos);
      }
#elif defined(__APPLE__)
      char buffer[PATH_MAX];
      uint32_t bufferSize = PATH_MAX;
      if (_NSGetExecutablePath (buffer, &bufferSize) == 0) {
        char realPath[PATH_MAX];
        if (realpath (buffer, realPath) != nullptr) {
          path = realPath;
          size_t pos = path.find_last_of ("/");
          if (pos != std::string::npos) {
            path = path.substr (0, pos);
          }
        }
      }
#else
      char buffer[PATH_MAX];
      ssize_t count = readlink ("/proc/self/exe", buffer, PATH_MAX);
      if (count != -1) {
        buffer[count] = '\0';
        path = buffer;
        size_t pos = path.find_last_of ("/");
        if (pos != std::string::npos) {
          path = path.substr (0, pos);
        }
      }
#endif

      return path;
    }

  }; // namespace FSManager

  namespace StringUtils {

    inline std::string trim (const std::string& str) {
      size_t first = str.find_first_not_of (" \t\n\r\f\v");
      size_t last = str.find_last_not_of (" \t\n\r\f\v");
      return (first == std::string::npos) ? "" : str.substr (first, (last - first + 1));
    }

    inline std::vector<std::string> split (const std::string& str, char delimiter) {
      std::vector<std::string> tokens;
      std::stringstream ss (str);
      std::string token;
      while (std::getline (ss, token, delimiter)) {
        tokens.push_back (trim (token));
      }
      return tokens;
    }

  } // namespace StringUtils

} // namespace Utils

#endif // UTILS_HPP