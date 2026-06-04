/**
 * @file ConnectionControllerQuery.cpp
 * @brief 连接控制器查询与统计方法 - 连接状态访问器、信号线控制和统计计数器
 *
 * 从 ConnectionController.cpp 拆分而来，包含:
 *   - currentConnection(): 获取当前活跃连接指针
 *   - setDtr() / setRts() / sendBreak(): 信号线控制(委托给IConnection)
 *   - portWatcher(): 获取端口热插拔监控器
 *   - totalConnections() / totalDisconnections() / totalReconnects(): 连接统计查询
 *   - errorCount() / totalDataSent() / totalDataReceived(): 数据统计查询
 *   - resetConnectionStatistics(): 重置所有统计计数器
 */

#include "core/connect/ConnectionController.h"

#include "serial/port/PortWatcher.h"

/** @brief 返回当前活动连接指针 @return IConnection指针，无连接时为nullptr */
IConnection* ConnectionController::currentConnection() const { return m_currentConn; }

/** @brief 设置DTR信号电平 @param enabled true=高电平 */
void ConnectionController::setDtr(bool enabled) { if (m_currentConn) m_currentConn->setDtr(enabled); }

/** @brief 设置RTS信号电平 @param enabled true=高电平 */
void ConnectionController::setRts(bool enabled) { if (m_currentConn) m_currentConn->setRts(enabled); }

/** @brief 发送Break信号(用于STM32/ESP32进入Bootloader) @param duration Break持续时间(毫秒) */
void ConnectionController::sendBreak(int duration) { if (m_currentConn) m_currentConn->sendBreak(duration); }

/** @brief 返回端口监听器 @return PortWatcher指针 */
PortWatcher* ConnectionController::portWatcher() const { return m_portWatcher; }

/** @brief 获取累计成功连接次数 @return 连接总次数 */
quint64 ConnectionController::totalConnections() const { return m_totalConnections; }

/** @brief 获取累计断开连接次数 @return 断开总次数 */
quint64 ConnectionController::totalDisconnections() const { return m_totalDisconnections; }

/** @brief 获取累计自动重连次数 @return 重连总次数 */
quint64 ConnectionController::totalReconnects() const { return m_totalReconnects; }

/** @brief 获取累计连接错误次数 @return 错误总次数 */
quint64 ConnectionController::errorCount() const { return m_errorCount; }

/** @brief 获取累计发送数据字节数 @return 发送总字节数 */
quint64 ConnectionController::totalDataSent() const { return m_totalDataSent; }

/** @brief 获取累计接收数据字节数 @return 接收总字节数 */
quint64 ConnectionController::totalDataReceived() const { return m_totalDataReceived; }

/** @brief 重置连接统计计数器(连接/断开/重连/错误/发送字节/接收字节)为初始值 */
void ConnectionController::resetConnectionStatistics()
{
    m_totalConnections = 0;
    m_totalDisconnections = 0;
    m_totalReconnects = 0;
    m_errorCount = 0;
    m_totalDataSent = 0;
    m_totalDataReceived = 0;
}
