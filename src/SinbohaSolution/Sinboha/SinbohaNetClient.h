#pragma once
#include <mutex>
#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include "Sinboha.h"
#include "gen-cpp/SinbohaRPC.h"

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace SINBOHA_NSP;
using namespace SINBOHA_NET;

class SinbohaNetClient
{
public:
    SinbohaNetClient();
    ~SinbohaNetClient();

    SinbohaError Initialize(
        const std::string & PeerAddress,
        int Port,
        std::chrono::milliseconds NetworkTimeout);

    SinbohaError Release();
    SinbohaError ReInitialize();

    SinbohaError CanYouActivateMe(
        const chrono::system_clock::time_point& PeerChangeTime,
        const SinbohaStatus& PeerStatus,
        bool& Activate);

    SinbohaError SyncData(const string& Data);

private:
    SinbohaError InitializeClient(
        const std::string & PeerAddress,
        int Port,
        std::chrono::milliseconds NetworkTimeout);

    SinbohaError ReleaseClient();

    boost::shared_ptr<TTransport> m_Transport;
    boost::shared_ptr<SinbohaRPCClient> m_Client;
    mutex m_Lock;

    std::string m_PeerAddress;
    int m_Port;
    std::chrono::milliseconds m_NetworkTimeout;
};

