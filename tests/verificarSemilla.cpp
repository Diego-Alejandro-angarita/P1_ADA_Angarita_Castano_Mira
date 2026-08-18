#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cctype>

using namespace std;

int main() {
    size_t n = 3;
    vector<string> apellidos(n);
    for (int i = 0; i < n; i++) {
        cout << "Apellido " << (i + 1) << ": ";
        getline(cin, apellidos[i]);
    }

    sort(apellidos.begin(), apellidos.end());

    string cadena;
    for (auto& a : apellidos) {
        cadena += a;
    }
    cout << "\nApellidos ordenados : \"" << cadena << "\"\n";

    uint64_t suma = 0;
    cout << "\nDesglose byte a byte:\n";
    for (unsigned char c : cadena) {
        suma += c;
        cout << "  byte " << (int)c << "  (acumulado: " << suma << ")\n";
    }

    uint64_t semilla = suma % 100000;
    cout << "\nSuma total = " << suma << "\n";
    cout << "Semilla = " << suma << " mod 100000 = " << semilla << "\n";

    int minLower  = 2 + (semilla % 3);
    int minUpper  = 1 + (semilla % 2);
    int minDigit  = 1 + (semilla % 3);
    int minSymbol = 1;
    int total = minLower + minUpper + minDigit + minSymbol;

    cout << "\nRequisitos de la contraseña:\n";
    /* 
        de 2 a 4 minusculas
        de 1 a 2 mayusculas
        de 1 a 3 digitos
        1 simbolo
    */
    cout << "minLower  = 2 + (semilla mod 3) = " << minLower  << "\n";
    cout << "minUpper  = 1 + (semilla mod 2) = " << minUpper  << "\n";
    cout << "minDigit  = 1 + (semilla mod 3) = " << minDigit  << "\n";
    cout << "minSymbol = " << minSymbol << "\n";
    cout << "Suma total = " << total;

    cout << "x0 = semilla = " << semilla << "\n";
    const string A1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const string A2 = "adcdefghijklmnopqrstuvwxyz0123456789";
    uint64_t x = semilla;
    const uint64_t MOD = 1ULL << 31;
    for (int i = 1; i <= 24; i++) {
        x = (1103515245ULL * x + 12345ULL) % MOD;
        cout << "x" << i << " = " << x
                  << "   char si A1: '" << A1[x % A1.size()] << "'"
                  << "   char si A2: '" << A2[x % A2.size()] << "'\n";
    }
    return 0;
}