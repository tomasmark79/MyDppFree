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
#include <vector>

// Public API

namespace dotname {

  class MyDpp {

    const std::string libName = std::string ("MyDpp v.") + MYDPP_VERSION;
    std::filesystem::path assetsPath_;

  public:
    // First define the structures
    struct RSSItem {
      std::string title;
      std::string link;
      std::string description;
      std::string pubDate; // pro datum publikace
      std::string guid;    // pro jedinečný identifikátor

      // Constructor
      RSSItem () = default;
      RSSItem (const std::string& t, const std::string& l, const std::string& d,
               const std::string& date = "", const std::string& id = "")
          : title (t), link (l), description (d), pubDate (date), guid (id) {
      }
    };

    struct RSSFeed {
      std::string title;
      std::string description;
      std::string link;
      std::vector<RSSItem> items;

      // Methods for manipulation
      void addItem (const RSSItem& item) {
        items.push_back (item);
      }

      size_t getItemCount () const {
        return items.size ();
      }

      std::string getTitle () const {
        return title;
      }
      std::string getDescription () const {
        return description;
      }
      std::string getLink () const {
        return link;
      }
      std::vector<RSSItem> getItems () const {
        return items;
      }


      std::string toString () const;
    };

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

    RSSFeed feedRootCz;
    std::string getRootcz ();
    std::string parseRSS (const std::string& xmlData);
    int getRandom (int min, int max);

    RSSFeed parseRSSToStruct (const std::string& xmlData);

  private:
    std::unique_ptr<dpp::cluster> m_bot;
    std::shared_ptr<dotname::EmojiTools> emojiTools;
    std::shared_ptr<dotname::Sunriset> sunrisetTools;
    std::string emoji;
  };

} // namespace dotname

#endif // __MYDPP_HPP