#pragma once
#include <future>
#include <atomic>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include "Sinboha.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace SINBOHA_NSP;

class SinbohaNetService
{
public:
    SinbohaNetService();
    ~SinbohaNetService();

    SinbohaError Start(int Port, int TimeOut);
    SinbohaError Stop();

private:
    void RPCService();

    future<void> m_Future;
    boost::shared_ptr<TServer> m_Server;
    atomic<bool> m_Quit;
    mutex m_Lock;
};


