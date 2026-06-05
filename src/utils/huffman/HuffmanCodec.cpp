/**
 * @file HuffmanCodec.cpp
 * @brief Huffman编码器实现
 */

#include "utils/huffman/HuffmanCodec.h"
#include <QElapsedTimer>
#include <QList>
#include <algorithm>

HuffmanCodec::HuffmanCodec(QObject* parent) : QObject(parent), m_timeSum(0.0) {}

QByteArray HuffmanCodec::encode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    if (data.isEmpty()) return result;

    /* 统计频率 */
    QMap<quint8, int> freq;
    for (int i = 0; i < data.size(); ++i) freq[static_cast<quint8>(data[i])]++;

    /* 写频率表到头部(用于解码) */
    quint16 symbolCount = freq.size();
    result.append(static_cast<char>((symbolCount >> 8) & 0xFF));
    result.append(static_cast<char>(symbolCount & 0xFF));

    for (auto it = freq.begin(); it != freq.end(); ++it) {
        result.append(static_cast<char>(it.key()));
        quint32 f = it.value();
        result.append(static_cast<char>((f >> 24) & 0xFF));
        result.append(static_cast<char>((f >> 16) & 0xFF));
        result.append(static_cast<char>((f >> 8) & 0xFF));
        result.append(static_cast<char>(f & 0xFF));
    }

    /* 构建Huffman树和编码表 */
    HuffNode* root = buildTree(freq);
    QMap<quint8, QByteArray> codes;
    buildCodes(root, QByteArray(), codes);

    /* 编码数据 */
    QByteArray bitBuffer;
    int bitCount = 0;
    quint8 currentByte = 0;

    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        const QByteArray& code = codes[byte];
        for (int j = 0; j < code.size(); ++j) {
            currentByte = (currentByte << 1) | (code[j] == '1' ? 1 : 0);
            ++bitCount;
            if (bitCount == 8) {
                bitBuffer.append(static_cast<char>(currentByte));
                currentByte = 0;
                bitCount = 0;
            }
        }
    }

    /* 写入剩余位 */
    quint8 paddingBits = 0;
    if (bitCount > 0) {
        paddingBits = 8 - bitCount;
        currentByte <<= paddingBits;
        bitBuffer.append(static_cast<char>(currentByte));
    }

    /* 写padding信息和编码数据 */
    result.append(static_cast<char>(paddingBits));
    result.append(static_cast<char>((bitBuffer.size() >> 24) & 0xFF));
    result.append(static_cast<char>((bitBuffer.size() >> 16) & 0xFF));
    result.append(static_cast<char>((bitBuffer.size() >> 8) & 0xFF));
    result.append(static_cast<char>(bitBuffer.size() & 0xFF));
    result.append(bitBuffer);

    deleteTree(root);

    double elapsed = timer.elapsed();
    ++m_stats.totalEncodes;
    m_stats.totalBytesIn += data.size();
    m_stats.totalBytesOut += result.size();
    m_stats.avgCompressionRatio = (m_stats.totalBytesIn > 0)
        ? static_cast<double>(m_stats.totalBytesOut) / m_stats.totalBytesIn : 0.0;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.averageProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit encodeComplete(data.size(), result.size(), m_stats.avgCompressionRatio);
    return result;
}

QByteArray HuffmanCodec::decode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    int pos = 0;

    if (data.size() < 2) return result;

    /* 读取频率表 */
    quint16 symbolCount = (static_cast<quint8>(data[0]) << 8) | static_cast<quint8>(data[1]);
    pos = 2;

    QMap<quint8, int> freq;
    for (int i = 0; i < symbolCount && pos + 4 < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data[pos++]);
        quint32 f = (static_cast<quint8>(data[pos]) << 24) | (static_cast<quint8>(data[pos+1]) << 16) |
                    (static_cast<quint8>(data[pos+2]) << 8) | static_cast<quint8>(data[pos+3]);
        pos += 4;
        freq[byte] = f;
    }

    /* 重建树 */
    HuffNode* root = buildTree(freq);

    /* 读取padding */
    quint8 paddingBits = static_cast<quint8>(data[pos++]);

    /* 读取编码数据长度 */
    quint32 encodedLen = (static_cast<quint8>(data[pos]) << 24) | (static_cast<quint8>(data[pos+1]) << 16) |
                         (static_cast<quint8>(data[pos+2]) << 8) | static_cast<quint8>(data[pos+3]);
    pos += 4;

    /* 解码 */
    HuffNode* current = root;
    int totalBits = encodedLen * 8 - paddingBits;
    int bitIdx = 0;

    while (bitIdx < totalBits && pos < data.size()) {
        quint8 byte = static_cast<quint8>(data[pos]);
        int bitsInByte = qMin(8, totalBits - bitIdx);

        for (int b = 7; b >= 8 - bitsInByte && bitIdx < totalBits; --b) {
            bool bit = (byte >> b) & 1;
            current = bit ? current->right : current->left;
            ++bitIdx;

            if (current->left == nullptr && current->right == nullptr) {
                result.append(static_cast<char>(current->byte));
                current = root;
            }
        }
        ++pos;
    }

    deleteTree(root);

    double elapsed = timer.elapsed();
    ++m_stats.totalDecodes;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.averageProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit decodeComplete(data.size(), result.size());
    return result;
}

HuffmanCodec::HuffNode* HuffmanCodec::buildTree(const QMap<quint8, int>& freq)
{
    QList<HuffNode*> nodes;
    for (auto it = freq.begin(); it != freq.end(); ++it)
        nodes.append(new HuffNode(it.key(), it.value()));

    while (nodes.size() > 1) {
        std::sort(nodes.begin(), nodes.end(),
                  [](const HuffNode* a, const HuffNode* b) { return a->freq < b->freq; });

        HuffNode* left = nodes.takeFirst();
        HuffNode* right = nodes.takeFirst();
        HuffNode* parent = new HuffNode(0, left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        nodes.append(parent);
    }
    return nodes.isEmpty() ? nullptr : nodes.first();
}

void HuffmanCodec::buildCodes(HuffNode* node, const QByteArray& prefix,
                                QMap<quint8, QByteArray>& codes)
{
    if (!node) return;
    if (!node->left && !node->right) {
        codes[node->byte] = prefix.isEmpty() ? QByteArray("0") : prefix;
        return;
    }
    buildCodes(node->left, prefix + "0", codes);
    buildCodes(node->right, prefix + "1", codes);
}

void HuffmanCodec::deleteTree(HuffNode* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

void HuffmanCodec::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
