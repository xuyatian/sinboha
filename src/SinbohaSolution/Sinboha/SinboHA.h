/*************************************************************************************************//**
@file     Sinboha.h
@brief    HA(High availability)
@note     运行环境VS2015
*****************************************************************************************************/
#pragma once

#include <chrono>
#include <string>
#include <memory>

#ifdef SINBOHA_DLL_EXPORT
#define SINBOHA_DLL __declspec(dllexport)
#else
#define SINBOHA_DLL __declspec(dllimport)
#endif

/*************************************************************************************************//**
* @brief HA namespace
*****************************************************************************************************/
namespace SINBOHA_NSP 
{

    /*************************************************************************************************//**
    * @brief 错误码
    *****************************************************************************************************/
    enum SinbohaError : unsigned short 
    {
        SINBOHA_ERROR_OK = 0,   /**< 操作成功 */
        SINBOHA_ERROR_FAIL,     /**< 操作失败 */
    };

    /*************************************************************************************************//**
    * @brief HA状态定义
    *****************************************************************************************************/
    enum SinbohaStatus: unsigned short 
    {
        SINBOHA_STATUS_PENDING = 0,     /**< 初始未定义状态 */
        SINBOHA_STATUS_ACTIVE,          /**< 激活状态 */
        SINBOHA_STATUS_STANDBY,         /**< 待机状态 */
    };

    /*************************************************************************************************//**
    * @brief HA回调接口
    *****************************************************************************************************/
    class SinbohaCallbackIf
    {
    public:
        /*************************************************************************************************//**
        *    HA状态变化回调方法
        *
        *    @param          Status
        *                    变化后的状态。
        *    @return         void
        *****************************************************************************************************/
        virtual void OnStatusChange(SinbohaStatus Status) = 0;

        /*************************************************************************************************//**
        *    HA数据同步回调方法
        *
        *    @param          Data
        *                    来自激活端同步的数据。
        *    @return         void
        *    @note           如果当前为非激活状态，则该方法不会被回调。
        *****************************************************************************************************/
        virtual void OnReceiveData(const std::string& Data) = 0;
    };

    /*************************************************************************************************//**
    * @brief HA接口
    *****************************************************************************************************/
    class SinbohaIf
    {
     public:
        /*************************************************************************************************//**
        *    HA接口初始化方法
        *
        *    @param          PeerPrimaryAddress
        *                    对端主网络地址(IP或主机名)。
        *    @param          PeerSecondaryAddress
        *                    对端备用网络地址(IP或主机名)。
        *    @param          PeerPort
        *                    对端同步端口号，与对端Port参数对应。
        *    @param          Port
        *                    本机同步端口号，与对端PeerPort端口对应。同时保留2个端口号，以便在本机调试运行。
        *    @param          NetworkTimeout
        *                    网络超时，TCP连接，发送、接收超时。建议200ms。
        *    @param          Heartbeat
        *                    双方同步状态时间间隔。建议500ms。
        *    @param          SwitchTimeout
        *                    对端无响应切换超时。如果在SwitchTimeout时间内无法与对端通信，则自动切换为激活状态。建议1000ms。
        *    @param          Debug
        *                    是否开启Debug级别的log，log路径为可执行文件目录下sinboha.log。
        *    @return         SinbohaError
        *****************************************************************************************************/
         virtual SinbohaError Initialize(
             const std::string& PeerPrimaryAddress,
             const std::string& PeerSecondaryAddress,
             int PeerPort,
             int Port,
             std::chrono::milliseconds NetworkTimeout,
             std::chrono::milliseconds Heartbeat,
             std::chrono::milliseconds SwitchTimeout,
             bool Debug = false) = 0;

        /*************************************************************************************************//**
        *    HA接口释放方法
        *
        *    @return         SinbohaError
        *****************************************************************************************************/
         virtual SinbohaError Release() = 0;

        /*************************************************************************************************//**
        *    注册回调方法
        *
        *    @param          Callback
        *                    回调对象指针。
        *****************************************************************************************************/
         virtual void RegisterCallback(std::shared_ptr<SinbohaCallbackIf> Callback) = 0;

        /*************************************************************************************************//**
        *    反注册回调方法
        *****************************************************************************************************/
         virtual void UnRegisterCallback() = 0;

        /*************************************************************************************************//**
        *    同步数据方法
        *
        *    @param          Data
        *                    字符串对象，可以用来同步二进制数据。
        *    @return         SinbohaError
        *****************************************************************************************************/
         virtual SinbohaError SyncData(const std::string& Data) = 0;

        /*************************************************************************************************//**
        *    主从切换方法
        *
        *    @return         SinbohaError
        *    @note           该方法只能在激活端调用。
        *****************************************************************************************************/
         virtual SinbohaError Switch() = 0;

        /*************************************************************************************************//**
        *    获取当前状态方法
        *
        *    @return         当前状态
        *****************************************************************************************************/
         virtual SinbohaStatus GetHaStatus() = 0;
    };
}

/*************************************************************************************************//**
*    动态连接库导出方法
*
*    @return         返回接口指针，无需释放。
*****************************************************************************************************/
SINBOHA_DLL SINBOHA_NSP::SinbohaIf* GetSinbohaIf();
