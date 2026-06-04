/**
 * @file SendControllerStats.cpp
 * @brief 发送控制器 - 统计计数器接口实现
 *
 * 包含发送操作的统计查询方法（发送次数、字节数、HEX 模式计数、
 * 错误次数、宏执行次数）以及统计重置方法。
 * 从 SendController.cpp 拆分而来，保持主文件聚焦于发送流程。
 */

#include "core/send/SendController.h"

// ---- 统计计数器接口 ----

/** @brief 获取累计发送操作总次数 @return 发送次数 */
quint64 SendController::totalSends() const
{
    return m_totalSends;
}

/** @brief 获取累计发送的字节总数 @return 字节数 */
quint64 SendController::totalBytesSent() const
{
    return m_totalBytesSent;
}

/** @brief 获取累计HEX模式发送次数 @return HEX发送次数 */
quint64 SendController::totalHexSends() const
{
    return m_totalHexSends;
}

/** @brief 获取累计发送错误次数 @return 错误次数 */
quint64 SendController::totalErrors() const
{
    return m_totalErrors;
}

/** @brief 获取累计快捷指令(宏)执行次数 @return 宏执行次数 */
quint64 SendController::totalMacroExecutions() const
{
    return m_totalMacroExecutions;
}

/** @brief 重置所有统计计数器(发送/字节/HEX/错误/宏) */
void SendController::resetSendStatistics()
{
    m_totalSends = 0;
    m_totalBytesSent = 0;
    m_totalHexSends = 0;
    m_totalErrors = 0;
    m_totalMacroExecutions = 0;
}
