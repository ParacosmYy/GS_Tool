/**
 * @file OtaManagerProgress.cpp
 * @brief OTA升级管理器 - 进度回调、传输统计、平均速率与工具方法
 *
 * 本文件从 OtaManager.cpp 拆分而来，包含:
 *   - 协议显示名称映射 (protocolDisplayName)
 *   - 传输统计查询接口 (totalTransfers / successfulTransfers / ...)
 *   - 平均速率计算 (averageSpeed)
 *   - 统计重置 (resetTransferStatistics)
 *
 * 拆分原因: OtaManager.cpp 体积接近 500 行限制，将统计与辅助方法
 * 独立出来以保持主文件聚焦于 OTA 生命周期管理。
 *
 * @see OtaManager.cpp — 构造/析构/信号连接/状态管理/文件验证/传输控制/校验和验证
 */

#include "ota/manager/OtaManager.h"

// ============================================================================
// 工具方法
// ============================================================================

/** @brief 获取协议的可读显示名称 @param protocol 协议标识字符串 @return 人类可读的协议名称 */
QString OtaManager::protocolDisplayName(const QString& protocol) const
{
    if (protocol == "xmodem-crc")      return tr("XMODEM-CRC");
    if (protocol == "xmodem-checksum") return tr("XMODEM-Checksum");
    if (protocol == "xmodem-1k")       return tr("XMODEM-1K");
    if (protocol == "ymodem")          return tr("YMODEM");
    if (protocol == "zmodem")          return tr("ZMODEM");
    return protocol.toUpper();
}

// ============================================================================
// 传输统计
// ============================================================================

/** @brief 获取传输尝试总次数(包含成功和失败) @return 总尝试次数 */
quint64 OtaManager::totalTransfers() const
{
    return m_totalTransfers;
}

/** @brief 获取成功完成的传输次数 @return 成功次数 */
quint64 OtaManager::successfulTransfers() const
{
    return m_successfulTransfers;
}

/** @brief 获取失败的传输次数 @return 失败次数 */
quint64 OtaManager::failedTransfers() const
{
    return m_failedTransfers;
}

/** @brief 获取所有会话累计传输的字节数 @return 累计字节数 */
quint64 OtaManager::totalBytesTransferred() const
{
    return m_totalBytesTransferred;
}

/** @brief 获取累计CRC校验验证次数 @return 校验次数 */
quint64 OtaManager::totalCrcChecks() const
{
    return m_totalCrcChecks;
}

/**
 * @brief 计算历史平均传输速率
 * @return 平均速率(字节/秒)，无历史记录时返回0
 *
 * 基于所有已完成传输的瞬时速率取算术平均值，
 * 用于在UI中显示"历史平均速率"和预估传输时间。
 * 最多保留最近kMaxSpeedHistory(100)条记录。
 */
double OtaManager::averageSpeed() const
{
    if (m_speedHistory.isEmpty()) {
        return 0.0;
    }

    double sum = 0.0;
    for (double speed : m_speedHistory) {
        sum += speed;
    }
    return sum / static_cast<double>(m_speedHistory.size());
}

/** @brief 重置传输统计计数器和速率历史(不影响transferCount和lastTransferSuccess等历史记录) */
void OtaManager::resetTransferStatistics()
{
    m_totalTransfers = 0;
    m_successfulTransfers = 0;
    m_failedTransfers = 0;
    m_totalBytesTransferred = 0;
    m_totalCrcChecks = 0;
    m_speedHistory.clear();
    m_totalCancellations = 0;
    m_totalHexConversions = 0;
    m_totalProtocolSwitches = 0;
}

/** @brief 获取累计传输取消次数 @return 取消次数 */
quint64 OtaManager::totalCancellations() const
{
    return m_totalCancellations;
}

/** @brief 获取累计HEX转BIN次数 @return 转换次数 */
quint64 OtaManager::totalHexConversions() const
{
    return m_totalHexConversions;
}

/** @brief 获取累计协议切换次数 @return 切换次数 */
quint64 OtaManager::totalProtocolSwitches() const
{
    return m_totalProtocolSwitches;
}
