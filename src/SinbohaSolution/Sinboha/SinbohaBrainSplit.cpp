#include "SinbohaImp.h"
#include "SinbohaBrainSplit.h"
#include "spdlog/spdlog.h"
#include "SinbohaHelper.h"

SinbohaBrainSplit::SinbohaBrainSplit():m_Quit(true)
{
}


SinbohaBrainSplit::~SinbohaBrainSplit()
{
}

SinbohaError SinbohaBrainSplit::Start(std::chrono::milliseconds SwitchTimeout)
{
    m_SwitchTimeout = SwitchTimeout;

    unique_lock<mutex> _(m_Lock);

    if (!m_Quit.load())
    {
        spdlog::default_logger()->warn("Brain split check is running.");
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    spdlog::default_logger()->info("Starting brain split check.");
    m_Quit.store(false);

    try
    {
        m_Future = std::async(launch::async, &SinbohaBrainSplit::BrainSplitCheck, this);
    }
    catch (std::system_error e)
    {
        spdlog::default_logger()->error("Fail to start brain split check: {}.", e.what());
        m_Quit.store(true);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (std::bad_alloc e)
    {
        spdlog::default_logger()->error("Fail to start brain split check: {}.", e.what());
        m_Quit.store(true);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (...)
    {
        spdlog::default_logger()->error("Fail to start brain split check: Unknown exception.");
        m_Quit.store(true);
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    spdlog::default_logger()->info("Started brain split check.");
    return SinbohaError::SINBOHA_ERROR_OK;
}

SinbohaError SinbohaBrainSplit::Stop()
{
    {
        unique_lock<mutex> _(m_Lock);

        if (m_Quit.load())
        {
            spdlog::default_logger()->warn("Brain split check is stopped.");
            return SinbohaError::SINBOHA_ERROR_FAIL;
        }

        spdlog::default_logger()->info("Stopping brain split check.");

        m_Quit.store(true);
        m_Cond.notify_one();
    }

    m_Future.wait();

    spdlog::default_logger()->info("Stopped brain split check.");

    return SinbohaError::SINBOHA_ERROR_OK;
}

void SinbohaBrainSplit::BrainSplitCheck()
{
    while (!m_Quit.load())
    {
        {
            unique_lock<mutex> lk(m_Lock);
            bool notify = m_Cond.wait_for(lk, chrono::milliseconds(100), [this]() {return m_Quit.load(); });
            if (notify)
            {
                spdlog::default_logger()->info("Exiting brain split check.");
                return;
            }
        }

        auto PeerLiveTime = SinbohaImp::Instance()->PeerLiveTime();
        if (chrono::system_clock::now() - PeerLiveTime > m_SwitchTimeout)
        {
            spdlog::default_logger()->debug("brain split, peer not response since: {}.", SINBOHA_HELPER::ToString(PeerLiveTime));
            SinbohaImp::Instance()->BrainSplit();
        }
    }
}
