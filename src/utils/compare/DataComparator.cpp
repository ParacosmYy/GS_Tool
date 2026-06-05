/**
 * @file DataComparator.cpp
 * @brief 数据缓冲区字节级比较引擎实现
 * @author Serial Tool Team
 * @date 2026-06-05
 *
 * 包含 LCS 回溯、差异块生成、相似度计算、
 * Hex 并排对比和 CSV 导出的核心实现。
 */

#include "utils/compare/DataComparator.h"

#include <QVector>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
DataComparator::DataComparator(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 构建 LCS 回溯表并提取匹配字节对
 * @param dataA 数据 A
 * @param dataB 数据 B
 * @return 匹配对列表，每对 (idxA, idxB) 表示 A[idxA] == B[idxB]
 *
 * 使用动态规划构建二维 DP 表，时间复杂度 O(N*M)，空间复杂度 O(N*M)。
 * 对超过 64KB 的数据自动降级为逐字节比较以避免内存爆炸。
 */
QList<QPair<int, int>> DataComparator::computeLcs(const QByteArray &dataA,
                                                   const QByteArray &dataB) const
{
    const int lenA = dataA.size();
    const int lenB = dataB.size();

    /* 空数据处理 */
    if (lenA == 0 || lenB == 0) {
        return {};
    }

    /*
     * 内存保护: 当输入超过 64KB 时，LCS DP 表的内存消耗过大
     * (>4GB)，降级为逐字节对齐比较。此时仅比较等长前缀部分，
     * 长度差异直接标记为插入/删除。
     */
    const int kLcsMaxSize = 65536;
    if (lenA > kLcsMaxSize || lenB > kLcsMaxSize) {
        QList<QPair<int, int>> matches;
        const int commonLen = qMin(lenA, lenB);
        matches.reserve(commonLen / 4);
        for (int i = 0; i < commonLen; ++i) {
            if (dataA[i] == dataB[i]) {
                matches.append({i, i});
            }
        }
        return matches;
    }

    /* 构建 DP 表 */
    QVector<QVector<int>> dp(lenA + 1, QVector<int>(lenB + 1, 0));

    for (int i = 1; i <= lenA; ++i) {
        const char byteA = dataA[i - 1];
        for (int j = 1; j <= lenB; ++j) {
            if (byteA == dataB[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = qMax(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    /* 回溯提取匹配对 */
    QList<QPair<int, int>> matches;
    matches.reserve(dp[lenA][lenB]);
    int i = lenA;
    int j = lenB;
    while (i > 0 && j > 0) {
        if (dataA[i - 1] == dataB[j - 1]) {
            matches.prepend({i - 1, j - 1});
            --i;
            --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            --i;
        } else {
            --j;
        }
    }

    return matches;
}

/**
 * @brief 根据 LCS 匹配对生成差异块列表
 * @param matches LCS 匹配对列表
 * @param dataA 数据 A
 * @param dataB 数据 B
 * @return 差异块列表，按偏移升序排列
 *
 * 遍历匹配对之间的间隔，根据 A/B 各自的游标差判定插入/删除/修改。
 */
QList<DataComparator::DiffBlock> DataComparator::buildDiffBlocks(
    const QList<QPair<int, int>> &matches,
    const QByteArray &dataA,
    const QByteArray &dataB) const
{
    QList<DiffBlock> blocks;
    const int lenA = dataA.size();
    const int lenB = dataB.size();

    int posA = 0; ///< 数据 A 当前游标
    int posB = 0; ///< 数据 B 当前游标

    for (const auto &match : matches) {
        const int matchA = match.first;
        const int matchB = match.second;

        /* 匹配对之间存在间隔 → 生成差异块 */
        if (posA < matchA || posB < matchB) {
            DiffBlock block;
            block.offsetA = posA;
            block.offsetB = posB;
            block.dataA = dataA.mid(posA, matchA - posA);
            block.dataB = dataB.mid(posB, matchB - posB);
            block.length = qMax(matchA - posA, matchB - posB);

            const bool hasA = (matchA > posA);
            const bool hasB = (matchB > posB);

            if (hasA && hasB) {
                block.type = DiffType::Modified;
            } else if (hasA) {
                block.type = DiffType::Deleted;
            } else {
                block.type = DiffType::Inserted;
            }

            blocks.append(block);
        }

        /* 匹配字节 → 生成相等块 */
        DiffBlock equalBlock;
        equalBlock.offsetA = matchA;
        equalBlock.offsetB = matchB;
        equalBlock.length = 1;
        equalBlock.type = DiffType::Equal;
        equalBlock.dataA = QByteArray(1, dataA[matchA]);
        equalBlock.dataB = QByteArray(1, dataB[matchB]);
        blocks.append(equalBlock);

        posA = matchA + 1;
        posB = matchB + 1;
    }

    /* 尾部剩余部分 */
    if (posA < lenA || posB < lenB) {
        DiffBlock block;
        block.offsetA = posA;
        block.offsetB = posB;
        block.dataA = dataA.mid(posA);
        block.dataB = dataB.mid(posB);
        block.length = qMax(lenA - posA, lenB - posB);

        const bool hasA = (posA < lenA);
        const bool hasB = (posB < lenB);

        if (hasA && hasB) {
            block.type = DiffType::Modified;
        } else if (hasA) {
            block.type = DiffType::Deleted;
        } else {
            block.type = DiffType::Inserted;
        }

        blocks.append(block);
    }

    return blocks;
}

/**
 * @brief 执行字节级 LCS 比较并生成差异块列表
 * @param dataA 数据缓冲区 A
 * @param dataB 数据缓冲区 B
 * @return 差异块列表
 *
 * 完成后发射 comparisonComplete 信号，每个非 Equal 差异块逐个发射 diffDetected。
 */
QList<DataComparator::DiffBlock> DataComparator::compare(const QByteArray &dataA,
                                                          const QByteArray &dataB)
{
    /* 计算 LCS 匹配对 */
    const auto matches = computeLcs(dataA, dataB);

    /* 生成差异块列表 */
    m_diffBlocks = buildDiffBlocks(matches, dataA, dataB);

    /* 计算相似度 */
    const double sim = similarity(dataA, dataB);

    /* 更新累计统计 */
    ++m_stats.totalComparisons;
    const quint64 sizeA = static_cast<quint64>(dataA.size());
    const quint64 sizeB = static_cast<quint64>(dataB.size());
    const quint64 maxSize = qMax(sizeA, sizeB);

    if (m_stats.totalComparisons == 1) {
        m_stats.minDataSize = maxSize;
        m_stats.maxDataSize = maxSize;
    } else {
        m_stats.minDataSize = qMin(m_stats.minDataSize, maxSize);
        m_stats.maxDataSize = qMax(m_stats.maxDataSize, maxSize);
    }

    /* 统计各类字节数 */
    quint64 equalBytes = 0;
    quint64 diffBytes = 0;
    quint64 insertedBytes = 0;
    quint64 deletedBytes = 0;

    for (const auto &block : m_diffBlocks) {
        switch (block.type) {
        case DiffType::Equal:
            equalBytes += static_cast<quint64>(block.length);
            break;
        case DiffType::Modified:
        case DiffType::Different:
            diffBytes += static_cast<quint64>(block.length);
            break;
        case DiffType::Inserted:
            insertedBytes += static_cast<quint64>(block.dataB.size());
            break;
        case DiffType::Deleted:
            deletedBytes += static_cast<quint64>(block.dataA.size());
            break;
        }
    }

    m_stats.totalEqualBytes += equalBytes;
    m_stats.totalDifferentBytes += diffBytes;
    m_stats.totalInsertedBytes += insertedBytes;
    m_stats.totalDeletedBytes += deletedBytes;
    m_similaritySum += sim;
    m_stats.avgSimilarity = static_cast<quint64>(m_similaritySum / m_stats.totalComparisons);

    /* 发射信号 */
    emit comparisonComplete(sim);
    for (const auto &block : m_diffBlocks) {
        if (block.type != DiffType::Equal) {
            emit diffDetected(block);
        }
    }

    return m_diffBlocks;
}

/**
 * @brief 计算两个数据缓冲区的相似度百分比
 * @param dataA 数据缓冲区 A
 * @param dataB 数据缓冲区 B
 * @return 相似度 (0.0 ~ 100.0)
 *
 * 基于 LCS 匹配字节数 / max(lenA, lenB)。双空输入返回 100.0。
 */
double DataComparator::similarity(const QByteArray &dataA,
                                  const QByteArray &dataB) const
{
    const int lenA = dataA.size();
    const int lenB = dataB.size();

    if (lenA == 0 && lenB == 0) {
        return 100.0;
    }

    const int maxLen = qMax(lenA, lenB);
    if (maxLen == 0) {
        return 100.0;
    }

    const auto matches = computeLcs(dataA, dataB);
    return matches.size() * 100.0 / maxLen;
}

/** @brief 获取最近一次比较的差异块列表 @return 差异块列表 */
QList<DataComparator::DiffBlock> DataComparator::diffBlocks() const
{
    return m_diffBlocks;
}

/**
 * @brief 将差异导出为 CSV 格式文本
 * @return CSV 文本，首行为表头: OffsetA,OffsetB,Length,Type,HexA,HexB
 *
 * 仅导出非 Equal 的差异块，Hex 列以空格分隔的十六进制字符串表示。
 */
QString DataComparator::exportDiff() const
{
    QString csv = QStringLiteral("OffsetA,OffsetB,Length,Type,HexA,HexB\n");

    for (const auto &block : m_diffBlocks) {
        if (block.type == DiffType::Equal) {
            continue;
        }

        /* 差异类型转字符串 */
        QString typeStr;
        switch (block.type) {
        case DiffType::Different: typeStr = tr("Different"); break;
        case DiffType::Inserted:  typeStr = tr("Inserted");  break;
        case DiffType::Deleted:   typeStr = tr("Deleted");   break;
        case DiffType::Modified:  typeStr = tr("Modified");  break;
        default:                  typeStr = tr("Unknown");   break;
        }

        /* 将原始字节转为空格分隔的 Hex 字符串 */
        const QString hexA = QString::fromLatin1(block.dataA.toHex(' ')).toUpper();
        const QString hexB = QString::fromLatin1(block.dataB.toHex(' ')).toUpper();

        csv += QStringLiteral("%1,%2,%3,%4,%5,%6\n")
                   .arg(block.offsetA)
                   .arg(block.offsetB)
                   .arg(block.length)
                   .arg(typeStr)
                   .arg(hexA.isEmpty() ? QStringLiteral("-") : hexA)
                   .arg(hexB.isEmpty() ? QStringLiteral("-") : hexB);
    }

    return csv;
}

/**
 * @brief 生成 Hex 并排对比文本
 * @param bytesPerLine 每行显示的字节数（默认 16）
 * @return 多行文本，左侧数据 A、右侧数据 B，差异字节以 '!' 标记
 *
 * 格式示例:
 * @code
 * 00000000  48 65 6C 6C 6F  |  48 65 6C 6C 6F
 * 00000005  57 6F 72 6C 64  |  57 6F 72 6C 64  ! byte 4 differs
 * @endcode
 */
QString DataComparator::exportHexDiff(int bytesPerLine) const
{
    if (m_diffBlocks.isEmpty()) {
        return tr("尚未执行比较或无差异");
    }

    if (bytesPerLine <= 0) {
        bytesPerLine = 16;
    }

    /* 从差异块重建完整的 A/B 数据 */
    QByteArray fullA;
    QByteArray fullB;
    for (const auto &block : m_diffBlocks) {
        if (block.type == DiffType::Equal) {
            fullA += block.dataA;
            fullB += block.dataB;
        } else if (block.type == DiffType::Deleted) {
            fullA += block.dataA;
        } else if (block.type == DiffType::Inserted) {
            fullB += block.dataB;
        } else {
            /* Modified / Different: 两端都有数据 */
            fullA += block.dataA;
            fullB += block.dataB;
        }
    }

    const int maxLen = qMax(fullA.size(), fullB.size());
    if (maxLen == 0) {
        return tr("两个缓冲区均为空");
    }

    QString result;
    for (int offset = 0; offset < maxLen; offset += bytesPerLine) {
        /* 行偏移地址 */
        result += QStringLiteral("%1  ")
                      .arg(offset, 8, 16, QChar('0'))
                      .toUpper();

        /* 左侧: 数据 A 的 Hex */
        QString hexA;
        for (int i = 0; i < bytesPerLine && (offset + i) < fullA.size(); ++i) {
            hexA += QStringLiteral("%1 ")
                        .arg(static_cast<quint8>(fullA[offset + i]), 2, 16, QChar('0'))
                        .toUpper();
        }
        /* 补齐空白 */
        const int paddingLen = bytesPerLine * 3;
        result += hexA.leftJustified(paddingLen);

        result += QStringLiteral(" |  ");

        /* 右侧: 数据 B 的 Hex */
        QString hexB;
        for (int i = 0; i < bytesPerLine && (offset + i) < fullB.size(); ++i) {
            hexB += QStringLiteral("%1 ")
                        .arg(static_cast<quint8>(fullB[offset + i]), 2, 16, QChar('0'))
                        .toUpper();
        }
        result += hexB.leftJustified(paddingLen);

        /* 差异标记行: 标记该行中不同的字节位置 */
        bool hasDiff = false;
        QString markers;
        int pos = 0;
        for (int i = 0; i < bytesPerLine && (offset + i) < maxLen; ++i) {
            const bool aExists = (offset + i) < fullA.size();
            const bool bExists = (offset + i) < fullB.size();

            if (aExists && bExists) {
                if (fullA[offset + i] != fullB[offset + i]) {
                    hasDiff = true;
                    markers += QStringLiteral("! ");
                } else {
                    markers += QStringLiteral("  ");
                }
            } else {
                /* 一端缺失 */
                hasDiff = true;
                markers += QStringLiteral("! ");
            }
            pos += 3;
        }

        if (hasDiff) {
            result += QStringLiteral("  ") + markers;
        }

        result += QLatin1Char('\n');
    }

    return result;
}
