// MIT License
// Copyright (c) 2024-2025 Tomáš Mark

#ifndef __MYDPP_HPP
#define __MYDPP_HPP

#include <EmojiTools/EmojiTools.hpp>
#include <Sunriset/Sunriset.hpp>
#include <MyDpp/version.h>
#include <dpp/dpp.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <random>
#include <string>

// Public API

namespace dotname {

  class MyDpp {

    const std::string libName = std::string ("MyDpp v.") + MYDPP_VERSION;
    std::filesystem::path assetsPath_;

  public:
    MyDpp ();
    MyDpp (const std::filesystem::path& assetsPath);
    ~MyDpp ();

    const std::filesystem::path getAssetsPath () const {
      return assetsPath_;
    }
    void setAssetsPath (const std::filesystem::path& assetsPath) {
      assetsPath_ = assetsPath;
    }

    std::string getEnvironmentInfo ();
    bool loadVariousBotCommands ();
    bool startPollingSunriset ();
    bool startPollingEmojies ();
    bool startPollingFortune ();
    bool startPollingBTCPrice ();
    bool startPollingCZExchRate ();
    bool startPollingGetBibleVerse ();

    bool welcomeWithFastfetch ();
    bool welcomeWithNeofetch ();
    bool initCluster ();
    bool getToken (std::string& token, const std::string& filePath);

    std::string getLinuxFortuneCpp ();
    std::string getLinuxFastfetchCpp ();
    std::string getLinuxNeofetchCpp ();
    std::string getBitcoinPrice ();
    std::string getCzechBibleVerse ();
    std::string getCzechExchangeRate ();
    std::string getCurrentTime ();
    std::string getSunriset ();
    int getRandom (int min, int max);

  private:
    std::unique_ptr<dpp::cluster> m_bot;
    std::shared_ptr<dotname::EmojiTools> emojiTools;
    std::shared_ptr<dotname::Sunriset> sunrisetTools;
    std::string emoji;
  };

} // namespace dotname

#endif // __MYDPP_HPP