#include "SinbohaImp.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

SinbohaImp::SinbohaImp()
{
    auto rotating_logger = spdlog::rotating_logger_mt("ha", "sinboha.log", 1048576 * 5, 3);
    spdlog::get("ha")->set_pattern("%L %Y-%m-%d %H:%M:%S.%e %t >>> %v");
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
