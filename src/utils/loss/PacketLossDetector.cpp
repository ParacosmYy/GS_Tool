/**
 * @file PacketLossDetector.cpp
 * @brief 丢包检测器实现 -- 构造、序列号处理、Gap检测、乱序/重复检测、CSV导出
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 统计查询与重置方法见：@see PacketLossDetectorStats.cpp
 */

#include "utils/loss/PacketLossDetector.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

// ──────────────────────────────────────────────
// 构造与析构
// ──────────────────────────────────────────────

/**
 * @brief 构造丢包检测器，默认 16 位序列号宽度
 * @param parent 父对象
 */
PacketLossDetector::PacketLossDetector(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("PacketLossDetector"));
    setSequenceWidth(SequenceWidth::Width16);
}

/** @brief 析构函数 */
PacketLossDetector::~PacketLossDetector() = default;

// ──────────────────────────────────────────────
// 配置接口
// ──────────────────────────────────────────────

/**
 * @brief 设置序列号位宽
 *
 * 根据位宽计算最大序列号(2^width - 1)，并重置期望序列号。
 * 不清除已有的 Gap 历史和统计信息，以便跨配置保留分析数据。
 *
 * @param width 序列号位宽枚举
 */
void PacketLossDetector::setSequenceWidth(SequenceWidth width)
{
    m_width = width;
    switch (width) {
    case SequenceWidth::Width8:
        m_maxSeqValue = 0xFFULL;
        break;
    case SequenceWidth::Width16:
        m_maxSeqValue = 0xFFFFULL;
        break;
    case SequenceWidth::Width32:
        m_maxSeqValue = 0xFFFFFFFFULL;
        break;
    }
    // 重置追踪状态以适配新位宽
    m_expectedSeq  = 0;
    m_lastReceived = 0;
    m_initialized  = false;
}

// ──────────────────────────────────────────────
// 核心检测逻辑
// ──────────────────────────────────────────────

/**
 * @brief 处理一个收到的序列号，执行丢包/乱序/重复检测
 *
 * 检测流程：
 * 1. 首包初始化：设定期望值为 seqNum + 1，直接返回
 * 2. 重复检测：seqNum == m_lastReceived，发射 duplicateDetected
 * 3. 正常序列：seqNum == m_expectedSeq，连续丢包计数归零
 * 4. Gap 检测：seqNum > m_expectedSeq（含回绕），计算丢失数，发射 gapDetected
 * 5. 乱序检测：seqNum < m_expectedSeq 且非回绕，发射 outOfOrder
 *
 * @param seqNum 收到的原始序列号(0 ~ 2^width-1)
 */
void PacketLossDetector::processPacket(quint64 seqNum)
{
    ++m_stats.totalPacketsReceived;

    // ── 首包初始化 ──
    if (!m_initialized) {
        m_lastReceived = seqNum;
        m_expectedSeq  = (seqNum + 1) & m_maxSeqValue;
        m_initialized  = true;
        updateLossRate();
        return;
    }

    // ── 重复检测：与上一次收到的序列号相同 ──
    if (seqNum == m_lastReceived) {
        ++m_stats.totalDuplicates;
        emit duplicateDetected(seqNum);
        updateLossRate();
        return;
    }

    // ── 正常序列：恰好是期望值 ──
    if (seqNum == m_expectedSeq) {
        m_stats.currentConsecutiveLoss = 0;
        m_lastReceived = seqNum;
        m_expectedSeq  = (seqNum + 1) & m_maxSeqValue;
        updateLossRate();
        return;
    }

    // ── Gap 检测：收到的序列号超前于期望值 ──
    // 分两种情况：正常超前 或 回绕后超前
    const bool normalAhead = (seqNum > m_expectedSeq);
    const bool rollover    = isRollover(seqNum, m_expectedSeq);

    if (normalAhead || rollover) {
        // 计算 Gap 大小
        quint64 gapSize = 0;
        if (rollover) {
            gapSize = rolloverGapSize(seqNum, m_expectedSeq);
            ++m_stats.sequenceRollovers;
        } else {
            gapSize = seqNum - m_expectedSeq;
        }

        // 构建 Gap 事件
        GapInfo gap;
        gap.expectedSeq = m_expectedSeq;
        gap.receivedSeq = seqNum;
        gap.gapSize     = gapSize;
        gap.timestamp   = QDateTime::currentMSecsSinceEpoch();

        // 更新统计
        ++m_stats.totalGapsDetected;
        m_stats.totalPacketsLost += gapSize;
        m_stats.currentConsecutiveLoss += gapSize;
        if (m_stats.currentConsecutiveLoss > m_stats.maxConsecutiveLoss) {
            m_stats.maxConsecutiveLoss = m_stats.currentConsecutiveLoss;
        }

        // 存储历史(限制最大数量)
        if (m_gaps.size() >= kMaxGapsStored) {
            m_gaps.removeFirst();
        }
        m_gaps.append(gap);

        // 更新追踪状态
        m_lastReceived = seqNum;
        m_expectedSeq  = (seqNum + 1) & m_maxSeqValue;

        emit gapDetected(gap);
        updateLossRate();
        return;
    }

    // ── 乱序检测：收到的序列号落后于期望值，且非回绕 ──
    // seqNum < m_expectedSeq 且不是回绕场景
    ++m_stats.totalOutOfOrder;
    emit outOfOrder(seqNum, m_expectedSeq);
    // 乱序包不更新 expectedSeq，避免丢失中间包的检测机会
    m_lastReceived = seqNum;
    updateLossRate();
}

/**
 * @brief 完全重置检测器
 *
 * 清空所有状态：期望序列号、Gap 历史、统计信息，
 * 恢复到等同于重新构造的状态。
 */
void PacketLossDetector::reset()
{
    m_expectedSeq  = 0;
    m_lastReceived = 0;
    m_initialized  = false;
    m_gaps.clear();

    m_stats = Stats();
}

// ──────────────────────────────────────────────
// 内部辅助方法
// ──────────────────────────────────────────────

/**
 * @brief 判断是否为序列号回绕
 *
 * 判定条件：received < expected 且差值超过位宽空间的一半。
 * 这排除了正常乱序的情况（差值较小时为乱序而非回绕）。
 *
 * 典型示例(8位)：
 * - expected=250, received=5  → 差值245，超过128 → 回绕 ✓
 * - expected=10,  received=8  → 差值2，未超128 → 乱序 ✗
 *
 * @param received 收到的序列号
 * @param expected 期望的序列号
 * @return true 判定为回绕
 */
bool PacketLossDetector::isRollover(quint64 received, quint64 expected) const
{
    if (received >= expected) {
        return false;
    }
    const quint64 diff = expected - received;
    // 差值超过序列号空间的一半则判定为回绕
    return diff > (m_maxSeqValue / 2);
}

/**
 * @brief 计算回绕场景下的 Gap 大小
 *
 * 公式：gapSize = (maxSeq - expected + 1) + received
 *
 * 含义：从 expected 到序列号空间末尾的包数 + 从 0 到 received 的包数。
 *
 * @param received 回绕后收到的序列号
 * @param expected 回绕前的期望序列号
 * @return Gap 大小(丢失包数)
 */
quint64 PacketLossDetector::rolloverGapSize(quint64 received, quint64 expected) const
{
    // 从 expected 到空间末尾(含 expected) + 从 0 到 received(不含 received)
    return (m_maxSeqValue - expected + 1) + received;
}

/**
 * @brief 更新运行时丢包率
 *
 * lossRate = totalPacketsLost / totalPacketsReceived
 * 分母为零时丢包率为 0.0。
 */
void PacketLossDetector::updateLossRate()
{
    if (m_stats.totalPacketsReceived == 0) {
        m_stats.lossRate = 0.0;
    } else {
        m_stats.lossRate = static_cast<double>(m_stats.totalPacketsLost)
                         / static_cast<double>(m_stats.totalPacketsReceived);
    }
}

// ──────────────────────────────────────────────
// CSV 导出
// ──────────────────────────────────────────────

/**
 * @brief 导出 Gap 历史到 CSV 文件
 *
 * CSV 格式：
 * @code
 * Timestamp,Expected_Seq,Received_Seq,Gap_Size
 * 2026-06-05T12:00:00.123,42,45,2
 * @endcode
 *
 * 时间戳使用 ISO 8601 格式(含毫秒)。
 *
 * @param filePath 目标文件路径
 * @return true 写入成功，false 文件打开失败
 */
bool PacketLossDetector::exportReport(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // CSV 表头
    stream << QStringLiteral("Timestamp,Expected_Seq,Received_Seq,Gap_Size\n");

    // 逐行写入 Gap 事件
    for (const GapInfo &gap : m_gaps) {
        const QDateTime dt = QDateTime::fromMSecsSinceEpoch(gap.timestamp);
        stream << dt.toString(Qt::ISODateWithMs)
               << QStringLiteral(",")
               << QString::number(gap.expectedSeq)
               << QStringLiteral(",")
               << QString::number(gap.receivedSeq)
               << QStringLiteral(",")
               << QString::number(gap.gapSize)
               << QStringLiteral("\n");
    }

    file.close();
    return true;
}

// ──────────────────────────────────────────────
// 统计查询 — 见 PacketLossDetectorStats.cpp
// ──────────────────────────────────────────────
