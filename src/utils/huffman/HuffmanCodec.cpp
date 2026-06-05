/**
 * @file HuffmanCodec.cpp
 * @brief 霍夫曼编解码器实现
 */

#include "HuffmanCodec.h"

#include <QElapsedTimer>
#include <QDataStream>
#include <QIODevice>
#include <algorithm>
#include <queue>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

HuffmanCodec::HuffmanCodec(QObject* parent)
    : QObject(parent)
{
}

HuffmanCodec::~HuffmanCodec() = default;

// ═══════════════════════════════════════════════════════════
// 编码
// ═══════════════════════════════════════════════════════════

QByteArray HuffmanCodec::encode(const QByteArray& data)
{
    if (data.isEmpty()) {
        m_stats.totalEncodes += 1;
        emit encoded({}, 0.0);
        return {};
    }

    // 1. 构建频率表
    QVector<quint32> freq = buildFreqTable(data);

    // 2. 构建霍夫曼树
    HuffNode* root = buildTree(freq);
    if (!root) {
        emit error(tr("霍夫曼编码错误: 无法构建编码树"));
        m_stats.totalEncodes += 1;
        emit encoded(data, 1.0);
        return data;
    }

    // 3. 生成编码表
    m_codeTable.clear();
    generateCodes(root, QByteArray());

    // 4. 编码数据到位流
    QByteArray bitStream;
    bitStream.reserve(data.size());

    for (int i = 0; i < data.size(); ++i) {
        const int symbol = static_cast<quint8>(data[i]);
        bitStream.append(m_codeTable.value(symbol));
    }

    // 5. 位流转字节数组
    QByteArray packedBytes = bitsToPackedBytes(bitStream);

    // 6. 组装输出: 原始大小(4B) + 位流长度(4B) + 频率表 + 压缩数据
    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);
    stream << static_cast<quint32>(data.size());
    stream << static_cast<quint32>(bitStream.size());

    QByteArray freqData = serializeFreqTable(freq);
    stream << static_cast<quint32>(freqData.size());
    result.append(freqData);
    result.append(packedBytes);

    // 7. 释放树
    freeTree(root);

    // 更新统计
    m_stats.totalEncodes += 1;
    const double ratio = static_cast<double>(result.size()) /
                         static_cast<double>(data.size());
    updateAvgRatio(ratio);

    emit encoded(result, ratio);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 解码
// ═══════════════════════════════════════════════════════════

QByteArray HuffmanCodec::decode(const QByteArray& data)
{
    if (data.size() < 12) {
        emit error(tr("霍夫曼解码错误: 数据太短, 缺少头部"));
        return {};
    }

    // 1. 读取头部
    QDataStream stream(data);
    quint32 originalSize = 0;
    quint32 totalBits = 0;
    quint32 freqTableSize = 0;
    stream >> originalSize >> totalBits >> freqTableSize;

    // 2. 反序列化频率表
    int bytesRead = 0;
    const QByteArray freqRaw = data.mid(12,
                                        static_cast<int>(freqTableSize));
    QVector<quint32> freq = deserializeFreqTable(freqRaw, &bytesRead);

    // 3. 重建霍夫曼树
    HuffNode* root = buildTree(freq);
    if (!root) {
        emit error(tr("霍夫曼解码错误: 无法重建编码树"));
        return {};
    }

    // 4. 解码位流
    const int headerSize = 12 + static_cast<int>(freqTableSize);
    const QByteArray packedData = data.mid(headerSize);
    QByteArray bitStream = packedBytesToBits(
        packedData, static_cast<int>(totalBits));

    QByteArray result;
    result.reserve(static_cast<int>(originalSize));

    HuffNode* current = root;
    for (int i = 0; i < bitStream.size(); ++i) {
        if (bitStream[i] == '0') {
            current = current->left;
        } else {
            current = current->right;
        }

        if (!current) {
            emit error(tr("霍夫曼解码错误: 位流损坏, 解码路径无效"));
            freeTree(root);
            return {};
        }

        if (current->symbol >= 0) {
            result.append(static_cast<char>(current->symbol));
            current = root;

            if (result.size() >= static_cast<int>(originalSize)) {
                break;
            }
        }
    }

    freeTree(root);

    m_stats.totalDecodes += 1;
    emit decoded(result);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

HuffmanCodec::Stats HuffmanCodec::stats() const
{
    return m_stats;
}

void HuffmanCodec::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部实现
// ═══════════════════════════════════════════════════════════

QVector<quint32> HuffmanCodec::buildFreqTable(
    const QByteArray& data) const
{
    QVector<quint32> freq(256, 0);
    for (int i = 0; i < data.size(); ++i) {
        freq[static_cast<quint8>(data[i])]++;
    }
    return freq;
}

HuffmanCodec::HuffNode* HuffmanCodec::buildTree(
    const QVector<quint32>& freq)
{
    auto cmp = [](const HuffNode* a, const HuffNode* b) {
        return a->freq > b->freq;
    };
    std::priority_queue<HuffNode*, std::vector<HuffNode*>,
                        decltype(cmp)> minHeap(cmp);

    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            auto* node = new HuffNode();
            node->symbol = i;
            node->freq = freq[i];
            minHeap.push(node);
        }
    }

    if (minHeap.size() == 1) {
        auto* leaf = minHeap.top();
        minHeap.pop();
        auto* root = new HuffNode();
        root->freq = leaf->freq;
        root->left = leaf;
        return root;
    }

    while (minHeap.size() > 1) {
        auto* left = minHeap.top();
        minHeap.pop();
        auto* right = minHeap.top();
        minHeap.pop();

        auto* parent = new HuffNode();
        parent->freq = left->freq + right->freq;
        parent->left = left;
        parent->right = right;

        minHeap.push(parent);
    }

    return minHeap.empty() ? nullptr : minHeap.top();
}

void HuffmanCodec::generateCodes(HuffNode* node, const QByteArray& code)
{
    if (!node) {
        return;
    }

    if (node->symbol >= 0) {
        m_codeTable[node->symbol] = code.isEmpty()
            ? QByteArray(1, '0') : code;
        return;
    }

    generateCodes(node->left, code + '0');
    generateCodes(node->right, code + '1');
}

void HuffmanCodec::freeTree(HuffNode* node)
{
    if (!node) {
        return;
    }
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

QByteArray HuffmanCodec::serializeFreqTable(
    const QVector<quint32>& freq) const
{
    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);

    quint16 count = 0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            ++count;
        }
    }

    stream << count;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            stream << static_cast<quint8>(i);
            stream << freq[i];
        }
    }

    return result;
}

QVector<quint32> HuffmanCodec::deserializeFreqTable(
    const QByteArray& data, int* bytesRead) const
{
    QVector<quint32> freq(256, 0);
    QDataStream stream(data);

    quint16 count = 0;
    stream >> count;

    for (quint16 i = 0; i < count; ++i) {
        quint8 symbol = 0;
        quint32 f = 0;
        stream >> symbol >> f;
        freq[symbol] = f;
    }

    if (bytesRead) {
        *bytesRead = static_cast<int>(
            2 + static_cast<quint64>(count) * 5);
    }

    return freq;
}

QByteArray HuffmanCodec::bitsToPackedBytes(
    const QByteArray& bits) const
{
    QByteArray result;
    const int totalBits = bits.size();
    const int fullBytes = totalBits / 8;

    for (int i = 0; i < fullBytes; ++i) {
        quint8 byte = 0;
        for (int b = 0; b < 8; ++b) {
            if (bits[i * 8 + b] == '1') {
                byte |= (1 << (7 - b));
            }
        }
        result.append(static_cast<char>(byte));
    }

    const int remain = totalBits % 8;
    if (remain > 0) {
        quint8 byte = 0;
        for (int b = 0; b < remain; ++b) {
            if (bits[fullBytes * 8 + b] == '1') {
                byte |= (1 << (7 - b));
            }
        }
        result.append(static_cast<char>(byte));
    }

    return result;
}

QByteArray HuffmanCodec::packedBytesToBits(
    const QByteArray& bytes, int totalBits) const
{
    QByteArray result;
    result.reserve(totalBits);

    for (int i = 0; i < bytes.size() && result.size() < totalBits; ++i) {
        const quint8 byte = static_cast<quint8>(bytes[i]);
        for (int b = 7; b >= 0 && result.size() < totalBits; --b) {
            result.append((byte & (1 << b)) ? '1' : '0');
        }
    }

    return result;
}

void HuffmanCodec::updateAvgRatio(double ratio)
{
    const auto n = m_stats.totalEncodes;
    if (n == 1) {
        m_stats.avgCompressionRatio = ratio;
    } else {
        m_stats.avgCompressionRatio =
            m_stats.avgCompressionRatio *
                static_cast<double>(n - 1) / static_cast<double>(n) +
            ratio / static_cast<double>(n);
    }
}
