#pragma once

#ifdef SINBOHA_DLL_EXPORT
#define SINBOHA_DLL dllexport
#else
#define SINBOHA_DLL dllimport
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

    class SinboHaCallbackIf
    {
    public:
        virtual void OnStatusChange(SinbohaStatus Status) = 0;
        virtual void OnReceiveTag() = 0;
        virtual void OnReceiveData() = 0;
    };

    class SINBOHA_DLL SinbohaIf
    {
     public:
         static SinbohaIf* Instance();
         virtual SinbohaError Initialize() = 0;
         virtual SinbohaError Release() = 0;
         virtual void RegisterCallback() = 0;
         virtual void UnRegisterCallback() = 0;
         virtual void SetHaStatus(SinbohaStatus Status) = 0;
         virtual SinbohaStatus GetHaStatus() = 0;
    };
}
