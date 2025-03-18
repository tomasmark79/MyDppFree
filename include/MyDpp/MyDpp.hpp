#ifndef __MYDPP_H__
#define __MYDPP_H__

// MIT License
// Copyright (c) 2024-2025 Tomáš Mark

#include <dpp/dpp.h>
#include <iostream>
#include <memory>
#include <random>
#include <string>

// Public API

namespace library {

class MyDpp {
public:
  MyDpp(const std::string &assetsPath);
  ~MyDpp();

  // alternatively, you can use a getter function
  const std::string getAssetsPath() const { return m_assetsPath; }

  bool loadVariousBotCommands();
  bool startPollingEmojies();
  bool startPollingFortune();
  bool startPollingBTCPrice();
  bool startPollingCZExchRate();
  bool startPollingGetBibleVerse();

  bool welcomeWithNeofetch();
  bool initCluster();
  bool getToken(std::string &token, const std::string &filePath);

  std::string getLinuxFortuneCpp();
  std::string getLinuxNeofetchCpp();
  std::string getBitcoinPrice();
  std::string getCzechBibleVerse();
  std::string getCzechExchangeRate();
  std::string getCurrentTime();
  int getRandom(int min, int max);

private:
  std::string m_assetsPath;
  std::unique_ptr<dpp::cluster> m_bot;
};

} // namespace library

#endif // __MYDPP_H__
