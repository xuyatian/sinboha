#include "SinbohaImp.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

SinbohaImp::SinbohaImp()
{
    auto rotating_logger = spdlog::rotating_logger_mt("ha", "sinboha.log", 1048576 * 5, 3);
    spdlog::get("ha")->set_pattern("%L %Y-%m-%d %H:%M:%S.%e %t %v");
    spdlog::set_default_logger(rotating_logger);
}


SinbohaImp * SinbohaImp::Instance()
{
    static SinbohaImp ins;
    return &ins;
}

SinbohaImp::~SinbohaImp()
{
}

SinbohaError SinbohaImp::Initialize()
{
    return SinbohaError();
}

SinbohaError SinbohaImp::Release()
{
    return SinbohaError();
}

void SinbohaImp::RegisterCallback()
{
}

void SinbohaImp::UnRegisterCallback()
{
}

void SinbohaImp::SetHaStatus(SinbohaStatus Status)
{
}

SinbohaStatus SinbohaImp::GetHaStatus()
{
    return SinbohaStatus();
}

SinbohaStatusRep::SinbohaStatusRep():
    m_ChangeTime(chrono::system_clock::now()),
    m_Status(SinbohaStatus::SINBOHA_STATUS_PENDING)
{
}

SinbohaStatus SinbohaStatusRep::Query()
{
    unique_lock<mutex> _(m_Lock);
    return m_Status;
}

void SinbohaStatusRep::Query(chrono::system_clock::time_point & ChangeTime, SinbohaStatus & Status)
{
    unique_lock<mutex> _(m_Lock);
    m_QueryTime = chrono::system_clock::now();

    ChangeTime = m_ChangeTime;
    Status = m_Status;
}

bool SinbohaStatusRep::AllowPeerActivate(chrono::system_clock::time_point & PeerChangeTime, SinbohaStatus & PeerStatus)
{
    return false;
}

void SinbohaStatusRep::TryActivate(bool PeerAllowActivate)
{
}
