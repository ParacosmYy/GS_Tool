/**
 * @file SvdParserStats.cpp
 * @brief CMSIS SVD 解析器 - 统计接口实现
 *
 * 统计 getter 和 resetStatistics() 的独立实现文件，
 * 保持 SvdParser.cpp 主解析逻辑在 500 行以内。
 */

#include "protocol/svd/SvdParser.h"

// ──────────────────────── 统计 Getter ────────────────────────

/** @brief 累计解析外设总数 */
quint64 SvdParser::totalPeripherals() const
{
    return m_stats.totalPeripherals;
}

/** @brief 累计解析寄存器总数 */
quint64 SvdParser::totalRegisters() const
{
    return m_stats.totalRegisters;
}

/** @brief 累计解析字段总数 */
quint64 SvdParser::totalFields() const
{
    return m_stats.totalFields;
}

/** @brief 累计解析错误总数 */
quint64 SvdParser::totalParseErrors() const
{
    return m_stats.totalParseErrors;
}

/** @brief 重置所有统计计数器为零 */
void SvdParser::resetStatistics()
{
    m_stats = SvdParseStatistics{};
}
