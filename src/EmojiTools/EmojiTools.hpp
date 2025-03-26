#ifndef __EMOJITOOLS_H__
#define __EMOJITOOLS_H__

#include "EmojiLogic.hpp"

class EmojiTools
{
public:
  EmojiTools ();
  ~EmojiTools ();

  // hide the implementation details
private:
  EmojiTransmitter et;

  // public interface
public:
  std::string getEmojiStringCharByCodePoint (char32_t *emojiCodePoints,
                                             size_t length)
  {
    return et.getEmojiStringCharByCodePoint (emojiCodePoints, length);
  }
  char8_t getEmojiChar8_tCharByCodePoint (char32_t *emojiCodePoints,
                                          size_t length)
  {
    return et.getEmojiChar8_tCharByCodePoint (emojiCodePoints, length);
  }
  std::string getRandomEmoji () { return et.getRandomEmoji (); }
  std::string getRandomEmojiFromGroup (const std::string emojiGroup)
  {
    return et.getRandomEmojiFromGroup (emojiGroup);
  }
  std::string getRandomEmojiFromSubGroup (const std::string emojiSubGroup)
  {
    return et.getRandomEmojiFromSubGroup (emojiSubGroup);
  }
  std::string getAllEmojiesFromGroup (const std::string emojiGroup)
  {
    return et.getEmojiesFromGroup (emojiGroup);
  }
  std::string getAllEmojiesFromSubGroup (const std::string emojiSubGroup)
  {
    return et.getEmojiesFromSubGroup (emojiSubGroup);
  }
  std::vector<std::string> getEmojiGroups ()
  {
    return et.getEmojiGroupsNames ();
  }
  std::vector<std::string> getEmojiSubGroups ()
  {
    return et.getEmojiSubGroupsNames ();
  }
  int getSizeOfGroupItems (const std::string emojiGroup)
  {
    return et.getSizeOfGroupItems (emojiGroup);
  }
  int getSizeOfSubGroupItems (const std::string emojiSubGroup)
  {
    return et.getSizeOfSubGroupItems (emojiSubGroup);
  }
  std::string getEmojiStringByIndexFromGroup (const std::string emojiGroup,
                                              const int index)
  {
    return et.getEmojiStringByIndexFromGroup (emojiGroup, index);
  }
  std::string
  getEmojiStringByIndexFromSubGroup (const std::string emojiSubGroup,
                                     const int index)
  {
    return et.getEmojiStringByIndexFromSubGroup (emojiSubGroup, index);
  }
  std::string getEmojiGroupDescription (const std::string emojiGroup)
  {
    return et.getEmojiGroupDescription (emojiGroup);
  }
  std::string getEmojiSubGroupDescription (const std::string emojiSubGroup)
  {
    return et.getEmojiSubGroupDescription (emojiSubGroup);
  }
};

#endif // __EMOJITOOLS_H__
