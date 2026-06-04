/**
 * @file BluetoothAtConsoleStats.cpp
 * @brief 蓝牙AT指令控制台 — 统计计数器查询与重置实现
 *
 * 从 BluetoothAtConsole.cpp 拆分而来，包含AT命令发送/
 * 响应接收的统计 getter 和 resetStatistics 方法。
 */

#include "connection/ble/BluetoothAtConsole.h"

/** @brief 获取累计发送AT命令次数 */
quint64 BluetoothAtConsole::totalCommandsSent() const
{
    return m_totalCommandsSent;
}

/** @brief 获取累计接收响应次数 */
quint64 BluetoothAtConsole::totalResponsesReceived() const
{
    return m_totalResponsesReceived;
}

/** @brief 重置所有统计计数器 */
void BluetoothAtConsole::resetStatistics()
{
    m_totalCommandsSent = 0;
    m_totalResponsesReceived = 0;
    m_totalSendErrors = 0;
    m_totalBytesSent = 0;
}
