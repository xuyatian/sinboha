#include "SinbohaNetClient.h"
#include "spdlog/spdlog.h"

void DummyOutput(const char* msg)
{
}

SinbohaNetClient::SinbohaNetClient()
{
}


SinbohaNetClient::~SinbohaNetClient()
{
    ReleaseClient();
}

SinbohaError SinbohaNetClient::Initialize(const std::string & PeerAddress, int Port, std::chrono::milliseconds NetworkTimeout)
{
    unique_lock<mutex> _(m_Lock);
    m_PeerAddress = PeerAddress;
    m_Port = Port;
    m_NetworkTimeout = NetworkTimeout;
    ReleaseClient();
    return InitializeClient(PeerAddress, Port, NetworkTimeout);
}

SinbohaError SinbohaNetClient::Release()
{
    unique_lock<mutex> _(m_Lock);
    return ReleaseClient();
}

SinbohaError SinbohaNetClient::ReInitialize()
{
    unique_lock<mutex> _(m_Lock);
    ReleaseClient();
    return InitializeClient(m_PeerAddress, m_Port, m_NetworkTimeout);
}

SinbohaError SinbohaNetClient::CanYouActivateMe(const chrono::system_clock::time_point & PeerChangeTime, const SinbohaStatus & PeerStatus, bool & Activate)
{
    Activate = false;
    auto epoch = chrono::duration_cast<chrono::milliseconds>(PeerChangeTime.time_since_epoch()).count();

    unique_lock<mutex> _(m_Lock);

    try
    {
        Activate = m_Client->CanYouActivateMe(epoch, (int16_t)PeerStatus);
    }
    catch (TTransportException ex)
    {
        spdlog::default_logger()->error("RPC call failed: {}.", ex.what());
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (TException ex)
    {
        spdlog::default_logger()->error("RPC call failed: {}.", ex.what());
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (...)
    {
        spdlog::default_logger()->error("RPC call failed.");
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    return SinbohaError::SINBOHA_ERROR_OK;
}

SinbohaError SinbohaNetClient::InitializeClient(const std::string & PeerAddress, int Port, std::chrono::milliseconds NetworkTimeout)
{
    GlobalOutput.setOutputFunction(DummyOutput);
    auto socket = boost::make_shared<TSocket>(PeerAddress, Port);
    socket->setConnTimeout(NetworkTimeout.count());
    socket->setRecvTimeout(NetworkTimeout.count());
    socket->setSendTimeout(NetworkTimeout.count());

    m_Transport = boost::make_shared<TBufferedTransport>(socket);
    boost::shared_ptr<TProtocol> protocol = boost::make_shared<TCompactProtocol>(m_Transport);
    m_Client = boost::make_shared<SinbohaRPCClient>(protocol);

    try
    {
        m_Transport->open();
    }
    catch (TTransportException ex)
    {
        spdlog::default_logger()->error("Fail to open transport: {}.", ex.what());
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (TException ex)
    {
        spdlog::default_logger()->error("Fail to open transport: {}.", ex.what());
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (...)
    {
        spdlog::default_logger()->error("Fail to open transport.");
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    return SinbohaError::SINBOHA_ERROR_OK;
}

SinbohaError SinbohaNetClient::ReleaseClient()
{
    if (!m_Transport)
    {
        return SinbohaError::SINBOHA_ERROR_OK;
    }

    try
    {
        m_Transport->close();
        m_Transport.reset();
    }
    catch (TTransportException ex)
    {
        spdlog::default_logger()->error("Fail to close transport: {}.", ex.what());
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (TException ex)
    {
        spdlog::default_logger()->error("Fail to close transport: {}.", ex.what());
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }
    catch (...)
    {
        spdlog::default_logger()->error("Fail to close transport.");
        return SinbohaError::SINBOHA_ERROR_FAIL;
    }

    return SinbohaError::SINBOHA_ERROR_OK;
}
