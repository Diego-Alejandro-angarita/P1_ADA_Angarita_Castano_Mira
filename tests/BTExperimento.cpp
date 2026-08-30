// Arnes instrumentado para la comparacion "con poda" vs "sin poda".
// Reproduce exactamente la logica de src/BT_Angarita.cpp, pero:
//   - enumera TODAS las soluciones (sin limite de cadenas),
//   - no escribe las cadenas a disco (solo cuenta), para medir el recorrido puro,
//   - cuenta los nodos visitados (una vez por entrada a la funcion recursiva).
// La version "sin poda" omite la desigualdad faltan > n - pos y valida en la hoja.
// Ambas respetan la regla de no-consecutivos, asi que definen el mismo conjunto
// de soluciones: los conteos deben coincidir.
//
// Compilar:  g++ -O2 -o results/BTExperimento.exe tests/BTExperimento.cpp
// Ejecutar:  results/BTExperimento.exe            (imprime CSV por stdout)

#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include "../src/BTDatos.hpp"

using namespace std;

bool esMinuscula(char c) { return c >= 'a' && c <= 'z'; }
bool esMayuscula(char c) { return c >= 'A' && c <= 'Z'; }
bool esDigito(char c)    { return c >= '0' && c <= '9'; }
bool esSimbolo(char c)   { return string("!@#$%").find(c) != string::npos; }

static long long nodos;        // nodos visitados (entradas a la funcion)
static long long soluciones;   // hojas validas

// ---------------------------------------------------------------- con poda
void btConPoda(string& password, int pos,
               int nLower, int nUpper, int nDigit, int nSymbol,
               const PoliticaBT& pol)
{
    ++nodos;

    int faltan = 0;
    if (pol.minLower  - nLower  > 0) faltan += pol.minLower  - nLower;
    if (pol.minUpper  - nUpper  > 0) faltan += pol.minUpper  - nUpper;
    if (pol.minDigit  - nDigit  > 0) faltan += pol.minDigit  - nDigit;
    if (pol.minSymbol - nSymbol > 0) faltan += pol.minSymbol - nSymbol;
    if (faltan > pol.n - pos) return;   // PODA: rama infactible

    if (pos == pol.n) { ++soluciones; return; }

    for (char c : ALFABETO_BT) {
        if (pol.prohibirSeguidos && pos > 0 && c == password[pos - 1]) continue;
        password[pos] = c;
        btConPoda(password, pos + 1,
                  nLower  + (esMinuscula(c) ? 1 : 0),
                  nUpper  + (esMayuscula(c) ? 1 : 0),
                  nDigit  + (esDigito(c)    ? 1 : 0),
                  nSymbol + (esSimbolo(c)   ? 1 : 0),
                  pol);
    }
}

// ---------------------------------------------------------------- sin poda
void btSinPoda(string& password, int pos,
               int nLower, int nUpper, int nDigit, int nSymbol,
               const PoliticaBT& pol)
{
    ++nodos;

    if (pos == pol.n) {                 // se valida SOLO en la hoja
        if (nLower  >= pol.minLower  && nUpper  >= pol.minUpper &&
            nDigit  >= pol.minDigit   && nSymbol >= pol.minSymbol) ++soluciones;
        return;
    }

    for (char c : ALFABETO_BT) {
        if (pol.prohibirSeguidos && pos > 0 && c == password[pos - 1]) continue;
        password[pos] = c;
        btSinPoda(password, pos + 1,
                  nLower  + (esMinuscula(c) ? 1 : 0),
                  nUpper  + (esMayuscula(c) ? 1 : 0),
                  nDigit  + (esDigito(c)    ? 1 : 0),
                  nSymbol + (esSimbolo(c)   ? 1 : 0),
                  pol);
    }
}

int main(int argc, char** argv) {
    int nMax = (argc > 1) ? atoi(argv[1]) : 5;

    cout.precision(9);
    cout << fixed;
    cout << "variante,n,nodos,soluciones,segundos,repeticiones\n";

    for (int n = 2; n <= nMax; ++n) {
        PoliticaBT pol; pol.minLower = 1; pol.minUpper = 1;
        pol.minDigit = 1; pol.minSymbol = 1; pol.n = n; pol.prohibirSeguidos = true;

        for (int variante = 0; variante < 2; ++variante) {
            // Se repite la corrida hasta acumular >= 0.5 s (o 2000 repeticiones)
            // y se reporta el promedio: sin esto, los casos pequenos caen por
            // debajo de la resolucion del reloj y se miden como 0 s.
            string password(n, ' ');
            double acumulado = 0.0;
            long long reps = 0;
            do {
                nodos = 0; soluciones = 0;
                auto ini = chrono::steady_clock::now();
                if (variante == 0) btConPoda(password, 0, 0, 0, 0, 0, pol);
                else               btSinPoda(password, 0, 0, 0, 0, 0, pol);
                auto fin = chrono::steady_clock::now();
                acumulado += chrono::duration<double>(fin - ini).count();
                ++reps;
            } while (acumulado < 0.5 && reps < 2000);
            double seg = acumulado / reps;

            cout << (variante == 0 ? "con_poda" : "sin_poda") << ',' << n << ','
                 << nodos << ',' << soluciones << ',' << seg << ',' << reps << '\n' << flush;
        }
    }
    return 0;
}
