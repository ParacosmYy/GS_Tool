/**
 * @file HexDiffEngineDiff.cpp
 * @brief 十六进制差异引擎 — Hex dump 与差异格式化输出
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 包含并排 hex dump、带上下文行的差异块输出、统一差异格式的实现。
 */

#include "utils/hex_diff/HexDiffEngine.h"

#include <algorithm>

/* ═══════════════════════════════════════════════════════════════
 *  并排 Hex Dump
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 生成并排 hex dump 文本
 * @param dataA 数据 A
 * @param dataB 数据 B
 * @param bytesPerLine 每行字节数（默认 16）
 * @return 类 hexdump -C 格式的多行文本
 *
 * 格式示例:
 * @code
 * 00000000  48 65 6C 6C 6F  Hello  |  48 65 6C 6C 6F  Hello
 * @endcode
 * 差异字节以 '!' 标记，插入以 '+'，删除以 '-'。
 */
QString HexDiffEngine::sideBySideHexDump(const QByteArray &dataA,
                                         const QByteArray &dataB,
                                         int bytesPerLine) const
{
    if (bytesPerLine <= 0) {
        bytesPerLine = 16;
    }

    const int maxLen = std::max(dataA.size(), dataB.size());
    if (maxLen == 0) {
        return tr("两个缓冲区均为空");
    }

    QString result;
    result.reserve(maxLen * 8);

    for (int offset = 0; offset < maxLen; offset += bytesPerLine) {
        /* 行偏移地址 */
        result += formatOffset(static_cast<quint64>(offset));
        result += QStringLiteral("  ");

        /* 左侧: 数据 A 的 Hex + ASCII */
        QString hexA;
        QString asciiA;
        for (int i = 0; i < bytesPerLine; ++i) {
            const int idx = offset + i;
            if (idx < dataA.size()) {
                hexA += formatHexByte(static_cast<quint8>(dataA[idx])) + QLatin1Char(' ');
                const char ch = dataA[idx];
                asciiA += (ch >= 0x20 && ch <= 0x7E) ? QLatin1Char(ch) : QLatin1Char('.');
            } else {
                hexA += QStringLiteral("   ");
                asciiA += QLatin1Char(' ');
            }
        }
        result += hexA;
        result += asciiA;

        result += QStringLiteral("  |  ");

        /* 右侧: 数据 B 的 Hex + ASCII */
        QString hexB;
        QString asciiB;
        for (int i = 0; i < bytesPerLine; ++i) {
            const int idx = offset + i;
            if (idx < dataB.size()) {
                hexB += formatHexByte(static_cast<quint8>(dataB[idx])) + QLatin1Char(' ');
                const char ch = dataB[idx];
                asciiB += (ch >= 0x20 && ch <= 0x7E) ? QLatin1Char(ch) : QLatin1Char('.');
            } else {
                hexB += QStringLiteral("   ");
                asciiB += QLatin1Char(' ');
            }
        }
        result += hexB;
        result += asciiB;

        /* 差异标记行 */
        bool hasDiff = false;
        QString markers;
        for (int i = 0; i < bytesPerLine; ++i) {
            const int idx = offset + i;
            if (idx >= maxLen) {
                break;
            }
            const bool aOk = idx < dataA.size();
            const bool bOk = idx < dataB.size();

            if (aOk && bOk) {
                if (dataA[idx] != dataB[idx]) {
                    markers += QLatin1Char('!');
                    hasDiff = true;
                } else {
                    markers += QLatin1Char(' ');
                }
            } else if (bOk && !aOk) {
                markers += QLatin1Char('+');
                hasDiff = true;
            } else if (aOk && !bOk) {
                markers += QLatin1Char('-');
                hasDiff = true;
            }
        }

        if (hasDiff) {
            result += QStringLiteral("  | ");
            result += markers;
        }

        result += QLatin1Char('\n');
    }

    return result;
}

/* ═══════════════════════════════════════════════════════════════
 *  带上下文行的差异块输出
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 生成带上下文行的差异块文本
 * @param dataA 数据 A
 * @param dataB 数据 B
 * @param contextBytes 上下文字节数（默认 8）
 * @return 仅包含差异块及其上下文的文本
 *
 * 先执行逐字节比较找到差异块，再输出每个差异块前后
 * contextBytes 字节的上下文。
 */
QString HexDiffEngine::contextDiff(const QByteArray &dataA,
                                   const QByteArray &dataB,
                                   int contextBytes) const
{
    if (contextBytes < 0) {
        contextBytes = 8;
    }

    const auto entries = const_cast<HexDiffEngine*>(this)->compare(dataA, dataB);
    if (entries.isEmpty()) {
        return tr("无数据可比较");
    }

    const auto blocks = compareBlocks(entries);

    /* 过滤出非匹配块 */
    bool hasAnyDiff = false;
    for (const auto &block : blocks) {
        if (block.kind != DiffKind::Match) {
            hasAnyDiff = true;
            break;
        }
    }
    if (!hasAnyDiff) {
        return tr("数据完全相同");
    }

    QString result;
    const int maxLen = std::max(dataA.size(), dataB.size());

    for (const auto &block : blocks) {
        if (block.kind == DiffKind::Match) {
            continue;
        }

        /* 计算上下文范围 */
        const quint64 ctxStart = (block.startOffset > static_cast<quint64>(contextBytes))
                                     ? block.startOffset - contextBytes
                                     : 0;
        const quint64 blockEnd = block.startOffset + static_cast<quint64>(block.length);
        const quint64 ctxEnd = std::min(blockEnd + static_cast<quint64>(contextBytes),
                                        static_cast<quint64>(maxLen));

        result += QStringLiteral("***************\n");
        result += QStringLiteral("--- %1\n").arg(formatOffset(ctxStart));
        result += QStringLiteral("+++ %1\n").arg(formatOffset(ctxStart));

        for (quint64 i = ctxStart; i < ctxEnd; ++i) {
            const int idx = static_cast<int>(i);
            const bool inBlock = (i >= block.startOffset && i < blockEnd);

            const bool aOk = idx < dataA.size();
            const bool bOk = idx < dataB.size();

            QString prefix = QStringLiteral("  ");
            if (inBlock) {
                if (aOk && bOk) {
                    if (dataA[idx] != dataB[idx]) {
                        prefix = QStringLiteral("! ");
                    }
                } else if (bOk && !aOk) {
                    prefix = QStringLiteral("+ ");
                } else if (aOk && !bOk) {
                    prefix = QStringLiteral("- ");
                }
            }

            const QString hexA = aOk ? formatHexByte(static_cast<quint8>(dataA[idx]))
                                     : QStringLiteral("--");
            const QString hexB = bOk ? formatHexByte(static_cast<quint8>(dataB[idx]))
                                     : QStringLiteral("--");

            result += QStringLiteral("%1%2  %3  %4\n")
                          .arg(prefix)
                          .arg(formatOffset(i))
                          .arg(hexA)
                          .arg(hexB);
        }
    }

    return result;
}

/* ═══════════════════════════════════════════════════════════════
 *  统一差异格式
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 生成统一差异格式输出
 * @param dataA 数据 A
 * @param dataB 数据 B
 * @param labelA 数据 A 标签
 * @param labelB 数据 B 标签
 * @return 统一差异格式文本
 *
 * 格式类似 unified diff:
 * @code
 * --- A
 * +++ B
 * @@ offset,length @@
 *  48 65 6C 6C 6F
 * -57 6F 72 6C 64
 * +57 6F 72 6C 45
 * @endcode
 */
QString HexDiffEngine::unifiedDiff(const QByteArray &dataA,
                                   const QByteArray &dataB,
                                   const QString &labelA,
                                   const QString &labelB) const
{
    QString result;
    result += QStringLiteral("--- %1\n").arg(labelA);
    result += QStringLiteral("+++ %2\n").arg(labelB);

    const int maxLen = std::max(dataA.size(), dataB.size());
    if (maxLen == 0) {
        result += tr("两个缓冲区均为空\n");
        return result;
    }

    /* 逐字节比较，寻找差异区域 */
    const int commonLen = std::min(dataA.size(), dataB.size());

    int i = 0;
    while (i < maxLen) {
        /* 跳过匹配字节 */
        bool hasDiff = false;
        if (i < commonLen) {
            hasDiff = (dataA[i] != dataB[i]);
        } else {
            hasDiff = true;
        }

        if (!hasDiff) {
            ++i;
            continue;
        }

        /* 找到差异块的结束位置 */
        int blockStart = i;
        int blockEnd = i;
        while (blockEnd < maxLen) {
            if (blockEnd < commonLen) {
                if (dataA[blockEnd] == dataB[blockEnd]) {
                    /* 检查后续是否还有差异（简单贪心） */
                    bool moreDiff = false;
                    for (int peek = blockEnd + 1;
                         peek < std::min(blockEnd + 9, maxLen); ++peek) {
                        if (peek < commonLen) {
                            if (dataA[peek] != dataB[peek]) { moreDiff = true; break; }
                        } else {
                            moreDiff = true; break;
                        }
                    }
                    if (!moreDiff) break;
                }
            }
            ++blockEnd;
        }

        /* 输出差异块标题 */
        result += QStringLiteral("@@ %1,%2 @@\n")
                      .arg(blockStart)
                      .arg(blockEnd - blockStart);

        /* 输出差异块内容 */
        for (int j = blockStart; j < blockEnd; ++j) {
            if (j < commonLen) {
                if (dataA[j] == dataB[j]) {
                    result += QStringLiteral(" %1\n")
                                  .arg(formatHexByte(static_cast<quint8>(dataA[j])));
                } else {
                    result += QStringLiteral("-%1\n")
                                  .arg(formatHexByte(static_cast<quint8>(dataA[j])));
                    result += QStringLiteral("+%1\n")
                                  .arg(formatHexByte(static_cast<quint8>(dataB[j])));
                }
            } else if (j < dataA.size()) {
                result += QStringLiteral("-%1\n")
                              .arg(formatHexByte(static_cast<quint8>(dataA[j])));
            } else if (j < dataB.size()) {
                result += QStringLiteral("+%1\n")
                              .arg(formatHexByte(static_cast<quint8>(dataB[j])));
            }
        }

        i = blockEnd;
    }

    return result;
}
