#include "SinbohaHeartbeat.h"
#include "SinbohaImp.h"
#include "spdlog/spdlog.h"

SinbohaHeartbeat::SinbohaHeartbeat():m_Quit(true),m_SwitchTimeout(3000),m_Heartbeat(chrono::milliseconds(500))
{
}


SinbohaHeartbeat::~SinbohaHeartbeat()
{
}

SinbohaError SinbohaHeartbeat::Start(const std::string & PeerPrimaryAddress, const std::string & PeerSecondaryAddress, int Port, std::chrono::milliseconds NetworkTimeout, std::chrono::milliseconds Heartbeat, std::chrono::milliseconds SwitchTimeout)
{
    m_SwitchTimeout = SwitchTimeout;
    m_Heartbeat = Heartbeat;
    unique_lock<mutex> _(m_Lock);

    if (!m_Quit.load())
    {
        spdlog::default_logger()->warn("Heartbeat is running.");
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    spdlog::default_logger()->info("Starting heartbeat.");
    m_Quit.store(false);

    if (!PeerPrimaryAddress.empty())
    {
        m_PrimaryNetwork = make_shared<SinbohaNetClient>();
        m_PrimaryNetwork->Initialize(PeerPrimaryAddress, Port, NetworkTimeout);
    }

    if (!PeerSecondaryAddress.empty())
    {
        m_SecondaryNetwork = make_shared<SinbohaNetClient>();
        m_SecondaryNetwork->Initialize(PeerPrimaryAddress, Port, NetworkTimeout);
    }

    try
    {
        m_Future = std::async(launch::async, &SinbohaHeartbeat::Heartbeat, this);
    }
    catch (std::system_error e)
    {
        spdlog::default_logger()->error("Fail to start heartbeat: {}.", e.what());
        m_Quit.store(true);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (std::bad_alloc e)
    {
        spdlog::default_logger()->error("Fail to start heartbeat: {}.", e.what());
        m_Quit.store(true);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (...)
    {
        spdlog::default_logger()->error("Fail to start heartbeat: Unknown exception.");
        m_Quit.store(true);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    spdlog::default_logger()->info("Started heartbeat.");
    return SinbohaError::SINBOHA_ERROR_OK;
}

SinbohaError SinbohaHeartbeat::Stop()
{
    {
        unique_lock<mutex> _(m_Lock);

        if (m_Quit.load())
        {
            spdlog::default_logger()->warn("Heartbeat is stopped.");
            return SinbohaError::SINBOHA_ERROR_FAIL;
        }

        spdlog::default_logger()->info("Stopping heartbeat.");

        m_Quit.store(true);
        m_Cond.notify_one();
    }

    m_Future.wait();

    m_PrimaryNetwork->Release();
    m_SecondaryNetwork->Release();

    spdlog::default_logger()->info("Stopped heartbeat.");

    return SinbohaError::SINBOHA_ERROR_OK;
}

void SinbohaHeartbeat::Heartbeat()
{
    chrono::system_clock::time_point ChangeTime;
    chrono::system_clock::time_point SuccessTime;
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
                spdlog::default_logger()->info("Exiting heartbeat.");
                return;
            }

            Interval = m_Heartbeat;
        }

        SinbohaImp::Instance()->Query(ChangeTime, Status);
        if (m_PrimaryNetwork)
        {
            if (SinbohaError::SINBOHA_ERROR_OK == m_PrimaryNetwork->CanYouActivateMe(ChangeTime, Status, Activate))
            {
                SinbohaImp::Instance()->TryActivate(Activate);
                SuccessTime = chrono::system_clock::now();
            }
            else
            {
                spdlog::default_logger()->warn("Primary network down, reset.");
                m_PrimaryNetwork->ReInitialize();

                if (m_SecondaryNetwork)
                {
                    spdlog::default_logger()->warn("Try secondary network.");
                    if (SinbohaError::SINBOHA_ERROR_OK == m_SecondaryNetwork->CanYouActivateMe(ChangeTime, Status, Activate))
                    {
                        SinbohaImp::Instance()->TryActivate(Activate);
                        SuccessTime = chrono::system_clock::now();
                    }
                    else
                    {
                        spdlog::default_logger()->warn("Secondary network down, reset.");
                        m_SecondaryNetwork->ReInitialize();
                    }
                }
            }
        }

        if (chrono::system_clock::now()- SuccessTime > m_SwitchTimeout)
        {
            spdlog::default_logger()->debug("Brain split, activate myself.");
            SinbohaImp::Instance()->BrainSplit(m_SwitchTimeout);
            SuccessTime = chrono::system_clock::now();
        }
    }
}
