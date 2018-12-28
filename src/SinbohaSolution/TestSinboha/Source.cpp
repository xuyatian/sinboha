#include <chrono>
#include <iostream>
#include "../Sinboha/Sinboha.h"

using namespace std;
using namespace SINBOHA_NSP;
int main()
{
    auto ha = GetSinbohaIf();

    auto now = chrono::system_clock::now();

    auto epoch = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
    cout << epoch << endl;

    auto now2 = chrono::system_clock::time_point(chrono::milliseconds(epoch));
    epoch =  chrono::duration_cast<chrono::milliseconds>(now2.time_since_epoch()).count();
    cout << epoch << endl;

    auto now3 = chrono::system_clock::time_point();
    epoch =  chrono::duration_cast<chrono::milliseconds>(now3.time_since_epoch()).count();
    cout << epoch << endl;
}