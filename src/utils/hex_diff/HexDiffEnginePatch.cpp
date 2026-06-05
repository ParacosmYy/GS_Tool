/**
 * @file HexDiffEnginePatch.cpp
 * @brief 十六进制差异引擎 — 三路比较与补丁操作
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 包含三路比较（base/A/B 共识与冲突检测）、补丁生成与应用的实现。
 */

#include "utils/hex_diff/HexDiffEngine.h"

#include <algorithm>

/* ═══════════════════════════════════════════════════════════════
 *  三路比较
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 执行三路比较
 * @param base 基准数据
 * @param dataA 分支 A
 * @param dataB 分支 B
 * @return 三路比较结果
 *
 * 逐字节比较 base、A、B 三者:
 * - 三者一致 → 共识
 * - base==A 且 base!=B → B 修改
 * - base==B 且 base!=A → A 修改
 * - base!=A 且 base!=B 且 A==B → 双方相同修改（共识）
 * - 其他 → 冲突
 */
HexDiffEngine::ThreeWayResult HexDiffEngine::threeWayCompare(const QByteArray &base,
                                                             const QByteArray &dataA,
                                                             const QByteArray &dataB)
{
    ThreeWayResult result;
    const int maxLen = std::max({base.size(), dataA.size(), dataB.size()});

    result.entries.reserve(maxLen);

    int consensus = 0;
    int conflict = 0;

    for (int i = 0; i < maxLen; ++i) {
        DiffEntry entry;
        entry.offset = static_cast<quint64>(i);

        const bool hasBase = i < base.size();
        const bool hasA = i < dataA.size();
        const bool hasB = i < dataB.size();

        entry.byteA = hasA ? static_cast<quint8>(dataA[i]) : 0;
        entry.byteB = hasB ? static_cast<quint8>(dataB[i]) : 0;

        const quint8 baseVal = hasBase ? static_cast<quint8>(base[i]) : 0;

        if (hasBase && hasA && hasB) {
            if (baseVal == entry.byteA && baseVal == entry.byteB) {
                /* 三者完全一致 */
                entry.kind = DiffKind::Match;
                entry.color = ColorCode::Normal;
                ++consensus;
            } else if (baseVal == entry.byteA && baseVal != entry.byteB) {
                /* B 修改 */
                entry.kind = DiffKind::Mismatch;
                entry.color = ColorCode::Red;
            } else if (baseVal == entry.byteB && baseVal != entry.byteA) {
                /* A 修改 */
                entry.kind = DiffKind::Mismatch;
                entry.color = ColorCode::Red;
            } else if (entry.byteA == entry.byteB) {
                /* A 和 B 相同修改 → 共识 */
                entry.kind = DiffKind::Match;
                entry.color = ColorCode::Normal;
                ++consensus;
            } else {
                /* 冲突 */
                entry.kind = DiffKind::Mismatch;
                entry.color = ColorCode::Red;
                ++conflict;
            }
        } else if (!hasBase && hasA && hasB) {
            if (entry.byteA == entry.byteB) {
                entry.kind = DiffKind::Inserted;
                entry.color = ColorCode::Green;
                ++consensus;
            } else {
                entry.kind = DiffKind::Mismatch;
                entry.color = ColorCode::Red;
                ++conflict;
            }
        } else if (hasA && !hasB) {
            entry.kind = DiffKind::Deleted;
            entry.color = ColorCode::Blue;
        } else if (!hasA && hasB) {
            entry.kind = DiffKind::Inserted;
            entry.color = ColorCode::Green;
        } else {
            entry.kind = DiffKind::Match;
            entry.color = ColorCode::Normal;
        }

        result.entries.append(entry);
    }

    result.consensusBytes = consensus;
    result.conflictBytes = conflict;
    const int total = std::max(1, maxLen);
    result.consensusRate = static_cast<double>(consensus) * 100.0 / static_cast<double>(total);

    return result;
}

/* ═══════════════════════════════════════════════════════════════
 *  补丁生成
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 生成将 dataA 变换为 dataB 的补丁
 * @param dataA 原始数据
 * @param dataB 目标数据
 * @return 补丁条目列表
 *
 * 对公共段逐字节比较生成 Equal/Replace 操作，
 * 超出公共段的部分生成 Insert/Delete 操作。
 */
QList<HexDiffEngine::PatchEntry> HexDiffEngine::generatePatch(const QByteArray &dataA,
                                                               const QByteArray &dataB)
{
    QList<PatchEntry> patch;
    const int commonLen = std::min(dataA.size(), dataB.size());

    patch.reserve(std::max(dataA.size(), dataB.size()));

    /* 公共段 */
    for (int i = 0; i < commonLen; ++i) {
        PatchEntry entry;
        entry.offset = static_cast<quint64>(i);
        entry.originalData = QByteArray(1, dataA[i]);

        if (dataA[i] == dataB[i]) {
            entry.op = PatchOp::Equal;
            entry.newData = entry.originalData;
        } else {
            entry.op = PatchOp::Replace;
            entry.newData = QByteArray(1, dataB[i]);
        }
        patch.append(entry);
    }

    /* A 比 B 长 → 删除 */
    for (int i = commonLen; i < dataA.size(); ++i) {
        PatchEntry entry;
        entry.offset = static_cast<quint64>(i);
        entry.op = PatchOp::Delete;
        entry.originalData = QByteArray(1, dataA[i]);
        patch.append(entry);
    }

    /* B 比 A 长 → 插入 */
    for (int i = commonLen; i < dataB.size(); ++i) {
        PatchEntry entry;
        entry.offset = static_cast<quint64>(commonLen);
        entry.op = PatchOp::Insert;
        entry.originalData.clear();
        entry.newData = QByteArray(1, dataB[i]);
        patch.append(entry);
    }

    ++m_stats.totalPatchesGenerated;
    return patch;
}

/* ═══════════════════════════════════════════════════════════════
 *  补丁应用
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 应用补丁到数据
 * @param data 原始数据
 * @param patch 补丁条目列表
 * @param verify 是否校验原始字节（默认 true）
 * @return 应用补丁后的数据；校验失败返回空 QByteArray
 *
 * 按偏移顺序应用每个补丁操作。Insert 在当前偏移处插入，
 * Delete 跳过原始字节，Equal 直接复制，Replace 替换。
 */
QByteArray HexDiffEngine::applyPatch(const QByteArray &data,
                                     const QList<PatchEntry> &patch,
                                     bool verify)
{
    QByteArray result;
    result.reserve(data.size() + patch.size() / 4);

    quint64 dataPos = 0;

    for (const auto &entry : patch) {
        switch (entry.op) {
        case PatchOp::Equal:
            /* 校验原始字节 */
            if (verify) {
                if (entry.offset >= static_cast<quint64>(data.size())) {
                    return QByteArray();
                }
                if (data.mid(static_cast<int>(entry.offset), entry.originalData.size())
                    != entry.originalData) {
                    return QByteArray();
                }
            }
            result.append(entry.newData);
            dataPos = entry.offset + 1;
            break;

        case PatchOp::Replace:
            if (verify) {
                if (entry.offset >= static_cast<quint64>(data.size())) {
                    return QByteArray();
                }
                if (data.mid(static_cast<int>(entry.offset), entry.originalData.size())
                    != entry.originalData) {
                    return QByteArray();
                }
            }
            result.append(entry.newData);
            dataPos = entry.offset + 1;
            break;

        case PatchOp::Insert:
            result.append(entry.newData);
            break;

        case PatchOp::Delete:
            if (verify) {
                if (entry.offset >= static_cast<quint64>(data.size())) {
                    return QByteArray();
                }
                if (data.mid(static_cast<int>(entry.offset), entry.originalData.size())
                    != entry.originalData) {
                    return QByteArray();
                }
            }
            /* 跳过被删除的字节，不写入 result */
            dataPos = entry.offset + static_cast<quint64>(entry.originalData.size());
            break;
        }
    }

    ++m_stats.totalPatchesApplied;
    return result;
}
