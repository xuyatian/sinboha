#include "SinbohaHelper.h"

string SINBOHA_HELPER::ToString(const chrono::time_point<chrono::system_clock>& SinbohaTime)
{
    auto s = SinbohaTime.time_since_epoch();
    auto diff = chrono::duration_cast<chrono::milliseconds>(s).count();
    auto const msecs = diff % 1000;

    std::time_t t = chrono::system_clock::to_time_t(SinbohaTime);

    stringstream ss;
    ss << std::put_time(std::localtime(&t), "%F %T") << "." << msecs;
    return ss.str();
}
