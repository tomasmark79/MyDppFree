#ifndef __EMOJILOGIC_H__
#define __EMOJILOGIC_H__

#include <assert.h>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#if __cplusplus < 202002L
using char8_t = char; // C++17 and older
#endif

#ifndef size_t
using size_t = decltype (sizeof (0));
#endif

class EmojiBuilder
{
  struct EmojiPropertiesStructure
  {
    std::vector<char32_t> m_emojiCodePoints;
    std::string m_emojiGroup;
    std::string m_emojiSubGroup;
    std::string m_emojiUnicodeVersion;
    std::string m_emojiTextDescription;
  };

public:
  std::map<int, EmojiPropertiesStructure> m_emojiPropertiesMap;

  EmojiBuilder ();
  ~EmojiBuilder () {};

  char8_t *encodeUtf8 (char32_t emojiCodePoint, char8_t *buffer8);
  char8_t *encodeUtf8Sequence (const char32_t *emojiCodePoints, size_t length,
                               char8_t *buffer8);
  std::string getEmojiStringCharByCodePoint (char32_t *emojiCodePoints,
                                             size_t length);
  char8_t getEmojiChar8_tCharByCodePoint (char32_t *emojiCodePoints,
                                          size_t length);

  bool m_isPopulated{ false };

private:
  void
  constructEmojiPropertiesMap (std::map<int, EmojiPropertiesStructure> &epm,
                               std::istream &file);

  std::istringstream loadEmojiAssetsFromHardcodedHeader ();
  std::ifstream loadEmojiAssetsFromFile ();
};

class EmojiTransmitter
{
public:
  EmojiTransmitter () = default;

  EmojiBuilder emojiBuilder;

public:
  std::string getEmojiStringCharByCodePoint (char32_t *emojiCodePoints,
                                             size_t length);
  char8_t getEmojiChar8_tCharByCodePoint (char32_t *emojiCodePoints,
                                          size_t length);
  std::string getRandomEmoji ();
  std::string getRandomEmojiFromGroup (std::string emojiGroup);
  std::string getRandomEmojiFromSubGroup (std::string emojiSubGroup);
  std::string getEmojiesFromGroup (std::string emojiGroup);
  std::string getEmojiesFromSubGroup (std::string emojiSubGroup);
  std::vector<std::string> getEmojiGroupsNames ();
  std::vector<std::string> getEmojiSubGroupsNames ();
  int getSizeOfGroupItems (std::string emojiGroup);
  int getSizeOfSubGroupItems (std::string emojiSubGroup);
  std::string getEmojiStringByIndexFromGroup (std::string emojiGroup,
                                              int index);
  std::string getEmojiStringByIndexFromSubGroup (std::string emojiSubGroup,
                                                 int index);
  std::string getEmojiGroupDescription (std::string emojiGroup);
  std::string getEmojiSubGroupDescription (std::string emojiSubGroup);

  // TODO

  // Get emoji by name
  // Get emoji by description
  // Get emoji by unicode version

  void printEmojiGroup (std::string emojiGroup);
  void printEmojiSubGroupWDescription (std::string emojiSubGroup);
  void printEmojiSubGroup (std::string emojiSubGroup);

  void printGroupsText ();
  void printSubGroupsText ();
  void printEmojiDescription (std::string emojiDescription);
};

#endif // __EMOJILOGIC_H__