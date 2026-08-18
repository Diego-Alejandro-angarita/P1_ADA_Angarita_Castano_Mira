#include <iostream>
#include <String>
#include "third_party/Picosha2.H"
#include <vector>
#include "resources/BTDatos.hpp"

using namespace std;
using namespace picosha2;

void BackTracking(string& password, int pos, const string& A1, const string& A2) {
    if (pos == 8) {
        cout << "Password generada: " << password << "\n";
        return;
    }


}