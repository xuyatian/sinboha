#include "SinbohaNetService.h"
#include "SinbohaRPCHandler.h"
#include "spdlog/spdlog.h"

SinbohaNetService::SinbohaNetService():m_Quit(true)
{
}


SinbohaNetService::~SinbohaNetService()
{
}

SinbohaError SinbohaNetService::Start(int Port, int Timeout)
{
    unique_lock<mutex> _(m_Lock);

    if (!m_Quit.load())
    {
        spdlog::default_logger()->warn("RPC service is running.");
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    spdlog::default_logger()->info("Starting RPC service.");
    m_Quit.store(false);

    boost::shared_ptr<TProtocolFactory> protocolFactory = boost::make_shared<TCompactProtocolFactory>();
    boost::shared_ptr<TTransportFactory> transportFactory = boost::make_shared<TBufferedTransportFactory>();
    m_Server = boost::make_shared<TThreadedServer>(
        boost::make_shared<SinbohaRPCProcessorFactory>(boost::make_shared<SinbohaRPCFactory>()),
        boost::make_shared<TServerSocket>(Port, Timeout, 1000 * 60),
        transportFactory,
        protocolFactory);

    try {
        m_Future = std::async(launch::async, &SinbohaNetService::RPCService, this);
    }
    catch (std::system_error e)
    {
        spdlog::default_logger()->error("Fail to start RPC service: {}.", e.what());
        m_Quit.store(true);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (std::bad_alloc e)
    {
        spdlog::default_logger()->error("Fail to start RPC service: {}.", e.what());
        m_Quit.store(true);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (...)
    {
        spdlog::default_logger()->error("Fail to start RPC service: Unknown exception.");
        m_Quit.store(true);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    spdlog::default_logger()->info("Started RPC service.");
    return SinbohaError::SINBOHA_ERROR_OK;
}

SinbohaError SinbohaNetService::Stop()
{
    unique_lock<mutex> _(m_Lock);

    if (m_Quit.load())
    {
        spdlog::default_logger()->warn("RPC service is stopped.");
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    spdlog::default_logger()->info("Stopping RPC service.");

    m_Quit.store(true);

    try
    {
        m_Server->stop();
    }
    catch (apache::thrift::TException ex)
    {
        spdlog::default_logger()->error(ex.what());
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (...)
    {
        spdlog::default_logger()->error("Unknown exception in stopping RPC service.");
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    m_Future.wait();
    spdlog::default_logger()->info("Stopped RPC service.");

    return SinbohaError::SINBOHA_ERROR_OK;
}

void SinbohaNetService::RPCService()
{
    while (!m_Quit.load())
    {
        try
        {
            m_Server->serve();
        }
        catch (apache::thrift::TException ex)
        {
            spdlog::default_logger()->error(ex.what());
            this_thread::sleep_for(chrono::seconds(1));
            continue;
        }
        catch (...)
        {
            spdlog::default_logger()->error("Unknown exception in RPC service.");
            this_thread::sleep_for(chrono::seconds(1));
            continue;
        }
    }
}

