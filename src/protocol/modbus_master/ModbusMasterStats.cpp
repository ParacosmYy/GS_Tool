/**
 * @file ModbusMasterStats.cpp
 * @brief ModbusMaster轮询调度器统计接口实现
 */
#include "protocol/modbus_master/ModbusMaster.h"

quint64 ModbusMasterPoller::totalRequests() const  { return m_totalRequests; }
quint64 ModbusMasterPoller::totalResponses() const { return m_totalResponses; }
quint64 ModbusMasterPoller::totalTimeouts() const  { return m_totalTimeouts; }
quint64 ModbusMasterPoller::totalErrors() const    { return m_totalErrors; }
quint64 ModbusMasterPoller::totalRetries() const   { return m_totalRetries; }
quint64 ModbusMasterPoller::bytesSent() const      { return m_bytesSent; }
quint64 ModbusMasterPoller::bytesReceived() const  { return m_bytesReceived; }

/** @brief 重置所有统计计数器 */
void ModbusMasterPoller::resetStatistics() {
    m_totalRequests = 0;  m_totalResponses = 0;
    m_totalTimeouts = 0;  m_totalErrors = 0;
    m_totalRetries = 0;   m_bytesSent = 0;
    m_bytesReceived = 0;
}
