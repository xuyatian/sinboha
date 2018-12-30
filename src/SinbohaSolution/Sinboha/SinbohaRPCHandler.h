#pragma once
#include "gen-cpp/SinbohaRPC.h"
#include "spdlog/spdlog.h"
#include "SinbohaImp.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace SINBOHA_NET;

class SinbohaRPCHandler : virtual public SinbohaRPCIf
{
public:
    SinbohaRPCHandler() 
    {
    }

    ~SinbohaRPCHandler()
    {
    }

    bool CanYouActivateMe(const int64_t ChangeTime, const int16_t Status) 
    {
        auto PeerChangeTime = chrono::system_clock::time_point(chrono::milliseconds(ChangeTime));
        return SinbohaImp::Instance()->IfAllowPeerActivate(PeerChangeTime, SinbohaStatus(Status));
    }

    int16_t SyncData(const std::string & Data) 
    {
        return (int16_t)SinbohaImp::Instance()->RecvData(Data);
    }
};

class SinbohaRPCFactory : public SinbohaRPCIfFactory
{
    SinbohaRPCIf * getHandler(const::apache::thrift::TConnectionInfo & connInfo) override
    {
        auto sock = dynamic_pointer_cast<TSocket>(connInfo.transport);
        if (sock)
        {
            SinbohaRPCIf* handler = new SinbohaRPCHandler;
            spdlog::default_logger()->info("New connection from:{}:{}, handler:{:x}.", sock->getPeerAddress(), sock->getPeerPort(), (uint64_t)handler);
            return handler;
        }
        return nullptr;
    }

    void releaseHandler(SinbohaRPCIf * handler) override
    {
        spdlog::default_logger()->info("Release handler:{:x}.", (uint64_t)handler);
        if (handler)
        {
            delete handler;
            handler = nullptr;
        }
    }
};
