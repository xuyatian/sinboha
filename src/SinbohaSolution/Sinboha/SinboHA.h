#pragma once

#include <chrono>
#include <string>
#include <memory>

#ifdef SINBOHA_DLL_EXPORT
#define SINBOHA_DLL __declspec(dllexport)
#else
#define SINBOHA_DLL __declspec(dllimport)
#endif

namespace SINBOHA_NSP 
{
    enum SinbohaError : unsigned short 
    {
        SINBOHA_ERROR_OK = 0,
        SINBOHA_ERROR_FAIL,
    };

    enum SinbohaStatus: unsigned short 
    {
        SINBOHA_STATUS_PENDING = 0,
        SINBOHA_STATUS_ACTIVE,
        SINBOHA_STATUS_STANDBY,
    };

    class SinbohaCallbackIf
    {
    public:
        virtual void OnStatusChange(SinbohaStatus Status) = 0;
        virtual void OnReceiveData(const std::string& Data) = 0;
    };

    class SinbohaIf
    {
     public:
         virtual SinbohaError Initialize(
             const std::string& PeerPrimaryAddress,
             const std::string& PeerSecondaryAddress,
             int PeerPort,
             int Port,
             std::chrono::milliseconds NetworkTimeout,
             std::chrono::milliseconds Heartbeat,
             std::chrono::milliseconds SwitchTimeout,
             bool Debug = false) = 0;

         virtual SinbohaError Release() = 0;
         virtual void RegisterCallback(std::shared_ptr<SinbohaCallbackIf>) = 0;
         virtual void UnRegisterCallback() = 0;
         virtual SinbohaError SyncData(const std::string& Data) = 0;
         virtual SinbohaError Switch() = 0;
         virtual SinbohaStatus GetHaStatus() = 0;
    };
}

SINBOHA_DLL SINBOHA_NSP::SinbohaIf* GetSinbohaIf();
