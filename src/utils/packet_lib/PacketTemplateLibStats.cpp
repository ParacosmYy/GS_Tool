/**
 * @file PacketTemplateLibStats.cpp
 * @brief 报文模板库统计接口 — 重置计数器
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/packet_lib/PacketTemplateLib.h"

/**
 * @brief 重置所有统计计数器
 *
 * 将模板加载数、发送次数、发送字节数、编辑次数、
 * 单分类峰值全部归零。
 */
void PacketTemplateLib::resetStatistics()
{
    m_stats.totalTemplatesLoaded = 0;
    m_stats.totalSends = 0;
    m_stats.totalBytesSent = 0;
    m_stats.totalEdits = 0;
    m_stats.peakTemplatesPerCategory = 0;
}
