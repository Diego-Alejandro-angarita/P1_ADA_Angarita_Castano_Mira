#include <cstdint>
#include <string>
using namespace std;

const string ALFABETO_BT =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789"
    "!@#$%";

struct PoliticaBT {
    int minLower = 2;
    int minUpper = 2;
    int minDigit = 3;
    int minSymbol = 1;
    int n;       
    bool prohibirSeguidos = true;
};