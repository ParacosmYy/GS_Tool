/**
 * @file FirmwareDiffer.cpp
 * @brief 固件二进制差异比较引擎 — 核心比较逻辑
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/firmware/FirmwareDiffer.h"

#include <QFile>

// ──────────────────────────── 构造 / 配置 ────────────────────────────

FirmwareDiffer::FirmwareDiffer(QObject *parent)
    : QObject(parent)
{
}

void FirmwareDiffer::setAlignment(int bytes)
{
    // 仅允许 1/2/4/8 字节对齐，非法值回退为 1
    if (bytes == 1 || bytes == 2 || bytes == 4 || bytes == 8) {
        m_alignment = bytes;
    } else {
        m_alignment = 1;
    }
}

int FirmwareDiffer::alignment() const
{
    return m_alignment;
}

void FirmwareDiffer::setIgnoreBytes(const QVector<quint8> &ignoreList)
{
    m_ignoreBytes = ignoreList;
}

// ──────────────────────────── 文件加载 ────────────────────────────

bool FirmwareDiffer::loadFirmwareA(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    m_firmwareA = file.readAll();
    file.close();
    m_filePathA = filePath;
    emit firmwareALoaded(filePath);
    return true;
}

bool FirmwareDiffer::loadFirmwareB(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    m_firmwareB = file.readAll();
    file.close();
    m_filePathB = filePath;
    emit firmwareBLoaded(filePath);
    return true;
}

// ──────────────────────────── 访问器 ────────────────────────────

QByteArray FirmwareDiffer::firmwareA() const { return m_firmwareA; }
QByteArray FirmwareDiffer::firmwareB() const { return m_firmwareB; }
QString FirmwareDiffer::filePathA() const    { return m_filePathA; }
QString FirmwareDiffer::filePathB() const    { return m_filePathB; }
bool FirmwareDiffer::isLoadedA() const       { return !m_firmwareA.isEmpty(); }
bool FirmwareDiffer::isLoadedB() const       { return !m_firmwareB.isEmpty(); }

// ──────────────────────────── 公共比较接口 ────────────────────────────

FirmwareDiffResult FirmwareDiffer::compare()
{
    FirmwareDiffResult result = doCompare(m_firmwareA, m_firmwareB, 0);
    ++m_totalComparisons;
    m_totalBytesCompared += static_cast<quint64>(
        qMax(m_firmwareA.size(), m_firmwareB.size()));
    m_totalDiffsFound += static_cast<quint64>(result.diffs.size());
    m_totalPatchesGenerated += static_cast<quint64>(result.changedBlocks);
    emit comparisonComplete(result);
    return result;
}

FirmwareDiffResult FirmwareDiffer::compareRegion(quint64 start, quint64 size)
{
    QByteArray regionA;
    QByteArray regionB;

    if (static_cast<qsizetype>(start) < m_firmwareA.size()) {
        int avail = qMin(static_cast<qsizetype>(size),
                         m_firmwareA.size() - static_cast<qsizetype>(start));
        regionA = m_firmwareA.mid(static_cast<int>(start), avail);
    }

    if (static_cast<qsizetype>(start) < m_firmwareB.size()) {
        int avail = qMin(static_cast<qsizetype>(size),
                         m_firmwareB.size() - static_cast<qsizetype>(start));
        regionB = m_firmwareB.mid(static_cast<int>(start), avail);
    }

    FirmwareDiffResult result = doCompare(regionA, regionB, start);
    ++m_totalComparisons;
    m_totalBytesCompared += static_cast<quint64>(
        qMax(regionA.size(), regionB.size()));
    m_totalDiffsFound += static_cast<quint64>(result.diffs.size());
    m_totalPatchesGenerated += static_cast<quint64>(result.changedBlocks);
    emit comparisonComplete(result);
    return result;
}

// ──────────────────────────── 核心比较实现 ────────────────────────────

FirmwareDiffResult FirmwareDiffer::doCompare(const QByteArray &dataA,
                                              const QByteArray &dataB,
                                              quint64 baseAddress) const
{
    FirmwareDiffResult result;

    const int lenA = dataA.size();
    const int lenB = dataB.size();
    const int commonLen = qMin(lenA, lenB);
    const int maxLen = qMax(lenA, lenB);

    if (maxLen == 0) {
        return result;
    }

    // ---- 逐对齐块比较 ----
    int matchedBytes = 0;
    int comparedBytes = 0;

    // 临时向量: 记录每个对齐块是否不同
    QVector<bool> blockDiff((maxLen + m_alignment - 1) / m_alignment, false);

    for (int i = 0; i < maxLen; i += m_alignment) {
        bool blockMatches = true;
        for (int j = 0; j < m_alignment && (i + j) < maxLen; ++j) {
            int idx = i + j;
            quint8 byteA = (idx < lenA) ? static_cast<quint8>(dataA[idx]) : 0;
            quint8 byteB = (idx < lenB) ? static_cast<quint8>(dataB[idx]) : 0;

            // 检查是否在忽略列表中
            bool aIgnored = (idx >= lenA) || m_ignoreBytes.contains(byteA);
            bool bIgnored = (idx >= lenB) || m_ignoreBytes.contains(byteB);

            if (aIgnored && bIgnored) {
                continue; // 双方都被忽略，视为匹配
            }
            if (byteA != byteB) {
                blockMatches = false;
                break;
            }
        }
        if (!blockMatches) {
            blockDiff[static_cast<int>(i / m_alignment)] = true;
        } else {
            matchedBytes += qMin(m_alignment, maxLen - i);
        }
        comparedBytes += qMin(m_alignment, maxLen - i);
    }

    // ---- 合并连续差异块 ----
    int blockIdx = 0;
    while (blockIdx < blockDiff.size()) {
        if (!blockDiff[blockIdx]) {
            ++blockIdx;
            continue;
        }

        // 找到连续差异块的起点和长度
        int startBlock = blockIdx;
        int diffBlockCount = 0;
        while (blockIdx < blockDiff.size() && blockDiff[blockIdx]) {
            ++diffBlockCount;
            ++blockIdx;
        }

        int byteStart = startBlock * m_alignment;
        int byteLen = diffBlockCount * m_alignment;
        // 裁剪到实际数据范围
        byteLen = qMin(byteLen, maxLen - byteStart);

        FirmwareDiffBlock block;
        block.address = baseAddress + static_cast<quint64>(byteStart);
        block.size = byteLen;

        int endA = qMin(byteStart + byteLen, lenA);
        if (byteStart < lenA) {
            block.dataA = dataA.mid(byteStart, endA - byteStart);
        }

        int endB = qMin(byteStart + byteLen, lenB);
        if (byteStart < lenB) {
            block.dataB = dataB.mid(byteStart, endB - byteStart);
        }

        result.diffs.append(block);
    }

    // ---- 长度差异: 尾部追加/删除块 ----
    if (lenB > lenA) {
        FirmwareDiffBlock block;
        block.address = baseAddress + static_cast<quint64>(lenA);
        block.size = lenB - lenA;
        block.dataA.clear();
        block.dataB = dataB.mid(lenA);
        result.diffs.append(block);
        result.addedBlocks = 1;
    } else if (lenA > lenB) {
        FirmwareDiffBlock block;
        block.address = baseAddress + static_cast<quint64>(lenB);
        block.size = lenA - lenB;
        block.dataA = dataA.mid(lenB);
        block.dataB.clear();
        result.diffs.append(block);
        result.removedBlocks = 1;
    }

    // ---- 统计 ----
    result.changedBlocks = 0;
    for (const auto &blk : result.diffs) {
        if (!blk.dataA.isEmpty() && !blk.dataB.isEmpty()) {
            ++result.changedBlocks;
        }
    }

    int totalBlocks = (maxLen + m_alignment - 1) / m_alignment;
    result.totalBlocks = totalBlocks;
    result.unchangedBlocks = totalBlocks
                             - result.changedBlocks
                             - result.addedBlocks
                             - result.removedBlocks;

    // 相似度 = 匹配字节数 / 总比较字节数 * 100
    if (comparedBytes > 0) {
        result.similarity = static_cast<double>(matchedBytes)
                            / static_cast<double>(comparedBytes) * 100.0;
    }

    return result;
}
