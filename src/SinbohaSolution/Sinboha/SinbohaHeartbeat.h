#pragma once
#include <future>
#include <atomic>
#include <memory>
#include <condition_variable>
#include "Sinboha.h"
#include "SinbohaNetClient.h"

using namespace std;
using namespace SINBOHA_NSP;

class SinbohaHeartbeat
{
public:
    SinbohaHeartbeat();
    ~SinbohaHeartbeat();

    SinbohaError Start(
        const std::string & PeerPrimaryAddress, 
        const std::string & PeerSecondaryAddress,
        int Port, 
        std::chrono::milliseconds NetworkTimeout,
        std::chrono::milliseconds Heartbeat, 
        std::chrono::milliseconds SwitchTimeout);

    SinbohaError Stop();
private:
    void Heartbeat();
    future<void> m_Future;
    condition_variable m_Cond;
    atomic<bool> m_Quit;

    mutex m_Lock;

    shared_ptr<SinbohaNetClient> m_PrimaryNetwork;
    shared_ptr<SinbohaNetClient> m_SecondaryNetwork;
    chrono::milliseconds m_Heartbeat;
    chrono::milliseconds m_SwitchTimeout;
};

