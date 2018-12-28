#pragma once
#include <chrono>
#include <mutex>
#include "Sinboha.h"

using namespace std;
using namespace SINBOHA_NSP;

class SinbohaStatusRep
{
public:
    SinbohaStatusRep();
    SinbohaStatus Query();
    void Query(chrono::system_clock::time_point& ChangeTime, SinbohaStatus& Status);
    bool AllowPeerActivate(chrono::system_clock::time_point& PeerChangeTime, SinbohaStatus& PeerStatus);
    void TryActivate(bool PeerAllowActivate);

private:
    mutex m_Lock;
    chrono::system_clock::time_point m_ChangeTime;
    chrono::system_clock::time_point m_QueryTime;
    SinbohaStatus m_Status;
};

class SinbohaImp : public SinbohaIf
{
public:
    static SinbohaImp* Instance();
    ~SinbohaImp();

    SinbohaError Initialize() override;
    SinbohaError Release() override;
    void RegisterCallback() override;
    void UnRegisterCallback() override;
    void SetHaStatus(SinbohaStatus Status) override;
    SinbohaStatus GetHaStatus() override;

    void TryActivate();
private:
    SinbohaImp();
    SinbohaImp(const SinbohaImp&) = delete;
    SinbohaImp(const SinbohaImp&&) = delete;
    SinbohaImp& operator=(const SinbohaImp&) = delete;
    SinbohaImp& operator=(const SinbohaImp&&) = delete;

    SinbohaStatusRep m_Status;
};

