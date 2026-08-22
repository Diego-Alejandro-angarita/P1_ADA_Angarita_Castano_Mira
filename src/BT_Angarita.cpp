#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include "BTDatos.hpp"

using namespace std;

bool esMinuscula(char c) { return c >= 'a' && c <= 'z'; }
bool esMayuscula(char c) { return c >= 'A' && c <= 'Z'; }
bool esDigito(char c)    { return c >= '0' && c <= '9'; }
bool esSimbolo(char c)   { return string("!@#$%").find(c) != string::npos; }

void backtracking(string& password, int pos,
                  int nLower, int nUpper, int nDigit, int nSymbol,
                  const PoliticaBT& pol, ofstream& out,
                  long long& contador, long long limite) 
                  
{
    // Corte por limite de cadenas a escribir
    if (contador >= limite) return;

    int faltan = 0;
    if (pol.minLower  - nLower  > 0) faltan += pol.minLower  - nLower;
    if (pol.minUpper  - nUpper  > 0) faltan += pol.minUpper  - nUpper;
    if (pol.minDigit  - nDigit  > 0) faltan += pol.minDigit  - nDigit;
    if (pol.minSymbol - nSymbol > 0) faltan += pol.minSymbol - nSymbol;
    if (faltan > pol.n - pos) return;   // rama imposible -> retroceder

    // Caso base: cadena completa (los minimos se garantizan por la poda)
    if (pos == pol.n) {
        out << password << "\n";
        ++contador;
        return;
    }

    // Probar cada caracter del alfabeto en la posicion actual
    for (char c : ALFABETO_BT) {
        if (pol.prohibirSeguidos && pos > 0 && c == password[pos - 1])
            continue;   // no se permiten dos caracteres iguales consecutivos

        password[pos] = c;
        backtracking(password, pos + 1,
                     nLower  + (esMinuscula(c) ? 1 : 0),
                     nUpper  + (esMayuscula(c) ? 1 : 0),
                     nDigit  + (esDigito(c)    ? 1 : 0),
                     nSymbol + (esSimbolo(c)   ? 1 : 0),
                     pol, out, contador, limite);

        if (contador >= limite) return;  // ya alcanzamos el limite
    }
}

long long generarPassword(const PoliticaBT& pol, ofstream& out, long long limite) {
    int minTotal = pol.minLower + pol.minUpper + pol.minDigit + pol.minSymbol;

    // Deteccion temprana: la longitud n no alcanza para los minimos requeridos.
    if (minTotal > pol.n) {
        out << "  [longitud n=" << pol.n << " insuficiente: se requieren al menos "
            << minTotal << " caracteres (minLower=" << pol.minLower
            << ", minUpper=" << pol.minUpper << ", minDigit=" << pol.minDigit
            << ", minSymbol=" << pol.minSymbol << ")]\n";
        cout << "  n=" << pol.n << ": longitud insuficiente (min requerido = "
             << minTotal << "). 0 cadenas.\n";
        return 0;
    }

    string password(pol.n, ' ');
    long long contador = 0;
    backtracking(password, 0, 0, 0, 0, 0, pol, out, contador, limite);
    return contador;
}

int main() {
    const long long MAX_CADENAS = 1000;   // limite configurable de cadenas por n

    ofstream out("results/PasswordsBT.txt");
    if (!out) {
        cerr << "No se pudo abrir results/Diccionario.txt (ejecuta desde la raiz del proyecto)\n";
        return 1;
    }

    clock_t inicioTotal = clock();

    for (int n : {8, 6, 10}) {
        PoliticaBT pol;
        pol.n = n;

        out << n << " (limite=" << MAX_CADENAS << ")\n";
        cout << "Generando para n=" << n << " ...\n";

        clock_t inicio = clock();                          // inicio de esta corrida
        long long escritas = generarPassword(pol, out, MAX_CADENAS);
        clock_t fin = clock();                             // fin de esta corrida
        double segundos = double(fin - inicio) / CLOCKS_PER_SEC;

        out << "n=" << n << ": " << escritas << " cadenas escritas en "
            << segundos << " s\n";
        cout << "  n=" << n << ": " << escritas << " cadenas escritas en "
             << segundos << " s.\n";
    }

    double segundosTotal = double(clock() - inicioTotal) / CLOCKS_PER_SEC;

    out.close();
    cout << "\nTiempo total de ejecucion: " << segundosTotal << " s\n";
    cout << "Resultados en results/Diccionario.txt\n";
    return 0;
}
