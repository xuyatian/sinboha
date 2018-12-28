#include "SinboHA.h"
#include "SinbohaImp.h"
using namespace SINBOHA_NSP;

SINBOHA_DLL SinbohaIf* GetSinbohaIf()
{
    return SinbohaImp::Instance();
}
