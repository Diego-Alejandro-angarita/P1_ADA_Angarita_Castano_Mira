#include "src/fb_password.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

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
    if (parsedCharacters != value.size() || parsed == 0 || parsed > static_cast<unsigned long long>(SIZE_MAX)) {
        throw std::invalid_argument("La longitud no es valida");
    }
    return static_cast<std::size_t>(parsed);
}

void printResult(const std::string& method, const fb::SearchResult& result) {
    std::cout << "Metodo: " << method << '\n';
    std::cout << "Estado: " << (result.found ? "encontrada" : "no encontrada") << '\n';
    if (result.found) {
        std::cout << "Texto plano: " << result.plaintext << '\n';
    }
    std::cout << "Candidatos evaluados: " << result.candidates << '\n';
    std::cout << "Tiempo (ms): " << result.milliseconds << '\n';
}

void printUsage(const char* executable) {
    std::cout << "Uso:\n"
              << "  " << executable << " --validate\n"
              << "  " << executable << " --brute HASH A1|A2 MIN MAX\n"
              << "  " << executable << " --dictionary HASH RUTA\n";
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
        if (command == "--brute" && argc == 6) {
            const fb::SearchResult result = fb::bruteForce(
                argv[2], parseAlphabet(argv[3]), parseLength(argv[4]), parseLength(argv[5]));
            printResult("fuerza bruta pura", result);
            return EXIT_SUCCESS;
        }
        if (command == "--dictionary" && argc == 4) {
            const fb::SearchResult result = fb::dictionaryAttack(argv[2], argv[3]);
            printResult("ataque por diccionario", result);
            return EXIT_SUCCESS;
        }

        printUsage(argv[0]);
        return EXIT_FAILURE;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
