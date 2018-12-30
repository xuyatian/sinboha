#pragma once
#include <chrono>
#include <mutex>
#include "Sinboha.h"
#include "SinbohaNetService.h"
#include "SinbohaSync.h"

using namespace std;
using namespace SINBOHA_NSP;

class SinbohaStatusRep
{
public:
    SinbohaStatusRep();
    SinbohaStatus Query();
    void Query(chrono::system_clock::time_point& ChangeTime, SinbohaStatus& Status);
    bool IfAllowPeerActivate(const chrono::system_clock::time_point& PeerChangeTime, const SinbohaStatus& PeerStatus);
    void TryActivate(bool PeerAllowActivate);
    void BrainSplit(const chrono::milliseconds& Idle);
    SinbohaError Switch();

    void RegisterCallback(shared_ptr<SinbohaCallbackIf>);
    void UnRegisterCallback();

    SinbohaError RecvData(const string& Data);
private:
    mutex m_Lock;
    chrono::system_clock::time_point m_ChangeTime;
    SinbohaStatus m_Status;
    shared_ptr<SinbohaCallbackIf> m_Callback;

    chrono::system_clock::time_point m_PeerAccessTime;
};

class SinbohaImp : public SinbohaIf
{
public:
    static SinbohaImp* Instance();
    ~SinbohaImp();

    SinbohaError Initialize(
        const std::string & PeerPrimaryAddress, 
        const std::string & PeerSecondaryAddress,
        int PeerPort, 
        int Port, 
        std::chrono::milliseconds NetworkTimeout,
        std::chrono::milliseconds Heartbeat, 
        std::chrono::milliseconds SwitchTimeout,
        bool Debug = false) override;

    SinbohaError Release() override;
    void RegisterCallback(shared_ptr<SinbohaCallbackIf>) override;
    void UnRegisterCallback() override;
    virtual SinbohaError Switch() override;
    SinbohaStatus GetHaStatus() override;
    SinbohaError SyncData(const std::string& Data) override;

    void Query(chrono::system_clock::time_point& ChangeTime, SinbohaStatus& Status);
    bool IfAllowPeerActivate(const chrono::system_clock::time_point& PeerChangeTime, const SinbohaStatus& PeerStatus);
    SinbohaError RecvData(const string& Data);
    void TryActivate(bool PeerAllowActivate);
    void BrainSplit(const chrono::milliseconds& PeerIdle);

private:
    SinbohaImp();
    SinbohaImp(const SinbohaImp&) = delete;
    SinbohaImp(const SinbohaImp&&) = delete;
    SinbohaImp& operator=(const SinbohaImp&) = delete;
    SinbohaImp& operator=(const SinbohaImp&&) = delete;

    SinbohaStatusRep m_Status;
    SinbohaNetService m_RPCService;
    SinbohaSync m_Sync;
};

