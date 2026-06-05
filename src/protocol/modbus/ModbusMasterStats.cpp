/**
 * @file ModbusMasterStats.cpp
 * @brief Modbus主站统计接口与兼容旧接口实现
 *
 * 包含所有统计计数器查询方法、快照获取、重置方法。
 * 按功能码分类的读写成功率追踪、字节级统计。
 */
#include "protocol/modbus/ModbusMaster.h"

// ============================================================================
// 统计接口
// ============================================================================

/** @brief 获取累计发送请求总数 @return 请求数 */
quint64 ModbusMaster::totalRequests() const { return m_totalRequests; }

/** @brief 获取累计接收有效响应总数 @return 响应数 */
quint64 ModbusMaster::totalResponses() const { return m_totalResponses; }

/** @brief 获取累计超时次数 @return 超时次数 */
quint64 ModbusMaster::totalTimeouts() const { return m_totalTimeouts; }

/** @brief 获取累计Modbus异常响应总数(兼容旧接口) @return 错误数 */
quint64 ModbusMaster::totalErrors() const { return m_totalErrors; }

/** @brief 获取累计CRC校验失败次数 @return CRC错误数 */
quint64 ModbusMaster::totalCrcErrors() const { return m_totalCrcErrors; }

/** @brief 获取累计Modbus异常响应次数 @return 异常数 */
quint64 ModbusMaster::totalExceptions() const { return m_totalExceptions; }

/** @brief 获取累计重试次数 @return 重试数 */
quint64 ModbusMaster::totalRetries() const { return m_totalRetries; }

/** @brief 获取累计读操作成功次数 @return 成功读次数 */
quint64 ModbusMaster::successfulReads() const { return m_successfulReads; }

/** @brief 获取累计读操作失败次数 @return 失败读次数 */
quint64 ModbusMaster::failedReads() const { return m_failedReads; }

/** @brief 获取累计写操作成功次数 @return 成功写次数 */
quint64 ModbusMaster::successfulWrites() const { return m_successfulWrites; }

/** @brief 获取累计写操作失败次数 @return 失败写次数 */
quint64 ModbusMaster::failedWrites() const { return m_failedWrites; }

/** @brief 获取累计发送字节数 @return 字节数 */
quint64 ModbusMaster::bytesTransmitted() const { return m_bytesTransmitted; }

/** @brief 获取累计接收字节数 @return 字节数 */
quint64 ModbusMaster::bytesReceived() const { return m_bytesReceived; }

/** @brief 获取指定功能码的调用次数 @param fc 功能码 @return 调用次数 */
quint64 ModbusMaster::functionCodeCount(int fc) const {
    return m_fcStats.value(fc, 0);
}

/**
 * @brief 获取统计快照 -- 将所有计数器打包到结构体中
 * @return ModbusMasterStats结构体
 */
ModbusMasterStats ModbusMaster::stats() const {
    ModbusMasterStats s;
    s.totalRequests    = m_totalRequests;
    s.totalResponses   = m_totalResponses;
    s.totalTimeouts    = m_totalTimeouts;
    s.totalCrcErrors   = m_totalCrcErrors;
    s.totalExceptions  = m_totalExceptions;
    s.bytesTransmitted = m_bytesTransmitted;
    s.bytesReceived    = m_bytesReceived;
    /* 按功能码填充统计数组 */
    s.requestsByFunctionCode[0] = m_fcStats.value(0x01, 0);
    s.requestsByFunctionCode[1] = m_fcStats.value(0x02, 0);
    s.requestsByFunctionCode[2] = m_fcStats.value(0x03, 0);
    s.requestsByFunctionCode[3] = m_fcStats.value(0x04, 0);
    s.requestsByFunctionCode[4] = m_fcStats.value(0x05, 0);
    s.requestsByFunctionCode[5] = m_fcStats.value(0x06, 0);
    s.requestsByFunctionCode[6] = m_fcStats.value(0x0F, 0);
    s.requestsByFunctionCode[7] = m_fcStats.value(0x10, 0);
    return s;
}

/** @brief 重置所有统计计数器 */
void ModbusMaster::resetStatistics() {
    m_totalRequests    = 0;
    m_totalResponses   = 0;
    m_totalTimeouts    = 0;
    m_totalCrcErrors   = 0;
    m_totalErrors      = 0;
    m_totalExceptions  = 0;
    m_totalRetries     = 0;
    m_successfulReads  = 0;
    m_failedReads      = 0;
    m_successfulWrites = 0;
    m_failedWrites     = 0;
    m_bytesTransmitted = 0;
    m_bytesReceived    = 0;
    m_fcStats.clear();
}

/** @brief 兼容旧接口: resetStats -> resetStatistics */
void ModbusMaster::resetStats() { resetStatistics(); }
