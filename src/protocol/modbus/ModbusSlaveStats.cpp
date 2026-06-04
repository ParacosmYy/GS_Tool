/**
 * @file ModbusSlaveStats.cpp
 * @brief Modbus从站 — 统计查询方法与重置
 *
 * 从 ModbusSlaveResponse.cpp 拆分而来，包含所有统计计数器的
 * 查询方法和重置操作:
 *   - requestCount(): 已处理请求总数
 *   - exceptionCount(): 异常响应计数
 *   - totalRequestsHandled(): 已成功处理请求数
 *   - totalResponsesSent(): 已发送响应总数
 *   - totalSlaveErrors(): 从站内部错误数
 *   - totalExceptionResponses(): 异常响应发送总数
 *   - functionCodeStats(): 各功能码调用次数
 *   - resetStatistics(): 重置所有计数器
 */

#include "protocol/modbus/ModbusSlave.h"

// ============================================================================
// 统计查询方法
// ============================================================================

/** @brief 获取已处理请求总数 @return 请求数 */
quint64 ModbusSlave::requestCount() const
{
    return m_requestCount;
}

/** @brief 获取异常响应计数 @return 异常数 */
quint64 ModbusSlave::exceptionCount() const
{
    return m_exceptionCount;
}

/** @brief 获取已成功处理的请求总数 @return 已处理请求数 */
quint64 ModbusSlave::totalRequestsHandled() const
{
    return m_totalRequestsHandled;
}

/** @brief 获取已发送的响应帧总数 @return 响应发送总数 */
quint64 ModbusSlave::totalResponsesSent() const
{
    return m_totalResponsesSent;
}

/** @brief 获取从站内部错误次数 @return 内部错误计数 */
quint64 ModbusSlave::totalSlaveErrors() const
{
    return m_totalSlaveErrors;
}

/** @brief 获取异常响应发送总数 @return 异常响应计数 */
quint64 ModbusSlave::totalExceptionResponses() const
{
    return m_totalExceptionResponses;
}

/** @brief 获取CRC校验失败次数 @return CRC错误计数 */
quint64 ModbusSlave::totalCrcErrors() const
{
    return m_totalCrcErrors;
}

/** @brief 获取不支持功能码被调用的次数 @return 不支持功能码计数 */
quint64 ModbusSlave::totalUnsupportedFunctions() const
{
    return m_totalUnsupportedFunctions;
}

/** @brief 获取各功能码调用次数统计 @return 功能码→调用次数映射 */
QMap<int, int> ModbusSlave::functionCodeStats() const
{
    return m_fcStats;
}

/** @brief 重置所有统计计数器(请求数/异常数/功能码统计/已处理/已发送/内部错误/异常响应/CRC错误/不支持功能码) */
void ModbusSlave::resetStatistics()
{
    m_requestCount = 0;
    m_exceptionCount = 0;
    m_totalRequestsHandled = 0;
    m_totalResponsesSent = 0;
    m_totalSlaveErrors = 0;
    m_totalExceptionResponses = 0;
    m_totalCrcErrors = 0;
    m_totalUnsupportedFunctions = 0;
    m_fcStats.clear();
}
