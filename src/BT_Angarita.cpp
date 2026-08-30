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

// Un caso de prueba = una politica completa mas una etiqueta legible.
struct CasoBT {
    const char* etiqueta;   // descripcion corta del caso
    PoliticaBT  pol;        // politica (n + minimos + bandera de no-consecutivos)
};

int main() {
    const long long MAX_CADENAS = 3500;   // limite configurable de cadenas por caso

    // Las 5 pruebas distintas. minLower, minUpper, minDigit, minSymbol, n, prohibirSeguidos.
    // Casos 1-3: politica de la semilla (2,2,3,1, no-consecutivos) variando solo n.
    // Caso 4: solo minLower=1; el resto de politicas NO se aplican (mins 0, sin no-consecutivos).
    // Caso 5: sin ninguna politica (mins 0, sin no-consecutivos) -> poda nula = fuerza bruta.
    const CasoBT casos[] = {
        {"C1 semilla n=8 (2,2,3,1, no-consec)",     {2, 2, 3, 1,  8, true}},
        {"C2 semilla n=6 (2,2,3,1, no-consec)",     {2, 2, 3, 1,  6, true}},
        {"C3 semilla n=10 (2,2,3,1, no-consec)",    {2, 2, 3, 1, 10, true}},
        {"C4 n=8 solo minLower=1 (sin otras pol.)", {1, 0, 0, 0,  8, false}},
        {"C5 n=6 sin politica (fuerza bruta)",      {0, 0, 0, 0,  6, false}},
    };

    ofstream out("results/PasswordsBT.txt");
    if (!out) {
        cerr << "No se pudo abrir results/PasswordsBT.txt (ejecuta desde la raiz del proyecto)\n";
        return 1;
    }

    clock_t inicioTotal = clock();

    for (const CasoBT& caso : casos) {
        const PoliticaBT& pol = caso.pol;

        out << caso.etiqueta << " | n=" << pol.n
            << " minLower=" << pol.minLower << " minUpper=" << pol.minUpper
            << " minDigit=" << pol.minDigit << " minSymbol=" << pol.minSymbol
            << " prohibirSeguidos=" << (pol.prohibirSeguidos ? "true" : "false")
            << " (limite=" << MAX_CADENAS << ")\n";
        cout << "Generando " << caso.etiqueta << " ...\n";

        clock_t inicio = clock();                          // inicio de esta corrida
        long long escritas = generarPassword(pol, out, MAX_CADENAS);
        clock_t fin = clock();                             // fin de esta corrida
        double segundos = double(fin - inicio) / CLOCKS_PER_SEC;

        out << caso.etiqueta << ": " << escritas << " cadenas escritas en "
            << segundos << " s\n";
        cout << "  " << caso.etiqueta << ": " << escritas << " cadenas escritas en "
             << segundos << " s.\n";
    }

    double segundosTotal = double(clock() - inicioTotal) / CLOCKS_PER_SEC;

    out.close();
    cout << "\nTiempo total de ejecucion: " << segundosTotal << " s\n";
    cout << "Resultados en results/PasswordsBT.txt\n";
    return 0;
}
