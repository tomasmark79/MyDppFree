// MIT License
// Copyright (c) 2024-2025 Tomáš Mark

#include <Logger/Logger.hpp>
#include <MyDpp/MyDpp.hpp>
#include <Utils/Utils.hpp>

#include <curl/curl.h>
#include <fmt/format.h>

#include <array>
#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include "tinyxml2.h"

// TODO
// pooling required better design pattern

#ifdef _WIN32
  #include <cstdio>
  #define popen _popen
  #define pclose _pclose
#else
  #include <cstdio>
#endif

#define EMOJI_INTERVAL_SEC (int)10

#define DISCORD_OAUTH_TOKEN_FILE "/home/tomas/.tokens/.discord_oauth.key"

#define URL_COIN_GECKO                       \
  "https://api.coingecko.com/api/v3/simple/" \
  "price?ids=bitcoin&vs_currencies=usd"

#define URL_EXCHANGE_RATES_CZ                                               \
  "https://www.cnb.cz/cs/financni-trhy/devizovy-trh/kurzy-devizoveho-trhu/" \
  "kurzy-devizoveho-trhu/denni_kurz.txt"

const dpp::snowflake channelDev = 1327591560065449995;

std::atomic<bool> isRefreshSunrisetRunning (false);
#define REGULAR_REFRESH_SUNRISET_MESSAGE_INTERVAL_SEC (int)43200 * 2 // 24 hours
std::atomic<bool> stopRefreshSunriset (false);

std::atomic<bool> isRefreshEmojiesRunning (false);
#define REGULAR_REFRESH_EMOJIES_MESSAGE_INTERVAL_SEC (int)10 // CCA 3 hours
std::atomic<bool> stopRefreshEmojies (false);

#define REGULAR_REFRESH_MESSAGE_INTERVAL_SEC (int)10800 // 3 hours
std::atomic<bool> stopRefreshMessageThread (false);

#define BITCOIN_PRICE_MESSAGE_INTERVAL_SEC (int)43200 * 2 // 24 hours
std::atomic<bool> stopGetBitcoinPrice (false);

#define GITHUB_INFO_MESSAGE_INTERVAL_SEC (int)43200 // 12 hours
std::atomic<bool> stopGetGithubInfo (false);

#define CZECH_EXCHANGERATES_MESSAGE_INTERVAL_SEC (int)43200 * 2 // 24 hours
std::atomic<bool> stopGetCzechExchangeRates (false);

#define GITHUB_EVENT_POLLING_INTERVAL_SEC (int)10
std::atomic<bool> stopGithubEventPooling (false);

std::atomic<bool> isGetBibleVerseRunning (false);
#define CZECH_BIBLE_VERSE_POLLING_INTERVAL_SEC (int)43200 // 12 hours
std::atomic<bool> stopGetCzechBibleVersePooling (false);

namespace dotname {

  MyDpp::MyDpp () {
    LOG_D_STREAM << libName << " ...constructed" << std::endl;
    if (!assetsPath_.empty ()) {
      LOG_D_STREAM << "Assets path: " << assetsPath_ << std::endl;
    } else {
      LOG_D_STREAM << "Assets path is empty" << std::endl;
    }
  }
  MyDpp::MyDpp (const std::filesystem::path& assetsPath) : MyDpp () {
    assetsPath_ = assetsPath;

    this->emojiTools = std::make_shared<dotname::EmojiTools> (assetsPath_);
    this->sunrisetTools = std::make_shared<dotname::Sunriset> ();

    this->initCluster ();
  }
  MyDpp::~MyDpp () {
    LOG_D_STREAM << libName << " ...destructed" << std::endl;
  }

  bool MyDpp::initCluster () {
    std::string token;
    if (getToken (token, DISCORD_OAUTH_TOKEN_FILE)) {
      try {
        if (m_bot) {
          LOG_E_STREAM << "Bot is already initialized!" << std::endl;
          return false;
        }

        // RedHats childs needs for failed ssl contexts in
        // in /etc/ssl/openssl.cnf
        // https://github.com/openssl/openssl/discussions/23016
        // config_diagnostics = 1 to config_diagnostics = 0

        MyDpp::m_bot = std::make_unique<dpp::cluster> (token, dpp::i_default_intents
                                                                  | dpp::i_message_content);

        m_bot->log (dpp::ll_debug, "DSDotBot");

        std::string message = this->getEnvironmentInfo ();
        dpp::message msg (channelDev, message);
        m_bot->message_create (msg);
        LOG_I_STREAM << message << std::endl;

        welcomeWithFastfetch ();
        startPollingFortune ();
        startPollingBTCPrice ();
        startPollingCZExchRate ();
        startPollingGetBibleVerse ();
        startPollingSunriset ();
        loadVariousBotCommands ();

        m_bot->start (dpp::st_wait);
      }

      catch (const std::exception& e) {
        LOG_E_STREAM << "Exception during bot initialization: " << e.what () << std::endl;
        return false;
      }
    }
    return true;
  }

  std::string MyDpp::getEnvironmentInfo () {
    return std::string ("C++ DSDotBot 🛸🛸🛸 ") + DPP_VERSION_TEXT + " loaded.\n";
  }

  bool MyDpp::welcomeWithFastfetch () {
    // DSDotBot loaded
    m_bot->on_ready ([&] (const dpp::ready_t& event) {
      try {
        dpp::message msgFastfetch (channelDev,
                                   this->getLinuxFastfetchCpp ().substr (0, 8192 - 2) + "\n");
        m_bot->message_create (msgFastfetch);
        LOG_I_STREAM << msgFastfetch.content << std::endl;
      } catch (const std::runtime_error& e) {
        LOG_E_STREAM << "Error: " << e.what () << std::endl;
      }
    });
    return true;
  }

  bool MyDpp::welcomeWithNeofetch () {
    // DSDotBot loaded
    m_bot->on_ready ([&] (const dpp::ready_t& event) {
      try {
        dpp::message msgNeofetch (channelDev, this->getLinuxNeofetchCpp ().substr (0, 1998) + "\n");
        m_bot->message_create (msgNeofetch);
        LOG_I_STREAM << msgNeofetch.content << std::endl;
      } catch (const std::runtime_error& e) {
        LOG_E_STREAM << "Error: " << e.what () << std::endl;
      }
    });
    return true;
  }

  bool MyDpp::startPollingSunriset () {
    {
      std::thread threadRegularSunriset ([&] () -> void {
        while (!stopRefreshSunriset.load ()) {
          try {
            std::string message = getSunriset ();
            dpp::message msg (channelDev, message);
            m_bot->message_create (msg);
            isRefreshSunrisetRunning.store (true);
          } catch (const std::runtime_error& e) {
            LOG_E_STREAM << "Error: " << e.what () << std::endl;
            isRefreshSunrisetRunning.store (false);
          }
          std::this_thread::sleep_for (
              std::chrono::seconds (REGULAR_REFRESH_SUNRISET_MESSAGE_INTERVAL_SEC));
        }
      });
      threadRegularSunriset.detach ();
    }
    return true;
  }

  bool MyDpp::startPollingGetBibleVerse () {
    {
      std::thread threadRegularGetBibleVerse ([&] () -> void {
        while (!stopGetCzechBibleVersePooling.load ()) {
          try {
            std::string message = getCzechBibleVerse ();
            dpp::message msg (channelDev, message);
            m_bot->message_create (msg);
            isGetBibleVerseRunning.store (true);
          } catch (const std::runtime_error& e) {
            LOG_E_STREAM << "Error: " << e.what () << std::endl;
            isGetBibleVerseRunning.store (false);
          }
          std::this_thread::sleep_for (
              std::chrono::seconds (CZECH_BIBLE_VERSE_POLLING_INTERVAL_SEC));
        }
      });
      threadRegularGetBibleVerse.detach ();
    }
    return true;
  }

  bool MyDpp::startPollingEmojies () {
    {
      std::thread threadRegularRefreshEmojiesMessage ([&] () -> void {
        while (!stopRefreshEmojies.load ()) {
          try {
            std::string message = emojiTools->getRandomEmoji ();
            // LOG_D << message << std::endl;
            dpp::message msg (channelDev, message);
            m_bot->message_create (msg);
            isRefreshEmojiesRunning.store (true);
          } catch (const std::runtime_error& e) {
            LOG_E_STREAM << "Error: " << e.what () << std::endl;
            isRefreshEmojiesRunning.store (false);
          }

          std::this_thread::sleep_for (
              std::chrono::seconds (REGULAR_REFRESH_EMOJIES_MESSAGE_INTERVAL_SEC));
        }
      });
      threadRegularRefreshEmojiesMessage.detach ();
    }
    return true;
  }

  bool MyDpp::startPollingFortune () {
    m_bot->on_ready ([&] (const dpp::ready_t& event) {
      std::thread threadRegularRefreshMessage ([&] () -> void {
        while (!stopRefreshMessageThread.load ()) {
          try {
            std::string message = getLinuxFortuneCpp ();
            // LOG_D_STREAM << message << std::endl;
            dpp::message msg (channelDev, "Quote\n\t" + message);
            m_bot->message_create (msg);
          } catch (const std::runtime_error& e) {
            LOG_E_STREAM << "Error: " << e.what () << std::endl;
          }

          std::this_thread::sleep_for (std::chrono::seconds (REGULAR_REFRESH_MESSAGE_INTERVAL_SEC));
        }
      });
      threadRegularRefreshMessage.detach ();
    });
    return true;
  }

  bool MyDpp::startPollingBTCPrice () {
    m_bot->on_ready ([&] (const dpp::ready_t& event) {
      std::thread threadBitcoinPriceMessage ([&] () -> void {
        while (!stopGetBitcoinPrice.load ()) {
          try {
            std::string message = getBitcoinPrice ();
            // LOG_D_STREAM << message << std::endl;
            dpp::message msg (channelDev, "\n🪙 " + message);
            m_bot->message_create (msg);
          } catch (const std::runtime_error& e) {
            LOG_E_STREAM << "Error: " << e.what () << std::endl;
          }

          std::this_thread::sleep_for (std::chrono::seconds (BITCOIN_PRICE_MESSAGE_INTERVAL_SEC));
        }
      });
      threadBitcoinPriceMessage.detach ();
    });
    return true;
  }

  // TODO Draw as bitmap table will looks better 😎
  bool MyDpp::startPollingCZExchRate () {
    m_bot->on_ready ([&] (const dpp::ready_t& event) {
      std::thread threadCzechExchangeRateMessage ([&] () -> void {
        while (!stopGetCzechExchangeRates.load ()) {
          try {
            std::string message = getCzechExchangeRate ();
            // LOG_D_STREAM << message << std::endl;
            dpp::message msg (channelDev, "Czech Exchange Rates 🇨🇿\n" + message);
            m_bot->message_create (msg);
          } catch (const std::runtime_error& e) {
            LOG_E_STREAM << "Error: " << e.what () << std::endl;
          }

          std::this_thread::sleep_for (
              std::chrono::seconds (CZECH_EXCHANGERATES_MESSAGE_INTERVAL_SEC));
        }
      });
      threadCzechExchangeRateMessage.detach ();
    });

    return true;
  }

  bool MyDpp::getToken (std::string& token, const std::string& filePath) {
    std::ifstream file (filePath);
    if (!file.is_open ()) {
      LOG_E_STREAM << "Error: Could not open file " << filePath << std::endl;
      return false;
    }
    std::getline (file, token);
    file.close ();
    if (token.empty ()) {
      LOG_E_STREAM << "Error: Token is empty" << std::endl;
      return false;
    }
    return true;
  }

  std::string MyDpp::getLinuxFortuneCpp () {
    constexpr size_t bufferSize = 2000;
    std::stringstream result;

    // Create unique_ptr with custom deleter for RAII
    std::unique_ptr<FILE, decltype (&pclose)> pipe (popen ("fortune", "r"), pclose);
    if (!pipe)
      throw std::runtime_error ("Failed to run fortune command");

    std::array<char, bufferSize> buffer;
    while (fgets (buffer.data (), buffer.size (), pipe.get ()) != nullptr) {
      result << buffer.data ();
      if (result.str ().size () > bufferSize - 2)
        break;
    }

    return result.str ();
  }

  std::string MyDpp::getLinuxFastfetchCpp () {
    constexpr size_t bufferSize = 2000;
    std::stringstream result;

    // Create unique_ptr with custom deleter for RAII
    std::unique_ptr<FILE, decltype (&pclose)> pipe (
        popen ("fastfetch -c archey.jsonc --pipe --logo none", "r"), pclose);
    if (!pipe)
      throw std::runtime_error ("Failed to run fastfetch command");

    std::array<char, bufferSize> buffer;
    while (fgets (buffer.data (), buffer.size (), pipe.get ()) != nullptr) {
      result << buffer.data ();
      if (result.str ().size () > bufferSize - 2)
        break;
    }

    return result.str ();
  }

  std::string MyDpp::getLinuxNeofetchCpp () {
    constexpr size_t bufferSize = 2000;
    std::stringstream result;

    // Create unique_ptr with custom deleter for RAII
    std::unique_ptr<FILE, decltype (&pclose)> pipe (popen ("neofetch --stdout", "r"), pclose);
    if (!pipe)
      throw std::runtime_error ("Failed to run neofetch command");

    std::array<char, bufferSize> buffer;
    while (fgets (buffer.data (), buffer.size (), pipe.get ()) != nullptr) {
      result << buffer.data ();
      if (result.str ().size () > bufferSize - 2)
        break;
    }

    return result.str ();
  }

  size_t WriteCallback (void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append ((char*)contents, size * nmemb);
    return size * nmemb;
  }

  std::string MyDpp::getBitcoinPrice () {
    CURL* curl;
    CURLcode res;
    std::string rawTxtBuffer;
    curl = curl_easy_init ();
    if (curl) {
      curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0L); /* temporary - todo cert */
      curl_easy_setopt (curl, CURLOPT_URL, URL_COIN_GECKO);
      curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt (curl, CURLOPT_WRITEDATA, &rawTxtBuffer);

      res = curl_easy_perform (curl);

      if (res != CURLE_OK) {
        LOG_E_STREAM << "curl_easy_perform() failed: " << curl_easy_strerror (res) << std::endl;
      } else {
        // use lohmann json to parse the response
        /*
              {
                  "bitcoin": {
                      "usd": 95802
                  }
              }
              */

        nlohmann::json j = nlohmann::json::parse (rawTxtBuffer);
        std::string usd = j["bitcoin"]["usd"].dump ();

        LOG_D_STREAM << "Downloaded content:\n" << rawTxtBuffer << std::endl;
        std::string message = "1 BTC = " + usd + " USD";
        LOG_I_STREAM << message << std::endl;

        return message;
      }

      curl_easy_cleanup (curl);
    }
    return "Error: Could not get the Bitcoin price!";
  }

  std::string MyDpp::getCzechBibleVerse () {
    std::string message = ""; // "📖 Czech Bible Verse 📖\n";
    std::string bibleChapter;
    std::string bibleVerse;
    std::string line;
    std::ifstream bibleFile (std::string (assetsPath_) + "/" + "kralicky.txt");
    std::vector<std::pair<std::string, std::string> > bible;

    try {
      if (bibleFile.is_open ()) {
        std::vector<std::string> lines;
        while (std::getline (bibleFile, line)) {
          lines.push_back (line);
        }
        bibleFile.close ();
        bibleVerse = lines[0] + "\n";
        for (int i = 0; i < lines.size (); ++i) {
          if (!lines[i].empty () && isdigit (lines[i][0])) {
            bibleVerse = lines[i] + "\n";
          } else if (lines[i].empty () && (i + 1) < lines.size () && !lines[i + 1].empty ()
                     && lines[i + 2].empty ()) {
            bibleChapter = lines[i + 1];
            bible.push_back (std::make_pair (bibleChapter, bibleVerse));
          }
        }
      } else {
        LOG.error ("Error: Could not open file kralicky.txt");
        return "Error: Could not get the Czech Bible verse!";
      }
    } catch (const std::exception& e) {
      LOG_E_STREAM << e.what () << std::endl;
    }

    int randomIndex = getRandom (0, bible.size () - 1);
    bibleVerse = bible[randomIndex].second;
    bibleChapter = bible[randomIndex].first;
    message += bibleChapter + "\n" + bibleVerse;
    return "📖 " + message;
  }

  std::string MyDpp::getCzechExchangeRate () {
    CURL* curl;
    CURLcode res;
    std::string rawTxtBuffer;
    curl = curl_easy_init ();
    if (curl) {
      curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0L); /* temporary - todo cert */
      curl_easy_setopt (curl, CURLOPT_URL, URL_EXCHANGE_RATES_CZ);
      curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt (curl, CURLOPT_WRITEDATA, &rawTxtBuffer);

      res = curl_easy_perform (curl);

      if (res != CURLE_OK) {
        LOG_E_STREAM << "curl_easy_perform() failed: " << curl_easy_strerror (res) << std::endl;
      } else {
        // replace char "|" with "\t"
        std::replace (rawTxtBuffer.begin (), rawTxtBuffer.end (), '|', '\t');
        // LOG_D_STREAM << "Downloaded content:\n" << rawTxtBuffer << std::endl;
        return rawTxtBuffer;
      }
      curl_easy_cleanup (curl);
    }

    return "Error: Could not get the Czech exchange rate!";
  }

  std::string MyDpp::getSunriset () {
    std::string today = getCurrentTime ();
    std::string year = today.substr (0, 4);
    std::string month = today.substr (5, 2);
    std::string day = today.substr (8, 2);
    int yearInt = std::stoi (year);
    int monthInt = std::stoi (month);
    int dayInt = std::stoi (day);
    double rise = 0.0;
    double set = 0.0;
    sunrisetTools->getSunriset (yearInt, monthInt, dayInt, 14.265802152828646, 49.86396819090531,
                                rise, set);
    return "Sunrise " + sunrisetTools->doubleTo24Time (rise + 2) + " and sunset "
           + sunrisetTools->doubleTo24Time (set + 2) + " for today!";
  }

  std::string MyDpp::parseRSS (const std::string& xmlData) {
    LOG_D_STREAM << "Parsing RSS feed..." << std::endl;
    LOG_D_STREAM << "XML data:\n" << xmlData << std::endl;

    std::string message = "RSS feed:\n";
    tinyxml2::XMLDocument doc;
    doc.Parse (xmlData.c_str ());

    tinyxml2::XMLElement* channel = doc.FirstChildElement ("rss")->FirstChildElement ("channel");
    if (channel == nullptr) {
      LOG_E_STREAM << "Error: RSS feed is not valid." << std::endl;
      return "";
    }
    tinyxml2::XMLElement* item = channel->FirstChildElement ("item");
    while (item) {
      const char* title = item->FirstChildElement ("title")->GetText ();
      const char* link = item->FirstChildElement ("link")->GetText ();
      const char* description = item->FirstChildElement ("description")->GetText ();

      if (title && link && description) {
        LOG_I_STREAM << "Title: " << title << std::endl;
        LOG_I_STREAM << "Link: " << link << std::endl;
        LOG_I_STREAM << "Description: " << description << std::endl;
        LOG_I_STREAM << "------------------------" << std::endl;

        message += "Title: " + std::string (title) + "\n";
        message += "Link: " + std::string (link) + "\n";
        message += "Description: " + std::string (description) + "\n";
        message += "------------------------\n";
      }
      item = item->NextSiblingElement ("item");
    }
    return message;
  }

  std::string MyDpp::getRootcz () {
    std::string msg = "";
    std::string msgFinal = "";
    CURL* curl;
    CURLcode res;
    std::string rawTxtBuffer;
    curl = curl_easy_init ();
    if (curl) {
      curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0L); /* temporary - todo cert */
      curl_easy_setopt (curl, CURLOPT_URL, "https://www.root.cz/rss/clanky/");
      curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt (curl, CURLOPT_WRITEDATA, &rawTxtBuffer);

      res = curl_easy_perform (curl);

      if (res != CURLE_OK) {
        LOG_E_STREAM << "curl_easy_perform() failed: " << curl_easy_strerror (res) << std::endl;
      } else {
        // LOG_D_STREAM << "Downloaded content:\n" << rawTxtBuffer << std::endl;
        parseRSSToStruct (rawTxtBuffer);
        for (const auto& item : feedRootCz.getItems ()) {
          LOG_I_STREAM << "Title: " << item.title << std::endl;
          LOG_I_STREAM << "Link: " << item.link << std::endl;
          msg = "[" + item.title + "](" + item.link + ")\n";
          if (msg.size () + msgFinal.size () < 2000) {
            msgFinal += msg;
          }
        }
        int len = msgFinal.length ();
        return msgFinal;
      }
      curl_easy_cleanup (curl);
    }
    return "Error: Could not get the Bitcoin price!";
  }

  bool MyDpp::loadVariousBotCommands () {

    m_bot->on_log ([&] (const dpp::log_t& log) {
      // std::cout << "[" << dpp::utility::current_date_time() << "] "
      //           << dpp::utility::loglevel(log.severity) << ": " <<
      //           log.message
      //           << std::endl;

      LOG_D_STREAM << "[" << dpp::utility::current_date_time () << "] "
                   << dpp::utility::loglevel (log.severity) << ": " << log.message << std::endl;
    });

    m_bot->on_slashcommand ([&, this] (const dpp::slashcommand_t& event) {
      if (event.command.get_command_name () == "verse") {
        std::string message = getCzechBibleVerse ();
        dpp::message msg (channelDev, message);
        event.reply (msg);
      }

      if (event.command.get_command_name () == "sunriset") {
        std::string message = getSunriset ();
        dpp::message msg (channelDev, message);
        event.reply (msg);
      }

      if (event.command.get_command_name () == "czk") {
        std::string message = getCzechExchangeRate ();
        dpp::message msg (channelDev, message);
        event.reply (msg);
      }

      if (event.command.get_command_name () == "btc") {
        std::string message = getBitcoinPrice ();
        dpp::message msg (channelDev, message);
        event.reply (msg);
      }

      if (event.command.get_command_name () == "fortune") {
        std::string message = getLinuxFortuneCpp ();
        dpp::message msg (channelDev, "Quote\n\t" + message);
        event.reply (msg);
      }

      if (event.command.get_command_name () == "noemojies") {
        if (!isRefreshEmojiesRunning.load ()) {
          dpp::message msg (channelDev, "Emojies are already stopped! 🛑");
          event.reply (msg);
          return;
        }
        event.reply ("Emojies are stopped! 🛑");
        isRefreshEmojiesRunning.store (false);
        stopRefreshEmojies.store (true);
      }

      if (event.command.get_command_name () == "emojies") {

        if (isRefreshEmojiesRunning.load ()) {
          dpp::message msg (channelDev, "Emojies already running! 🕒");
          event.reply (msg);
          return;
        }

        event.reply ("Emojies are being sent in regularly interval 10 seconds! 🕒");
        stopRefreshEmojies.store (false);
        startPollingEmojies ();
      }

      if (event.command.get_command_name () == "emoji") {
        std::string buf = emojiTools->getRandomEmoji ();
        LOG_I_STREAM << buf << std::endl;
        event.reply (buf);
      }

      if (event.command.get_command_name () == "rss") {
        std::string buf = getRootcz ();
        if (buf.empty ()) {
          dpp::message msg (channelDev, "Error: Could not get the RSS feed!");
          event.reply (msg);
          return;
        }
        LOG_I_STREAM << buf << std::endl;
        event.reply (buf);
      }

      if (event.command.get_command_name () == "ping") {
        event.reply ("Pong! 🏓");
      }

      if (event.command.get_command_name () == "pong") {
        event.reply ("Ping! 🏓");
      }

      if (event.command.get_command_name () == "gang") {
        dpp::message msg (event.command.channel_id, "Bang bang! 💥💥");
        event.reply (msg);
        m_bot->message_create (msg);
      }

      if (event.command.get_command_name () == "bot") {
        dpp::message msgFastfetch (channelDev,
                                   this->getLinuxFastfetchCpp ().substr (0, 8192 - 2) + "\n");
        event.reply (msgFastfetch);
      }

      if (event.command.get_command_name () == "stopbot") {
        dpp::message msgFastfetch (channelDev, "stoping bot ...\n");
        event.reply (msgFastfetch);
        // stop D++
        m_bot->shutdown ();
      }
    });

    m_bot->on_ready ([&] (const dpp::ready_t& event) {
      /* sunriset */
      m_bot->global_command_create (dpp::slashcommand ("sunriset", "Get sunriset!", m_bot->me.id));

      /* snippet */
      m_bot->global_command_create (
          dpp::slashcommand ("verse", "Get verse from Czech Bible!", m_bot->me.id));

      /* czk */
      m_bot->global_command_create (dpp::slashcommand ("czk", "Get Czech Exchange!", m_bot->me.id));

      /* bitcoin */
      m_bot->global_command_create (dpp::slashcommand ("btc", "Get Bitcoin Price!", m_bot->me.id));

      /* fortune */
      m_bot->global_command_create (
          dpp::slashcommand ("fortune", "Get random Quote!", m_bot->me.id));

      /* noemojies */
      m_bot->global_command_create (dpp::slashcommand (
          "noemojies", "Stop to getting random Emoji in regularly interval 10 seconds!",
          m_bot->me.id));

      /* emojies */
      m_bot->global_command_create (dpp::slashcommand (
          "emojies", "Get random Emoji in regularly interval 10 seconds!", m_bot->me.id));

      /* emoji */
      m_bot->global_command_create (dpp::slashcommand ("emoji", "Get random Emoji!", m_bot->me.id));

      /* rss */
      m_bot->global_command_create (dpp::slashcommand ("rss", "Get rss feed!", m_bot->me.id));

      /* ping */
      m_bot->global_command_create (dpp::slashcommand ("ping", "Ping pong!", m_bot->me.id));

      /* pong */
      m_bot->global_command_create (dpp::slashcommand ("pong", "Pong ping!", m_bot->me.id));

      /* gang */
      m_bot->global_command_create (dpp::slashcommand ("gang", "Will shoot!", m_bot->me.id));

      /* bot */
      m_bot->global_command_create (
          dpp::slashcommand ("stopbot", "Stop DSDotBot Bot!", m_bot->me.id));

      /* bot */
      m_bot->global_command_create (dpp::slashcommand ("bot", "About DSDotBot Bot!", m_bot->me.id));
    });

    return true;
  }

  std::string MyDpp::getCurrentTime () {
    time_t now = time (0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime (&now);
    strftime (buf, sizeof (buf), "%Y-%m-%d %X", &tstruct);
    return buf;
  }

  int MyDpp::getRandom (int min, int max) {
    std::random_device rd;
    std::mt19937 gen (rd ());
    std::uniform_int_distribution<> dis (min, max);
    int random = dis (gen);
    return random;
  }

  MyDpp::RSSFeed MyDpp::parseRSSToStruct (const std::string& xmlData) {
    LOG_D_STREAM << "Parsing RSS feed to structure..." << std::endl;

    tinyxml2::XMLDocument doc;
    doc.Parse (xmlData.c_str ());

    tinyxml2::XMLElement* rssElement = doc.FirstChildElement ("rss");
    if (!rssElement) {
      LOG_E_STREAM << "Error: No RSS root element found." << std::endl;
      return feedRootCz;
    }

    tinyxml2::XMLElement* channel = rssElement->FirstChildElement ("channel");
    if (!channel) {
      LOG_E_STREAM << "Error: RSS feed is not valid." << std::endl;
      return feedRootCz;
    }

    // Parse channel info
    if (auto titleElement = channel->FirstChildElement ("title")) {
      feedRootCz.title = titleElement->GetText () ? titleElement->GetText () : "";
    }
    if (auto descElement = channel->FirstChildElement ("description")) {
      feedRootCz.description = descElement->GetText () ? descElement->GetText () : "";
    }
    if (auto linkElement = channel->FirstChildElement ("link")) {
      feedRootCz.link = linkElement->GetText () ? linkElement->GetText () : "";
    }

    // Parse items
    tinyxml2::XMLElement* item = channel->FirstChildElement ("item");
    while (item) {
      RSSItem rssItem;

      if (auto titleElement = item->FirstChildElement ("title")) {
        rssItem.title = titleElement->GetText () ? titleElement->GetText () : "";
      }
      if (auto linkElement = item->FirstChildElement ("link")) {
        rssItem.link = linkElement->GetText () ? linkElement->GetText () : "";
      }
      if (auto descElement = item->FirstChildElement ("description")) {
        rssItem.description = descElement->GetText () ? descElement->GetText () : "";
      }
      if (auto pubDateElement = item->FirstChildElement ("pubDate")) {
        rssItem.pubDate = pubDateElement->GetText () ? pubDateElement->GetText () : "";
      }
      if (auto guidElement = item->FirstChildElement ("guid")) {
        rssItem.guid = guidElement->GetText () ? guidElement->GetText () : "";
      }

      if (!rssItem.title.empty () && !rssItem.link.empty ()) {
        feedRootCz.addItem (rssItem);
        LOG_I_STREAM << "Added item: " << rssItem.title << std::endl;
      }

      item = item->NextSiblingElement ("item");
    }

    LOG_I_STREAM << "Parsed " << feedRootCz.getItemCount () << " RSS items" << std::endl;
    return feedRootCz;
  }

  // Helper method to convert RSSFeed back to string if needed
  std::string MyDpp::RSSFeed::toString () const {
    std::string result = "RSS Feed: " + title + "\n";
    result += "Description: " + description + "\n";
    result += "Link: " + link + "\n\n";

    for (const auto& item : items) {
      result += "Title: " + item.title + "\n";
      result += "Link: " + item.link + "\n";
      result += "Description: " + item.description + "\n";
      if (!item.pubDate.empty ()) {
        result += "Published: " + item.pubDate + "\n";
      }
      result += "------------------------\n";
    }
    return result;
  }
} // namespace dotname