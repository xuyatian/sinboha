#include "SinbohaSync.h"
#include "SinbohaImp.h"
#include "spdlog/spdlog.h"

SinbohaSync::SinbohaSync():m_Quit(true),m_SwitchTimeout(3000),m_Heartbeat(chrono::milliseconds(500))
{
}


SinbohaSync::~SinbohaSync()
{
}

SinbohaError SinbohaSync::Start(const std::string & PeerAddress, int Port, std::chrono::milliseconds NetworkTimeout, std::chrono::milliseconds Heartbeat)
{
    m_Heartbeat = Heartbeat;
    m_PeerAddress = PeerAddress;
    m_PeerPort = Port;
    m_NetworkTimeout = NetworkTimeout;

    if (PeerAddress.empty())
    {
        spdlog::warn("Peer address is not available: {}.", PeerAddress);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    unique_lock<mutex> _(m_Lock);

    if (!m_Quit.load())
    {
        spdlog::default_logger()->warn("Heartbeat via {} is running.", PeerAddress);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    spdlog::default_logger()->info("Starting heartbeat via {}.", PeerAddress);
    m_Quit.store(false);

    m_Network = make_shared<SinbohaNetClient>();

    try
    {
        m_Future = std::async(launch::async, &SinbohaSync::SyncHeartbeat, this);
    }
    catch (std::system_error e)
    {
        spdlog::default_logger()->error("Fail to start heartbeat via {} : {}.", PeerAddress,e.what());
        m_Quit.store(true);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (std::bad_alloc e)
    {
        spdlog::default_logger()->error("Fail to start heartbeat via {} : {}.", PeerAddress,e.what());
        m_Quit.store(true);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (...)
    {
        spdlog::default_logger()->error("Fail to start heartbeat via {} : Unknown exception.", PeerAddress);
        m_Quit.store(true);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    spdlog::default_logger()->info("Started heartbeat.");
    return SinbohaError::SINBOHA_ERROR_OK;
}

SinbohaError SinbohaSync::Stop()
{
    {
        unique_lock<mutex> _(m_Lock);

        if (m_Quit.load())
        {
            spdlog::default_logger()->warn("Heartbeat via {} is stopped.", m_PeerAddress);
            return SinbohaError::SINBOHA_ERROR_FAIL;
        }

        spdlog::default_logger()->info("Stopping heartbeat via {}.", m_PeerAddress);

        m_Quit.store(true);
        m_Cond.notify_one();
    }

    m_Future.wait();

    m_Network->Release();

    spdlog::default_logger()->info("Stopped heartbeat via {}.", m_PeerAddress);

    return SinbohaError::SINBOHA_ERROR_OK;
}

SinbohaError SinbohaSync::SyncData(const string & Data)
{
    unique_lock<mutex> lk(m_Lock);

    if (m_Network)
    {
        return m_Network->SyncData(Data);
    }

    spdlog::default_logger()->error("Network is not available.");
    return SinbohaError::SINBOHA_ERROR_FAIL;
}

void SinbohaSync::SyncHeartbeat()
{
    m_Network->Initialize(m_PeerAddress, m_PeerPort, m_NetworkTimeout);

    chrono::system_clock::time_point ChangeTime;
    SinbohaStatus Status;
    bool Activate = false;
    chrono::milliseconds Interval(0);

    while (!m_Quit.load())
    {
        {
            unique_lock<mutex> lk(m_Lock);
            bool notify = m_Cond.wait_for(lk, Interval, [this]() {return m_Quit.load(); });
            if (notify)
            {
                spdlog::default_logger()->info("Exiting heartbeat via {}.", m_PeerAddress);
                return;
            }

            Interval = m_Heartbeat;
        }

        SinbohaImp::Instance()->Query(ChangeTime, Status);
        if (m_Network)
        {
            if (SinbohaError::SINBOHA_ERROR_OK == m_Network->CanYouActivateMe(ChangeTime, Status, Activate))
            {
                SinbohaImp::Instance()->TryActivate(Activate);
            }
            else
            {
                spdlog::default_logger()->warn("Network to {} down, reset.", m_PeerAddress);
                m_Network->ReInitialize();
            }
        }
    }
}
