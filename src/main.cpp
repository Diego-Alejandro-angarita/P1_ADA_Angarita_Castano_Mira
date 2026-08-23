#include "fb_password.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr const char* kReferenceHash =
    "8d51feb34e3e69f6fa6dffc577e2c60490cf9a7fcd835f9f6af1505b71d74773";

fb::Alphabet parseAlphabet(const std::string& value) {
    if (value == "A1") {
        return fb::Alphabet::A1;
    }
    if (value == "A2") {
        return fb::Alphabet::A2;
    }
    throw std::invalid_argument("El alfabeto debe ser A1 o A2");
}

std::size_t parseLength(const std::string& value) {
    std::size_t parsedCharacters = 0;
    const unsigned long long parsed = std::stoull(value, &parsedCharacters);
    if (parsedCharacters != value.size() || parsed == 0
        || parsed > static_cast<unsigned long long>(SIZE_MAX)) {
        throw std::invalid_argument("La longitud no es valida");
    }
    return static_cast<std::size_t>(parsed);
}

void printResult(const std::string& method, const fb::SearchResult& result) {
    std::cout << "Metodo: " << method << '\n';
    std::cout << "Estado: "
              << (result.found ? "encontrada" : "No encontrado dentro del espacio")
              << '\n';
    if (result.found) {
        std::cout << "Texto plano: " << result.plaintext << '\n';
    }
    std::cout << "Candidatos evaluados: " << result.candidates << '\n';
    std::cout << "Tiempo (ms): " << result.milliseconds << '\n';
}

void printUsage(const char* executable) {
    std::cout << "Uso:\n"
              << "  " << executable << " --validate\n"
              << "  " << executable << " --generate-instances\n"
              << "  " << executable << " --brute HASH A1|A2 MIN MAX\n"
              << "  " << executable << " --dictionary HASH RUTA\n";
}

void generateInstances() {
    std::vector<std::string> surnames = {"angarita", "castano", "mira"};
    std::sort(surnames.begin(), surnames.end());
    const std::string concatenated = surnames[0] + surnames[1] + surnames[2];

    std::uint64_t seed = 0;
    for (unsigned char character : concatenated) {
        seed += character;
    }
    seed %= 100000;

    constexpr std::uint64_t modulus = 1ULL << 31;
    constexpr std::uint64_t multiplier = 1103515245ULL;
    constexpr std::uint64_t increment = 12345ULL;
    const std::vector<std::size_t> lengths = {4, 4, 5, 5, 6};
    const std::vector<fb::Alphabet> alphabets = {
        fb::Alphabet::A1, fb::Alphabet::A2, fb::Alphabet::A1,
        fb::Alphabet::A2, fb::Alphabet::A1
    };
    std::uint64_t state = seed;

    std::cout << "Cadena: " << concatenated << '\n';
    std::cout << "Semilla: " << seed << '\n';
    for (std::size_t index = 0; index < lengths.size(); ++index) {
        const std::string symbols = fb::alphabet(alphabets[index]);
        std::string password;
        password.reserve(lengths[index]);
        for (std::size_t character = 0; character < lengths[index]; ++character) {
            password += symbols[state % symbols.size()];
            state = (multiplier * state + increment) % modulus;
        }

        const std::string hash = fb::sha256(password);
        std::cout << "\nCaso " << (index + 1)
                  << " | Alfabeto: " << (alphabets[index] == fb::Alphabet::A1 ? "A1" : "A2")
                  << " | Longitud: " << lengths[index] << '\n';
        std::cout << "Contrasena: " << password << '\n';
        std::cout << "Hash: " << hash << '\n';

        const fb::SearchResult result = fb::bruteForce(
            hash, alphabets[index], lengths[index], lengths[index]);
        printResult("fuerza bruta - caso " + std::to_string(index + 1), result);
    }
}

}

int main(int argc, char* argv[]) {
    try {
        if (argc == 1 || (argc == 2 && std::string(argv[1]) == "--validate")) {
            const fb::SearchResult result = fb::bruteForce(kReferenceHash, fb::Alphabet::A2, 5, 5);
            printResult("fuerza bruta pura (validacion)", result);
            return result.found && result.plaintext == "abc12" ? EXIT_SUCCESS : EXIT_FAILURE;
        }

        const std::string command = argv[1];
        if (command == "--generate-instances" && argc == 2) {
            generateInstances();
            return EXIT_SUCCESS;
        }
        if (command == "--brute" && argc == 6) {
            const fb::SearchResult result = fb::bruteForce(
                argv[2], parseAlphabet(argv[3]), parseLength(argv[4]), parseLength(argv[5]));
            printResult("fuerza bruta pura", result);
            return EXIT_SUCCESS;
        }
        if (command == "--dictionary" && argc == 4) {
            const fb::SearchResult result = fb::dictionaryAttack(argv[2], argv[3]);
            printResult("ataque por diccionario", result);
            std::cout << "Pertenece al diccionario: " << (result.found ? "si" : "no") << '\n';
            return EXIT_SUCCESS;
        }

        printUsage(argv[0]);
        return EXIT_FAILURE;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
