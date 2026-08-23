#ifndef FB_PASSWORD_HPP
#define FB_PASSWORD_HPP

#include <cstddef>
#include <string>

namespace fb {

enum class Alphabet {
    A1,
    A2
};

struct SearchResult {
    bool found = false;
    std::string plaintext;
    std::size_t candidates = 0;
    double milliseconds = 0.0;
};

std::string alphabet(Alphabet alphabetType);
std::string sha256(const std::string& text);

SearchResult bruteForce(const std::string& targetHash,
                        Alphabet alphabetType,
                        std::size_t minLength,
                        std::size_t maxLength);

SearchResult dictionaryAttack(const std::string& targetHash,
                              const std::string& dictionaryPath);

}

#endif
