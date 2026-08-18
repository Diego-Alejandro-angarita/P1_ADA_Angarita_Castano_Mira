
#include <string>
using namespace std;

const string ALFABETO_BT =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789"
    "!@#$%";

struct PoliticaBT {
    int minLower;
    int minUpper;
    int minDigit;
    int minSymbol;
    int n;       
    bool prohibirConsecutivosIdenticos = true;
};