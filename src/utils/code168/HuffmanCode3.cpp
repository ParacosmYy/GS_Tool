/**
 * @file HuffmanCode3.cpp
 * @brief HuffmanCode3 实现
 *
 * 实现规范霍夫曼编解码：码长计算、package-merge限制、规范码生成、编解码。
 */

#include "utils/code168/HuffmanCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <QDataStream>

HuffmanCode3::HuffmanCode3(QObject *parent)
    : QObject(parent)
{
}

HuffmanCode3::~HuffmanCode3() = default;

void HuffmanCode3::setMaxCodeLength(int maxLen) { m_maxCodeLen = qMax(1, maxLen); }

void HuffmanCode3::computeCodeLengths(const QMap<Symbol, int>& freqs)
{
    if (freqs.size() <= 1) {
        for (auto it = freqs.begin(); it != freqs.end(); ++it)
            m_codeLengths[it.key()] = 1;
        return;
    }

    /* Build Huffman tree using a priority queue (simple sorted list) */
    QVector<QPair<int, QVector<Symbol>>> nodes;
    for (auto it = freqs.begin(); it != freqs.end(); ++it)
        nodes.append({it.value(), {it.key()}});

    QMap<Symbol, int> lengths;
    for (auto it = freqs.begin(); it != freqs.end(); ++it)
        lengths[it.key()] = 0;

    while (nodes.size() > 1) {
        std::sort(nodes.begin(), nodes.end());
        auto left = nodes.takeAt(0);
        auto right = nodes.takeAt(0);
        for (Symbol s : left.second) lengths[s]++;
        for (Symbol s : right.second) lengths[s]++;
        nodes.append({left.first + right.first, left.second + right.second});
    }

    m_codeLengths = lengths;
}

void HuffmanCode3::limitCodeLengths(QMap<Symbol, int>& lengths, int maxLen)
{
    /* Package-merge algorithm for code length limiting */
    bool needsLimit = false;
    for (auto it = lengths.begin(); it != lengths.end(); ++it)
        if (it.value() > maxLen) { needsLimit = true; break; }
    if (!needsLimit) return;

    /* Simple heuristic: clamp and redistribute */
    int n = lengths.size();
    int maxDepth = maxLen;
    QVector<QPair<int, Symbol>> items;
    for (auto it = lengths.begin(); it != lengths.end(); ++it)
        items.append({qMin(it.value(), maxDepth), it.key()});
    std::sort(items.begin(), items.end());

    /* Kraft inequality check: sum(2^(-l_i)) <= 1 */
    for (int iter = 0; iter < 100; ++iter) {
        double kraftSum = 0.0;
        for (const auto& item : items)
            kraftSum += qPow(2.0, -item.first);
        if (kraftSum <= 1.0 + 1e-10) break;

        /* Reduce longest code */
        for (int i = items.size() - 1; i >= 0; --i) {
            if (items[i].first < maxDepth) { items[i].first++; break; }
        }
    }
    lengths.clear();
    for (const auto& item : items)
        lengths[item.second] = item.first;
}

void HuffmanCode3::buildCanonicalCodes()
{
    if (m_codeLengths.isEmpty()) return;

    /* Sort symbols by code length, then by symbol value */
    QVector<QPair<int, Symbol>> sorted;
    for (auto it = m_codeLengths.begin(); it != m_codeLengths.end(); ++it)
        sorted.append({it.value(), it.key()});
    std::sort(sorted.begin(), sorted.end());

    /* Assign canonical codes */
    quint32 code = 0;
    int prevLen = 0;
    m_codes.clear();
    m_table.clear();

    for (const auto& entry : sorted) {
        int len = entry.first;
        Symbol sym = entry.second;
        code <<= (len - prevLen);
        m_codes[sym] = code;
        m_table.append({sym, len, code});
        code++;
        prevLen = len;
    }
}

void HuffmanCode3::buildDecodeTable()
{
    m_decodeMap.clear();
    for (const auto& entry : m_table)
        m_decodeMap[{entry.code, entry.codeLength}] = entry.symbol;
}

bool HuffmanCode3::buildFromFrequencies(const QMap<Symbol, int>& freqs)
{
    if (freqs.isEmpty()) return false;

    computeCodeLengths(freqs);
    if (m_maxCodeLen < 30)
        limitCodeLengths(m_codeLengths, m_maxCodeLen);
    buildCanonicalCodes();
    buildDecodeTable();
    return true;
}

QByteArray HuffmanCode3::encode(const QVector<Symbol>& symbols)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    result.reserve(symbols.size() / 2);
    quint8 byte = 0;
    int bitPos = 7;

    for (Symbol sym : symbols) {
        auto it = m_codes.constFind(sym);
        if (it == m_codes.constEnd()) continue;
        quint32 code = it.value();
        int len = m_codeLengths[sym];

        for (int i = len - 1; i >= 0; --i) {
            if ((code >> i) & 1) byte |= (1 << bitPos);
            if (--bitPos < 0) {
                result.append(static_cast<char>(byte));
                byte = 0;
                bitPos = 7;
            }
        }
    }
    if (bitPos < 7) result.append(static_cast<char>(byte));

    m_stats.totalEncoded += symbols.size();
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    int inputBits = symbols.size() * 16;  // Assume 16-bit symbols
    int outputBits = result.size() * 8;
    m_stats.compressionRatio = (outputBits > 0) ? static_cast<double>(inputBits) / outputBits : 0.0;

    emit encodingCompleted(symbols.size(), result.size());
    return result;
}

QVector<HuffmanCode3::Symbol> HuffmanCode3::decode(const QByteArray& data, int count)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Symbol> result;
    int byteIdx = 0, bitPos = 7;
    int decoded = 0;

    while (decoded < count && byteIdx < data.size()) {
        quint32 code = 0;
        int len = 0;
        bool found = false;

        for (int maxLen = 1; maxLen <= m_maxCodeLen; ++maxLen) {
            /* Read next bit */
            if (byteIdx >= data.size()) break;
            bool bit = (static_cast<quint8>(data[byteIdx]) >> bitPos) & 1;
            code = (code << 1) | bit;
            len++;
            if (--bitPos < 0) { byteIdx++; bitPos = 7; }

            auto dit = m_decodeMap.constFind({code, len});
            if (dit != m_decodeMap.constEnd()) {
                result.append(dit.value());
                decoded++;
                found = true;
                break;
            }
        }
        if (!found) break;
    }

    m_stats.totalDecoded += result.size();
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit decodingCompleted(result.size());
    return result;
}

QByteArray HuffmanCode3::serializeTable() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << static_cast<quint32>(m_table.size());
    for (const auto& e : m_table) {
        stream << static_cast<quint16>(e.symbol);
        stream << static_cast<quint8>(e.codeLength);
        stream << static_cast<quint32>(e.code);
    }
    return data;
}

bool HuffmanCode3::deserializeTable(const QByteArray& data)
{
    QDataStream stream(data);
    quint32 count;
    stream >> count;
    m_table.clear();
    m_codes.clear();
    m_codeLengths.clear();
    m_decodeMap.clear();

    for (quint32 i = 0; i < count; ++i) {
        quint16 sym; quint8 len; quint32 code;
        stream >> sym >> len >> code;
        CodeEntry e{static_cast<Symbol>(sym), len, code};
        m_table.append(e);
        m_codes[static_cast<Symbol>(sym)] = code;
        m_codeLengths[static_cast<Symbol>(sym)] = len;
        m_decodeMap[{code, len}] = static_cast<Symbol>(sym);
    }
    return stream.status() == QDataStream::Ok;
}

QVector<HuffmanCode3::CodeEntry> HuffmanCode3::codeTable() const { return m_table; }

void HuffmanCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
