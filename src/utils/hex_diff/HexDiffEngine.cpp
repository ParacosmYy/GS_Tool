/**
 * @file HexDiffEngine.cpp
 * @brief 十六进制差异引擎 — 核心比较与统计实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 包含构造/统计、逐字节比较、差异块合并、私有工具方法。
 * 并排 hex dump / 统一差异格式见 HexDiffEngineDiff.cpp。
 * 三路比较 / 补丁操作见 HexDiffEnginePatch.cpp。
 */

#include "utils/hex_diff/HexDiffEngine.h"

#include <algorithm>

/* ═══════════════════════════════════════════════════════════════
 *  构造 / 统计
 * ═══════════════════════════════════════════════════════════════ */

/** @brief 构造函数 @param parent 父对象 */
HexDiffEngine::HexDiffEngine(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("HexDiffEngine"));
}

/** @brief 获取统计快照 @return 统计结构体副本 */
HexDiffEngine::Stats HexDiffEngine::stats() const
{
    return m_stats;
}

/** @brief 重置所有累计统计计数器为初始值 */
void HexDiffEngine::resetStatistics()
{
    m_stats = Stats{};
    m_similaritySum = 0.0;
}

/* ═══════════════════════════════════════════════════════════════
 *  核心比较
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 逐字节比较两个 QByteArray
 * @param dataA 数据缓冲区 A
 * @param dataB 数据缓冲区 B
 * @return 逐字节差异条目列表
 *
 * 对公共长度段逐字节比较，超出公共长度的部分标记为插入或删除。
 * 完成后更新统计并发射 comparisonComplete 和 diffBlockFound 信号。
 */
QList<HexDiffEngine::DiffEntry> HexDiffEngine::compare(const QByteArray &dataA,
                                                        const QByteArray &dataB)
{
    QList<DiffEntry> entries;
    const int lenA = dataA.size();
    const int lenB = dataB.size();
    const int commonLen = std::min(lenA, lenB);
    const quint64 totalBytes = static_cast<quint64>(std::max(lenA, lenB));

    entries.reserve(std::max(lenA, lenB));

    /* 公共段：逐字节比较 */
    quint64 matchCount = 0;
    quint64 mismatchCount = 0;

    for (int i = 0; i < commonLen; ++i) {
        DiffEntry entry;
        entry.offset = static_cast<quint64>(i);
        entry.byteA = static_cast<quint8>(dataA[i]);
        entry.byteB = static_cast<quint8>(dataB[i]);

        if (entry.byteA == entry.byteB) {
            entry.kind = DiffKind::Match;
            entry.color = ColorCode::Normal;
            ++matchCount;
        } else {
            entry.kind = DiffKind::Mismatch;
            entry.color = ColorCode::Red;
            ++mismatchCount;
        }
        entries.append(entry);
    }

    /* A 比 B 长 → 尾部为删除 */
    quint64 deleteCount = 0;
    for (int i = commonLen; i < lenA; ++i) {
        DiffEntry entry;
        entry.offset = static_cast<quint64>(i);
        entry.byteA = static_cast<quint8>(dataA[i]);
        entry.byteB = 0;
        entry.kind = DiffKind::Deleted;
        entry.color = ColorCode::Blue;
        entries.append(entry);
        ++deleteCount;
    }

    /* B 比 A 长 → 尾部为插入 */
    quint64 insertCount = 0;
    for (int i = commonLen; i < lenB; ++i) {
        DiffEntry entry;
        entry.offset = static_cast<quint64>(i);
        entry.byteA = 0;
        entry.byteB = static_cast<quint8>(dataB[i]);
        entry.kind = DiffKind::Inserted;
        entry.color = ColorCode::Green;
        entries.append(entry);
        ++insertCount;
    }

    /* 计算相似度 */
    const double sim = (totalBytes > 0)
                           ? (static_cast<double>(matchCount) * 100.0 / static_cast<double>(totalBytes))
                           : 100.0;

    /* 更新累计统计 */
    ++m_stats.totalComparisons;
    m_stats.totalBytesCompared += totalBytes;
    m_stats.totalMatches += matchCount;
    m_stats.totalMismatches += mismatchCount;
    m_stats.totalInsertions += insertCount;
    m_stats.totalDeletions += deleteCount;
    m_similaritySum += sim;
    m_stats.avgSimilarity = m_similaritySum / static_cast<double>(m_stats.totalComparisons);

    /* 发射信号 */
    emit comparisonComplete(sim);

    /* 合并为差异块并逐个发射 */
    const auto blocks = compareBlocks(entries);
    for (const auto &block : blocks) {
        if (block.kind != DiffKind::Match) {
            emit diffBlockFound(block.startOffset, block.length);
        }
    }

    return entries;
}

/**
 * @brief 将差异条目合并为连续差异块
 * @param entries 逐字节差异条目列表
 * @return 连续差异块列表
 *
 * 将相邻的相同 DiffKind 且偏移连续的条目合并为一个 DiffBlock。
 */
QList<HexDiffEngine::DiffBlock> HexDiffEngine::compareBlocks(const QList<DiffEntry> &entries) const
{
    QList<DiffBlock> blocks;
    if (entries.isEmpty()) {
        return blocks;
    }

    DiffBlock current;
    current.startOffset = entries.first().offset;
    current.kind = entries.first().kind;
    current.length = 1;

    /* 根据类型填充 dataA/dataB */
    if (entries.first().kind != DiffKind::Inserted) {
        current.dataA.append(static_cast<char>(entries.first().byteA));
    }
    if (entries.first().kind != DiffKind::Deleted) {
        current.dataB.append(static_cast<char>(entries.first().byteB));
    }

    for (int i = 1; i < entries.size(); ++i) {
        const auto &entry = entries[i];

        /* 同类型且偏移连续 → 合并 */
        if (entry.kind == current.kind
            && entry.offset == current.startOffset + static_cast<quint64>(current.length)) {
            ++current.length;
            if (entry.kind != DiffKind::Inserted) {
                current.dataA.append(static_cast<char>(entry.byteA));
            }
            if (entry.kind != DiffKind::Deleted) {
                current.dataB.append(static_cast<char>(entry.byteB));
            }
        } else {
            /* 类型变化或偏移不连续 → 保存当前块，开启新块 */
            blocks.append(current);
            current = DiffBlock{};
            current.startOffset = entry.offset;
            current.kind = entry.kind;
            current.length = 1;
            if (entry.kind != DiffKind::Inserted) {
                current.dataA.append(static_cast<char>(entry.byteA));
            }
            if (entry.kind != DiffKind::Deleted) {
                current.dataB.append(static_cast<char>(entry.byteB));
            }
        }
    }

    blocks.append(current);
    return blocks;
}

/* ═══════════════════════════════════════════════════════════════
 *  私有工具方法
 * ═══════════════════════════════════════════════════════════════ */

/** @brief 格式化单字节为两位大写十六进制 */
QString HexDiffEngine::formatHexByte(quint8 byte)
{
    return QStringLiteral("%1").arg(byte, 2, 16, QChar('0')).toUpper();
}

/** @brief 格式化偏移地址为8位大写十六进制 */
QString HexDiffEngine::formatOffset(quint64 offset)
{
    return QStringLiteral("%1").arg(offset, 8, 16, QChar('0')).toUpper();
}
