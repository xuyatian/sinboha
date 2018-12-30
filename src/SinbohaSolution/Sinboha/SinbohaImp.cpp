#include "SinbohaImp.h"
#include "SinbohaHelper.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"


SinbohaImp::SinbohaImp()
{
    auto logger = spdlog::rotating_logger_mt("ha", "sinboha.log", 1048576 * 5, 3);
    spdlog::get("ha")->set_pattern("%L %Y-%m-%d %H:%M:%S.%e %t %v");
    spdlog::set_default_logger(logger);
    spdlog::default_logger()->flush_on(spdlog::level::info);
    spdlog::default_logger()->set_level(spdlog::level::info);
}

SinbohaError SinbohaImp::SyncData(const string& Data)
{
    if (SinbohaStatus::SINBOHA_STATUS_ACTIVE != m_Status.Query())
    {
        spdlog::default_logger()->error("I am not active, ignore data synchronization.");
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    return m_Sync.SyncData(Data);
}

SinbohaError SinbohaImp::Initialize(
    const std::string & PeerPrimaryAddress, 
    const std::string & PeerSecondaryAddress,
    int PeerPort,
    int Port,
    std::chrono::milliseconds NetworkTimeout, 
    std::chrono::milliseconds Heartbeat,
    std::chrono::milliseconds SwitchTimeout,
    bool Debug)
{
    if (Debug)
    {
        spdlog::default_logger()->flush_on(spdlog::level::debug);
        spdlog::default_logger()->set_level(spdlog::level::debug);
    }

    if (SinbohaError::SINBOHA_ERROR_OK != m_RPCService.Start(Port, NetworkTimeout.count()))
    {
        spdlog::default_logger()->error("Could not start RPC service.");
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    //wait for rpc service is ready.
    this_thread::sleep_for(chrono::milliseconds(200));

    if (SinbohaError::SINBOHA_ERROR_OK != m_Sync.Start(
        PeerPrimaryAddress,
        PeerSecondaryAddress, 
        PeerPort,
        NetworkTimeout,
        Heartbeat, 
        SwitchTimeout))
    {
        spdlog::default_logger()->error("Could not start heartbeat.");
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    return SinbohaError::SINBOHA_ERROR_OK;
}

SinbohaError SinbohaImp::Switch()
{
    return m_Status.Switch();
}


SinbohaImp * SinbohaImp::Instance()
{
    static SinbohaImp ins;
    return &ins;
}

SinbohaImp::~SinbohaImp()
{
}

SinbohaError SinbohaImp::Release()
{
    m_Sync.Stop();
    m_RPCService.Stop();
    return SinbohaError();
}

void SinbohaImp::RegisterCallback(shared_ptr<SinbohaCallbackIf> Callback)
{
    m_Status.RegisterCallback(Callback);
}

void SinbohaImp::UnRegisterCallback()
{
    m_Status.UnRegisterCallback();
}

SinbohaStatus SinbohaImp::GetHaStatus()
{
    return m_Status.Query();
}

void SinbohaImp::Query(chrono::system_clock::time_point & ChangeTime, SinbohaStatus & Status)
{
    return m_Status.Query(ChangeTime, Status);
}

bool SinbohaImp::IfAllowPeerActivate(const chrono::system_clock::time_point & PeerChangeTime, const SinbohaStatus & PeerStatus)
{
    return m_Status.IfAllowPeerActivate(PeerChangeTime, PeerStatus);
}

SinbohaError SinbohaImp::RecvData(const string & Data)
{
    return m_Status.RecvData(Data);
}

void SinbohaImp::TryActivate(bool PeerAllowActivate)
{
    return m_Status.TryActivate(PeerAllowActivate);
}

void SinbohaImp::BrainSplit(const chrono::milliseconds& Idle)
{
    return m_Status.BrainSplit(Idle);
}

SinbohaStatusRep::SinbohaStatusRep():
    m_ChangeTime(chrono::system_clock::now()),
    m_Status(SinbohaStatus::SINBOHA_STATUS_PENDING)
{
}

SinbohaStatus SinbohaStatusRep::Query()
{
    unique_lock<mutex> _(m_Lock);
    spdlog::default_logger()->debug("Query status: {}.", m_Status);
    return m_Status;
}

void SinbohaStatusRep::Query(chrono::system_clock::time_point & ChangeTime, SinbohaStatus & Status)
{
    unique_lock<mutex> _(m_Lock);
    spdlog::default_logger()->debug("Query status: {}/{}.", m_Status, SINBOHA_HELPER::ToString(m_ChangeTime));

    ChangeTime = m_ChangeTime;
    Status = m_Status;
}

bool SinbohaStatusRep::IfAllowPeerActivate(const chrono::system_clock::time_point & PeerChangeTime, const SinbohaStatus & PeerStatus)
{
    /*
    Pending--->Active ����No, ���Standby
    Pending--->Standby ����Yes, ���Active
    Pending--->Pending �Ա�ʱ�䣬�������ı��Active
    Active--->Active �Ա�ʱ�䣬�ȼ���ı��Standby
    Active--->Standby ����Yes
    Active--->Pending ����Yes
    Standby--->Active ����No
    Standby--->Standby �Ա�ʱ�䣬�ȱ��Standby�ı��Active
    Standby--->Pending ����No
    */

    unique_lock<mutex> _(m_Lock);

    m_PeerAccessTime = chrono::system_clock::now();

    spdlog::default_logger()->debug("Peer status: {}/{}, my status: {}/{}.",
        PeerStatus,
        SINBOHA_HELPER::ToString(PeerChangeTime),
        m_Status,
        SINBOHA_HELPER::ToString(m_ChangeTime));

    if (PeerStatus == SinbohaStatus::SINBOHA_STATUS_PENDING &&
        m_Status == SinbohaStatus::SINBOHA_STATUS_ACTIVE)
    {
        spdlog::default_logger()->debug("I say NO.");
        return false;
    }

    if (PeerStatus == SinbohaStatus::SINBOHA_STATUS_PENDING &&
        m_Status == SinbohaStatus::SINBOHA_STATUS_STANDBY)
    {
        spdlog::default_logger()->debug("I say Yes.");
        return true;
    }

    if (PeerStatus == SinbohaStatus::SINBOHA_STATUS_PENDING &&
        m_Status == SinbohaStatus::SINBOHA_STATUS_PENDING)
    {
        spdlog::default_logger()->debug("I say {}.", PeerChangeTime < m_ChangeTime ? "Yes" : "No");
        return PeerChangeTime < m_ChangeTime;
    }

    if (PeerStatus == SinbohaStatus::SINBOHA_STATUS_ACTIVE &&
        m_Status == SinbohaStatus::SINBOHA_STATUS_ACTIVE)
    {
        spdlog::default_logger()->debug("I say {}.", PeerChangeTime > m_ChangeTime ? "Yes" : "No");
        return PeerChangeTime > m_ChangeTime;
    }

    if (PeerStatus == SinbohaStatus::SINBOHA_STATUS_ACTIVE &&
        m_Status == SinbohaStatus::SINBOHA_STATUS_STANDBY)
    {
        spdlog::default_logger()->debug("I say Yes.");
        return true;
    }

    if (PeerStatus == SinbohaStatus::SINBOHA_STATUS_ACTIVE &&
        m_Status == SinbohaStatus::SINBOHA_STATUS_PENDING)
    {
        spdlog::default_logger()->debug("I say Yes.");
        return true;
    }

    if (PeerStatus == SinbohaStatus::SINBOHA_STATUS_STANDBY && 
        m_Status == SinbohaStatus::SINBOHA_STATUS_ACTIVE) 
    {
        spdlog::default_logger()->debug("I say No.");
        return false;
    }

    if (PeerStatus == SinbohaStatus::SINBOHA_STATUS_STANDBY &&
        m_Status == SinbohaStatus::SINBOHA_STATUS_STANDBY) 
    {
        spdlog::default_logger()->debug("I say {}.", PeerChangeTime < m_ChangeTime ? "Yes" : "No");
        return PeerChangeTime < m_ChangeTime;
    }

    if (PeerStatus == SinbohaStatus::SINBOHA_STATUS_STANDBY &&
        m_Status == SinbohaStatus::SINBOHA_STATUS_PENDING) 
    {
        spdlog::default_logger()->debug("I say No.");
        return false;
    }

    spdlog::default_logger()->debug("I say No.");
    return false;
}

void SinbohaStatusRep::TryActivate(bool PeerAllowActivate)
{
    unique_lock<mutex> _(m_Lock);

    spdlog::default_logger()->debug("Peer activate me? {}.", PeerAllowActivate ? "Yes" : "No");

    if (PeerAllowActivate)
    {
        if (SinbohaStatus::SINBOHA_STATUS_ACTIVE != m_Status)
        {
            auto now = chrono::system_clock::now();

            spdlog::default_logger()->info("**SWITCH** status from {}/{} to {}/{}.",
                m_Status,
                SINBOHA_HELPER::ToString(m_ChangeTime),
                SinbohaStatus::SINBOHA_STATUS_ACTIVE,
                SINBOHA_HELPER::ToString(now));

            m_Status = SinbohaStatus::SINBOHA_STATUS_ACTIVE;
            m_ChangeTime = now;

            if (m_Callback) m_Callback->OnStatusChange(m_Status);
        }
    }
    else
    {
        if (SinbohaStatus::SINBOHA_STATUS_STANDBY != m_Status)
        {
            auto now = chrono::system_clock::now();

            spdlog::default_logger()->info("**SWITCH** status from {}/{} to {}/{}.",
                m_Status,
                SINBOHA_HELPER::ToString(m_ChangeTime),
                SinbohaStatus::SINBOHA_STATUS_STANDBY,
                SINBOHA_HELPER::ToString(now));

            m_Status = SinbohaStatus::SINBOHA_STATUS_STANDBY;
            m_ChangeTime = now;

            if (m_Callback) m_Callback->OnStatusChange(m_Status);
        }
    }
}

void SinbohaStatusRep::BrainSplit(const chrono::milliseconds& Idle)
{
    unique_lock<mutex> _(m_Lock);

    spdlog::default_logger()->debug("Brain split, try to active myself.");

    if (SinbohaStatus::SINBOHA_STATUS_ACTIVE == m_Status)
    {
        spdlog::default_logger()->debug("Brain split, ignore activate.");
        return;
    }

    auto now = chrono::system_clock::now();
    auto PeerIdle = now - m_PeerAccessTime;
    if (PeerIdle < Idle)
    {
        spdlog::default_logger()->debug("Brain split, peer idle: {}ms < {}ms, ignore.", PeerIdle.count(), Idle.count());
        return;
    }

    spdlog::default_logger()->info("Brain split, **SWITCH** status from {}/{} to {}/{}.",
        m_Status,
        SINBOHA_HELPER::ToString(m_ChangeTime),
        SinbohaStatus::SINBOHA_STATUS_ACTIVE,
        SINBOHA_HELPER::ToString(now));

    m_Status = SinbohaStatus::SINBOHA_STATUS_ACTIVE;
    m_ChangeTime = now;

    if (m_Callback) m_Callback->OnStatusChange(m_Status);
}

SinbohaError SinbohaStatusRep::Switch()
{
    unique_lock<mutex> _(m_Lock);

    if (SinbohaStatus::SINBOHA_STATUS_ACTIVE == m_Status)
    {
        auto now = chrono::system_clock::now();
        spdlog::default_logger()->info("Manual switch, **SWITCH** status from {}/{} to {}/{}.",
            m_Status,
            SINBOHA_HELPER::ToString(m_ChangeTime),
            SinbohaStatus::SINBOHA_STATUS_STANDBY,
            SINBOHA_HELPER::ToString(now));

        m_Status = SinbohaStatus::SINBOHA_STATUS_STANDBY;
        m_ChangeTime = now;

        if (m_Callback) m_Callback->OnStatusChange(m_Status);
        return SinbohaError::SINBOHA_ERROR_OK;
    }

    spdlog::default_logger()->warn("Switch is only allowed on active node.");
    return SinbohaError::SINBOHA_ERROR_FAIL;
}

void SinbohaStatusRep::RegisterCallback(shared_ptr<SinbohaCallbackIf> Callback)
{
    unique_lock<mutex> _(m_Lock);
    m_Callback = Callback;
}

void SinbohaStatusRep::UnRegisterCallback()
{
    unique_lock<mutex> _(m_Lock);
    m_Callback.reset();
}

SinbohaError SinbohaStatusRep::RecvData(const string & Data)
{
    unique_lock<mutex> _(m_Lock);
    if (SinbohaStatus::SINBOHA_STATUS_STANDBY != m_Status)
    {
        spdlog::default_logger()->error("I am not standby, ignore data synchronization.");
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    if (m_Callback)
    {
        m_Callback->OnReceiveData(Data);
    }
    return SinbohaError::SINBOHA_ERROR_OK;
}
