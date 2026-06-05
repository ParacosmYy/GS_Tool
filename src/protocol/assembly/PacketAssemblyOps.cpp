/**
 * @file PacketAssemblyOps.cpp
 * @brief 数据包组装器 -- 包完成/超时/缓冲区/查询/统计方法
 *
 * 从 PacketAssembler.cpp 拆分: emitCompletedPacket/dropPacket/checkTimeouts/
 * checkBufferLimits/evictOldestPacket/getCompletePackets/getIncompletePackets/
 * reset/isAssembling/activePacketCount/updateStats/stats/resetStatistics。
 */

#include "protocol/assembly/PacketAssembler.h"

#include <QDateTime>
#include <algorithm>

// ============================================================================
// 包完成与发射
// ============================================================================

/**
 * @brief 组装并发射完成的包
 * @param ctx 组装上下文(将被移动)
 *
 * 按序列号升序拼接所有分片数据，发射 packetAssembled 信号，
 * 更新统计，并从活跃包列表中移除。
 */
void PacketAssembler::emitCompletedPacket(AssemblyContext& ctx)
{
    QByteArray assembled;
    assembled.reserve(ctx.bufferSize);

    QList<int> sortedKeys = ctx.fragments.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end());

    for (int key : sortedKeys) {
        assembled.append(ctx.fragments.value(key));
    }

    qint64 elapsedMs = ctx.assemblyTimer.elapsed();
    m_completedPackets.append(assembled);

    ++m_stats.totalPacketsAssembled;
    m_sumAssemblyTimeMs += static_cast<quint64>(elapsedMs);
    m_activePackets.remove(ctx.packetId);

    emit packetAssembled(assembled);
}

/**
 * @brief 清理指定包的缓冲区
 * @param packetId 包标识
 * @param emitSignal 是否发射超时信号
 */
void PacketAssembler::dropPacket(int packetId, bool emitSignal)
{
    auto it = m_activePackets.find(packetId);
    if (it == m_activePackets.end()) {
        return;
    }

    if (emitSignal) {
        emit assemblyTimeout(packetId);
    }

    m_activePackets.erase(it);
}

// ============================================================================
// 超时检测
// ============================================================================

/**
 * @brief 扫描所有未完成包并清理超时包
 *
 * 遍历所有活跃包，若最近分片时间距当前时间超过 fragmentTimeoutMs，
 * 则丢弃该包并发射超时信号。
 */
void PacketAssembler::checkTimeouts()
{
    if (m_config.fragmentTimeoutMs <= 0) {
        return;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    QList<int> timeoutIds;

    for (auto it = m_activePackets.constBegin();
         it != m_activePackets.constEnd(); ++it) {
        qint64 elapsed = now - it.value().lastFragmentTime;
        if (elapsed > m_config.fragmentTimeoutMs) {
            timeoutIds.append(it.key());
        }
    }

    for (int id : timeoutIds) {
        ++m_stats.totalTimeouts;
        dropPacket(id, true);
    }

    if (m_activePackets.isEmpty()) {
        m_timeoutTimer->stop();
    }
}

// ============================================================================
// 缓冲区管理
// ============================================================================

/**
 * @brief 检查缓冲区是否允许追加指定大小的数据
 * @param additionalBytes 额外需要的字节数
 * @return true=允许追加, false=超出限制
 */
bool PacketAssembler::checkBufferLimits(int additionalBytes)
{
    int totalUsed = 0;
    for (const auto& ctx : m_activePackets) {
        totalUsed += ctx.bufferSize;
    }
    return (totalUsed + additionalBytes) <= m_config.maxBufferSize;
}

/** @brief 淘汰最旧的未完成包 */
void PacketAssembler::evictOldestPacket()
{
    if (m_activePackets.isEmpty()) {
        return;
    }

    int oldestId = -1;
    qint64 oldestTime = LLONG_MAX;
    for (auto it = m_activePackets.constBegin();
         it != m_activePackets.constEnd(); ++it) {
        if (it.value().firstFragmentTime < oldestTime) {
            oldestTime = it.value().firstFragmentTime;
            oldestId = it.key();
        }
    }

    if (oldestId >= 0) {
        ++m_stats.totalTimeouts;
        dropPacket(oldestId, true);
    }
}

// ============================================================================
// 结果查询
// ============================================================================

/** @brief 取出所有已组装完成的数据包 @return 完整数据包列表(取后清空) */
QList<QByteArray> PacketAssembler::getCompletePackets()
{
    QList<QByteArray> result = std::move(m_completedPackets);
    m_completedPackets.clear();
    return result;
}

/** @brief 获取当前未完成包的进度列表 */
QList<PacketAssembler::IncompletePacket> PacketAssembler::getIncompletePackets() const
{
    QList<IncompletePacket> result;
    for (const auto& ctx : m_activePackets) {
        IncompletePacket info;
        info.packetId = ctx.packetId;
        info.totalFragments = ctx.totalFragments;
        info.receivedFragments = ctx.receivedSeqs.size();
        info.firstFragmentTime = ctx.firstFragmentTime;
        info.lastFragmentTime = ctx.lastFragmentTime;
        info.bufferSize = ctx.bufferSize;
        result.append(info);
    }
    return result;
}

// ============================================================================
// 状态控制
// ============================================================================

/** @brief 重置组装状态(清空所有缓冲区和完成队列，不清零统计) */
void PacketAssembler::reset()
{
    m_activePackets.clear();
    m_completedPackets.clear();
    m_timestampWindowStart = 0;
    m_nextPacketId = 0;

    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }
}

bool PacketAssembler::isAssembling() const
{
    return !m_activePackets.isEmpty();
}

int PacketAssembler::activePacketCount() const
{
    return m_activePackets.size();
}

// ============================================================================
// 统计
// ============================================================================

/** @brief 更新派生统计值(平均耗时、缓冲区利用率) */
void PacketAssembler::updateStats()
{
    if (m_stats.totalPacketsAssembled > 0) {
        m_stats.avgAssemblyTimeMs =
            static_cast<double>(m_sumAssemblyTimeMs) /
            static_cast<double>(m_stats.totalPacketsAssembled);
    }

    int totalUsed = 0;
    for (const auto& ctx : m_activePackets) {
        totalUsed += ctx.bufferSize;
    }
    m_stats.bufferUtilization =
        (m_config.maxBufferSize > 0)
            ? static_cast<double>(totalUsed) / static_cast<double>(m_config.maxBufferSize)
            : 0.0;
}

const PacketAssembler::Stats& PacketAssembler::stats() const
{
    return m_stats;
}

void PacketAssembler::resetStatistics()
{
    m_stats = Stats{};
    m_sumAssemblyTimeMs = 0;
}
