/**
 * @file ModbusMasterStats.cpp
 * @brief Modbus主站统计接口与兼容旧接口实现
 *
 * 包含所有统计计数器查询方法、重置方法、兼容旧接口方法。
 * 按功能码分类的读写成功率追踪、异常/超时/重试计数查询。
 */
#include "protocol/modbus/ModbusMaster.h"

// ============================================================================
// 统计接口
// ============================================================================

/** @brief 获取累计发送的请求总数 @return 请求数 */
quint64 ModbusMaster::totalRequests() const { return m_totalRequests; }

/** @brief 获取累计接收的有效响应总数 @return 响应数 */
quint64 ModbusMaster::totalResponses() const { return m_totalResponses; }

/** @brief 获取累计超时次数 @return 超时次数 */
quint64 ModbusMaster::totalTimeouts() const { return m_totalTimeouts; }

/** @brief 获取累计Modbus异常响应总数 @return 错误数 */
quint64 ModbusMaster::totalErrors() const { return m_totalErrors; }

/** @brief 获取累计Modbus异常响应次数（功能码最高位置1的响应） @return 异常响应数 */
quint64 ModbusMaster::totalExceptions() const { return m_totalExceptions; }

/** @brief 获取累计重试发送次数 @return 重试次数 */
quint64 ModbusMaster::totalRetries() const { return m_totalRetries; }

/** @brief 获取累计读操作成功次数 @return 成功读次数 */
quint64 ModbusMaster::successfulReads() const { return m_successfulReads; }

/** @brief 获取累计读操作失败次数 @return 失败读次数 */
quint64 ModbusMaster::failedReads() const { return m_failedReads; }

/** @brief 获取累计写操作成功次数 @return 成功写次数 */
quint64 ModbusMaster::successfulWrites() const { return m_successfulWrites; }

/** @brief 获取累计写操作失败次数 @return 失败写次数 */
quint64 ModbusMaster::failedWrites() const { return m_failedWrites; }

/** @brief 获取指定功能码的调用次数 @param fc 功能码 @return 调用次数 */
quint64 ModbusMaster::functionCodeCount(int fc) const {
    return m_fcStats.value(fc, 0);
}

/** @brief 重置所有统计计数器（将所有累计值归零并清空功能码统计映射） */
void ModbusMaster::resetStats() {
    m_totalRequests    = 0;
    m_totalResponses   = 0;
    m_totalTimeouts    = 0;
    m_totalErrors      = 0;
    m_totalExceptions  = 0;
    m_totalRetries     = 0;
    m_successfulReads  = 0;
    m_failedReads      = 0;
    m_successfulWrites = 0;
    m_failedWrites     = 0;
    m_fcStats.clear();
}

// ============================================================================
// 兼容旧接口
// ============================================================================

/** @brief 兼容旧接口: readRegisters默认调用FC03 */
bool ModbusMaster::readRegisters(int slave, int start, int count) {
    return readHoldingRegisters(slave, start, count);
}

/** @brief 兼容旧接口: requestCount -> totalRequests */
quint64 ModbusMaster::requestCount() const { return m_totalRequests; }

/** @brief 兼容旧接口: responseCount -> totalResponses */
quint64 ModbusMaster::responseCount() const { return m_totalResponses; }

/** @brief 兼容旧接口: timeoutCount -> totalTimeouts */
quint64 ModbusMaster::timeoutCount() const { return m_totalTimeouts; }

/** @brief 兼容旧接口: errorCount -> totalErrors */
quint64 ModbusMaster::errorCount() const { return m_totalErrors; }

/** @brief 兼容旧接口: resetStatistics -> resetStats */
void ModbusMaster::resetStatistics() { resetStats(); }
