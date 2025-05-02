// MIT License
// Copyright (c) 2024-2025 Tomáš Mark

#include "MyDpp/MyDpp.hpp"
#include "Logger/Logger.hpp"
#include "Utils/Utils.hpp"

#include <cxxopts.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace DotNameUtils;

namespace AppContext {
  constexpr char standaloneName[] = "MyDppApp";
  const std::filesystem::path standalonePath
      = PathUtils::getParentPath (PathUtils::getStandalonePath ());
  constexpr std::string_view utilsAssetPath = UTILS_ASSET_PATH;
  constexpr std::string_view utilsFirstAssetFile = UTILS_FIRST_ASSET_FILE;
  const std::filesystem::path assetsPath = standalonePath / utilsAssetPath;
  const std::filesystem::path assetsPathFirstFile = assetsPath / utilsFirstAssetFile;
}

std::unique_ptr<dotname::MyDpp> uniqueLib;

int handlesArguments (int argc, const char* argv[]) {
  try {
    auto options = std::make_unique<cxxopts::Options> (argv[0], AppContext::standaloneName);
    options->positional_help ("[optional args]").show_positional_help ();
    options->set_width (80);
    options->set_tab_expansion ();
    options->add_options () ("h,help", "Show help");
    options->add_options () ("1,omit", "Omit library loading",
                             cxxopts::value<bool> ()->default_value ("false"));
    options->add_options () ("2,log2file", "Log to file",
                             cxxopts::value<bool> ()->default_value ("false"));
    const auto result = options->parse (argc, argv);

    if (result.count ("help")) {
      LOG_I_STREAM << options->help ({ "", "Group" }) << std::endl;
      return 0;
    }

    if (result["log2file"].as<bool> ()) {
      LOG.enableFileLogging (std::string (AppContext::standaloneName) + ".log");
      LOG_D_STREAM << "Logging to file enabled [-2]" << std::endl;
    }

    if (!result.count ("omit")) {
      // uniqueLib = std::make_unique<dotname::DotNameLib> ();
      uniqueLib = std::make_unique<dotname::MyDpp> (AppContext::assetsPath);
    } else {
      LOG_D_STREAM << "Loading library omitted [-1]" << std::endl;
    }

    if (!result.unmatched ().empty ()) {
      for (const auto& arg : result.unmatched ()) {
        LOG_E_STREAM << "Unrecognized option: " << arg << std::endl;
      }
      LOG_I_STREAM << options->help () << std::endl;
      return 1;
    }

  } catch (const cxxopts::exceptions::exception& e) {
    LOG_E_STREAM << "error parsing options: " << e.what () << std::endl;
    return 1;
  }
  return 0;
}

// unused right now
int printAssets (const std::filesystem::path& assetsPath) {
  try {
    auto files = FileManager::listFiles (assetsPath);
    if (files.empty ()) {
      LOG_D_STREAM << "No assets found in " << assetsPath << std::endl;
      return 0;
    }

    for (const auto& file : files) {
      LOG_D_STREAM << "╰➤ " << file << std::endl;
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what () << std::endl;
    return 1;
  }
  return 0;
}

int runApp (int argc, const char* argv[]) {

  LOG.noHeader (true);
  LOG.setSkipLine (false);
  LOG_I_STREAM << "Starting " << AppContext::standaloneName << " ..." << std::endl;

#ifdef EMSCRIPTEN
  LOG_I_STREAM << "╰➤ C++ Running in Emscripten environment" << std::endl;
#endif
#ifdef __EMSCRIPTEN_PTHREADS__
  LOG_I_STREAM << " ⤷ Emscripten C++ with pthreads support" << std::endl;
#endif

  if (handlesArguments (argc, argv) != 0) {
    return 1;
  }

  Performance::simpleCpuBenchmark ();

  // I know it is smartpointer, but we need to free it before exit scope bracelet
  uniqueLib = nullptr;

  // bye
  LOG_I_STREAM << "Sucessfully exited " << AppContext::standaloneName << std::endl;
  return 0;
}