/* StringGen.h: group generation for trainer-style character sets. */
#ifndef STRINGGEN_H
#define STRINGGEN_H

#include <string>

enum class GroupCharsetMode
{
    Letters,
    Digits,
    LettersDigits,
    LettersDigitsPunctuation
};

class StringGen
{
public:
    std::string generate(int length, GroupCharsetMode mode = GroupCharsetMode::LettersDigitsPunctuation) const;
    std::string generateFromCharset(int length, const std::string& charset) const;
    std::string generateOnlyLetters(int length) const;
    std::string generateOnlyDigits(int length) const;
    std::string generateLettersAndDigits(int length) const;

    static const std::string& charsetForMode(GroupCharsetMode mode);
};

#endif
