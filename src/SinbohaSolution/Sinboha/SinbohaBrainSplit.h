#pragma once
#include <mutex>
#include <future>
#include <atomic>
#include <condition_variable>
#include "Sinboha.h"

using namespace std;
using namespace SINBOHA_NSP;

class SinbohaBrainSplit
{
public:
    SinbohaBrainSplit();
    ~SinbohaBrainSplit();

    SinbohaError Start(std::chrono::milliseconds SwitchTimeout);

    SinbohaError Stop();
private:
    void BrainSplitCheck();
    atomic<bool> m_Quit;
    mutex m_Lock;
    condition_variable m_Cond;
    future<void> m_Future;
    chrono::milliseconds m_SwitchTimeout;
};

