#pragma once
#include <future>
#include <atomic>
#include <memory>
#include <condition_variable>
#include "Sinboha.h"
#include "SinbohaNetClient.h"

using namespace std;
using namespace SINBOHA_NSP;

class SinbohaSync
{
public:
    SinbohaSync();
    ~SinbohaSync();

    SinbohaError Start(
        const std::string & PeerAddress,
        int Port,
        std::chrono::milliseconds NetworkTimeout,
        std::chrono::milliseconds Heartbeat);

    SinbohaError Stop();

    SinbohaError SyncData(const string& Data);
private:
    void SyncHeartbeat();
    future<void> m_Future;
    condition_variable m_Cond;
    atomic<bool> m_Quit;

    mutex m_Lock;

    shared_ptr<SinbohaNetClient> m_Network;
    string m_PeerAddress;
    int m_PeerPort;
    chrono::milliseconds m_NetworkTimeout;

    chrono::milliseconds m_Heartbeat;
    chrono::milliseconds m_SwitchTimeout;
};

