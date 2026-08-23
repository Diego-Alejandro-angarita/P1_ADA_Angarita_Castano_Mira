#include "fb_password.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

constexpr std::array<std::uint32_t, 64> kRoundConstants = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

std::uint32_t rotateRight(std::uint32_t value, unsigned bits) {
    return (value >> bits) | (value << (32u - bits));
}

std::uint32_t choose(std::uint32_t x, std::uint32_t y, std::uint32_t z) {
    return (x & y) ^ (~x & z);
}

std::uint32_t majority(std::uint32_t x, std::uint32_t y, std::uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

bool nextCandidate(std::string& candidate, const std::string& symbols) {
    for (std::size_t position = candidate.size(); position > 0; --position) {
        char& current = candidate[position - 1];
        const std::size_t index = symbols.find(current);
        if (index + 1 < symbols.size()) {
            current = symbols[index + 1];
            return true;
        }
        current = symbols.front();
    }
    return false;
}

std::string normalizedHash(std::string hash) {
    for (char& character : hash) {
        if (character >= 'A' && character <= 'F') {
            character = static_cast<char>(character - 'A' + 'a');
        }
    }
    return hash;
}

}

namespace fb {

std::string alphabet(Alphabet alphabetType) {
    if (alphabetType == Alphabet::A1) {
        return "abcdefghijklmnopqrstuvwxyz";
    }
    return "abcdefghijklmnopqrstuvwxyz0123456789";
}

std::string sha256(const std::string& text) {
    std::vector<std::uint8_t> message(text.begin(), text.end());
    const std::uint64_t bitLength = static_cast<std::uint64_t>(message.size()) * 8u;
    message.push_back(0x80u);
    while ((message.size() % 64u) != 56u) {
        message.push_back(0u);
    }
    for (int shift = 56; shift >= 0; shift -= 8) {
        message.push_back(static_cast<std::uint8_t>(bitLength >> shift));
    }

    std::array<std::uint32_t, 8> hash = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
    };

    for (std::size_t offset = 0; offset < message.size(); offset += 64u) {
        std::array<std::uint32_t, 64> words{};
        for (std::size_t index = 0; index < 16u; ++index) {
            const std::size_t base = offset + index * 4u;
            words[index] = (static_cast<std::uint32_t>(message[base]) << 24u)
                         | (static_cast<std::uint32_t>(message[base + 1]) << 16u)
                         | (static_cast<std::uint32_t>(message[base + 2]) << 8u)
                         | static_cast<std::uint32_t>(message[base + 3]);
        }
        for (std::size_t index = 16u; index < 64u; ++index) {
            const std::uint32_t s0 = rotateRight(words[index - 15], 7u)
                                   ^ rotateRight(words[index - 15], 18u)
                                   ^ (words[index - 15] >> 3u);
            const std::uint32_t s1 = rotateRight(words[index - 2], 17u)
                                   ^ rotateRight(words[index - 2], 19u)
                                   ^ (words[index - 2] >> 10u);
            words[index] = words[index - 16] + s0 + words[index - 7] + s1;
        }

        std::uint32_t a = hash[0];
        std::uint32_t b = hash[1];
        std::uint32_t c = hash[2];
        std::uint32_t d = hash[3];
        std::uint32_t e = hash[4];
        std::uint32_t f = hash[5];
        std::uint32_t g = hash[6];
        std::uint32_t h = hash[7];
        for (std::size_t index = 0; index < 64u; ++index) {
            const std::uint32_t sum1 = rotateRight(e, 6u) ^ rotateRight(e, 11u) ^ rotateRight(e, 25u);
            const std::uint32_t temporary1 = h + sum1 + choose(e, f, g) + kRoundConstants[index] + words[index];
            const std::uint32_t sum0 = rotateRight(a, 2u) ^ rotateRight(a, 13u) ^ rotateRight(a, 22u);
            const std::uint32_t temporary2 = sum0 + majority(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + temporary1;
            d = c;
            c = b;
            b = a;
            a = temporary1 + temporary2;
        }
        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;
        hash[5] += f;
        hash[6] += g;
        hash[7] += h;
    }

    std::ostringstream output;
    output << std::hex << std::setfill('0');
    for (std::uint32_t word : hash) {
        output << std::setw(8) << word;
    }
    return output.str();
}

SearchResult bruteForce(const std::string& targetHash,
                        Alphabet alphabetType,
                        std::size_t minLength,
                        std::size_t maxLength) {
    if (minLength == 0 || minLength > maxLength) {
        throw std::invalid_argument("El rango de longitudes no es valido");
    }

    const std::string symbols = alphabet(alphabetType);
    const std::string expected = normalizedHash(targetHash);
    SearchResult result;
    const auto started = std::chrono::steady_clock::now();

    for (std::size_t length = minLength; length <= maxLength; ++length) {
        std::string candidate(length, symbols.front());
        do {
            ++result.candidates;
            if (sha256(candidate) == expected) {
                result.found = true;
                result.plaintext = std::move(candidate);
                length = maxLength;
                break;
            }
        } while (nextCandidate(candidate, symbols));
        if (result.found || length == maxLength) {
            break;
        }
    }

    const auto elapsed = std::chrono::steady_clock::now() - started;
    result.milliseconds = std::chrono::duration<double, std::milli>(elapsed).count();
    return result;
}

SearchResult dictionaryAttack(const std::string& targetHash,
                              const std::string& dictionaryPath) {
    SearchResult result;
    const auto started = std::chrono::steady_clock::now();
    std::ifstream dictionary(dictionaryPath);
    if (dictionary) {
        const std::string expected = normalizedHash(targetHash);
        std::string candidate;
        while (std::getline(dictionary, candidate)) {
            if (!candidate.empty() && candidate.back() == '\r') {
                candidate.pop_back();
            }
            ++result.candidates;
            if (sha256(candidate) == expected) {
                result.found = true;
                result.plaintext = std::move(candidate);
                break;
            }
        }
    }
    const auto elapsed = std::chrono::steady_clock::now() - started;
    result.milliseconds = std::chrono::duration<double, std::milli>(elapsed).count();
    return result;
}

}
