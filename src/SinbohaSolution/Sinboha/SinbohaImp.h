#pragma once
#include "Sinboha.h"

using namespace SINBOHA_NSP;

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

private:
    SinbohaImp();
    SinbohaImp(const SinbohaImp&) = delete;
    SinbohaImp(const SinbohaImp&&) = delete;
    SinbohaImp& operator=(const SinbohaImp&) = delete;
    SinbohaImp& operator=(const SinbohaImp&&) = delete;
};

