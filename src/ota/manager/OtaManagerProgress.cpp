/**
 * @file OtaManagerProgress.cpp
 * @brief OTA升级管理器 - 进度回调、传输统计与工具方法
 *
 * 本文件从 OtaManager.cpp 拆分而来，包含:
 *   - 协议显示名称映射 (protocolDisplayName)
 *   - 传输统计查询接口 (totalTransfers / successfulTransfers / ...)
 *   - 统计重置 (resetTransferStatistics)
 *
 * 拆分原因: OtaManager.cpp 体积接近 500 行限制，将统计与辅助方法
 * 独立出来以保持主文件聚焦于 OTA 生命周期管理。
 *
 * @see OtaManager.cpp — 构造/析构/信号连接/状态管理/文件验证/传输控制
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

/** @brief 重置传输统计计数器(不影响transferCount和lastTransferSuccess等历史记录) */
void OtaManager::resetTransferStatistics()
{
    m_totalTransfers = 0;
    m_successfulTransfers = 0;
    m_failedTransfers = 0;
    m_totalBytesTransferred = 0;
}
