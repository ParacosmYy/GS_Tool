/**
 * @file DataCompressorAlgo.cpp
 * @brief 数据压缩器算法实现 — RLE/Huffman/LZ77/Deflate + 辅助方法
 *
 * 从 DataCompressor.cpp 拆分而来。压缩/解压API入口和统计保留在主文件。
 */

#include "utils/compress/DataCompressor.h"

#include <QMap>
#include <QQueue>
#include <algorithm>
#include <cstring>

/** @brief 压缩数据魔数 */
static constexpr char MAGIC_0 = 'E';
static constexpr char MAGIC_1 = 'D';
static constexpr int HEADER_SIZE = 8;
static constexpr int LZ77_WINDOW_BASE = 4096;
static constexpr int LZ77_MAX_MATCH = 258;
static constexpr int RLE_MAX_RUN = 255;

// ── RLE ──

QByteArray DataCompressor::compressRle(const QByteArray& data)
{
    QByteArray result;
    result.reserve(data.size() / 2);
    int i = 0;
    while (i < data.size()) {
        char val = data[i];
        int run = 1;
        while (i + run < data.size() && data[i + run] == val && run < RLE_MAX_RUN) ++run;
        result.append(val);
        result.append(static_cast<char>(run));
        i += run;
    }
    return result;
}

QByteArray DataCompressor::decompressRle(const QByteArray& data)
{
    QByteArray result;
    for (int i = 0; i + 1 < data.size(); i += 2) {
        char val = data[i];
        quint8 count = static_cast<quint8>(data[i + 1]);
        result.append(count, val);
    }
    return result;
}

// ── Huffman ──

QByteArray DataCompressor::compressHuffman(const QByteArray& data)
{
    if (data.size() < 2) return data;
    QVector<quint32> freq = buildFreqTable(data);
    HuffNode* root = buildHuffmanTree(freq);
    if (!root) return {};

    QMap<int, QString> codeTable;
    QQueue<QPair<HuffNode*, QString>> queue;
    queue.enqueue({root, QString()});
    while (!queue.isEmpty()) {
        auto item = queue.dequeue();
        HuffNode* node = item.first;
        QString code = item.second;
        if (node->symbol >= 0) codeTable[node->symbol] = code.isEmpty() ? QStringLiteral("0") : code;
        if (node->left)  queue.enqueue({node->left, code + QLatin1Char('0')});
        if (node->right) queue.enqueue({node->right, code + QLatin1Char('1')});
    }

    QString bitString;
    bitString.reserve(data.size() * 6);
    for (char byte : data) {
        bitString.append(codeTable.value(static_cast<quint8>(byte), QStringLiteral("0")));
    }

    QByteArray payload;
    payload.reserve(1024 + bitString.size() / 8);
    for (int i = 0; i < 256; ++i) {
        quint32 f = freq[i];
        payload.append(static_cast<char>((f >> 24) & 0xFF));
        payload.append(static_cast<char>((f >> 16) & 0xFF));
        payload.append(static_cast<char>((f >> 8) & 0xFF));
        payload.append(static_cast<char>(f & 0xFF));
    }
    quint32 bitCount = static_cast<quint32>(bitString.size());
    payload.append(static_cast<char>((bitCount >> 24) & 0xFF));
    payload.append(static_cast<char>((bitCount >> 16) & 0xFF));
    payload.append(static_cast<char>((bitCount >> 8) & 0xFF));
    payload.append(static_cast<char>(bitCount & 0xFF));
    payload.append(bitsToBytes(bitString));

    freeHuffmanTree(root);
    return payload;
}

QByteArray DataCompressor::decompressHuffman(const QByteArray& data)
{
    const int freqTableSize = 256 * 4;
    if (data.size() < freqTableSize + 4) return {};

    QVector<quint32> freq(256, 0);
    for (int i = 0; i < 256; ++i) {
        freq[i] = (static_cast<quint32>(static_cast<quint8>(data[i * 4])) << 24) |
                  (static_cast<quint32>(static_cast<quint8>(data[i * 4 + 1])) << 16) |
                  (static_cast<quint32>(static_cast<quint8>(data[i * 4 + 2])) << 8) |
                   static_cast<quint32>(static_cast<quint8>(data[i * 4 + 3]));
    }

    int offset = freqTableSize;
    quint32 bitCount = (static_cast<quint32>(static_cast<quint8>(data[offset])) << 24) |
                       (static_cast<quint32>(static_cast<quint8>(data[offset + 1])) << 16) |
                       (static_cast<quint32>(static_cast<quint8>(data[offset + 2])) << 8) |
                        static_cast<quint32>(static_cast<quint8>(data[offset + 3]));
    offset += 4;

    HuffNode* root = buildHuffmanTree(freq);
    if (!root) return {};

    if (!root->left && !root->right) {
        QByteArray result;
        quint32 totalFreq = 0;
        for (int i = 0; i < 256; ++i) totalFreq += freq[i];
        for (int i = 0; i < 256; ++i) {
            if (freq[i] > 0) { result.append(static_cast<int>(totalFreq), static_cast<char>(i)); break; }
        }
        freeHuffmanTree(root);
        return result;
    }

    QByteArray encodedData = data.mid(offset);
    QString bitString = bytesToBits(encodedData, static_cast<int>(bitCount));
    QByteArray result;
    HuffNode* current = root;
    for (int i = 0; i < bitString.size(); ++i) {
        current = (bitString[i] == QLatin1Char('0')) ? current->left : current->right;
        if (!current) { freeHuffmanTree(root); return {}; }
        if (current->symbol >= 0) { result.append(static_cast<char>(current->symbol)); current = root; }
    }
    freeHuffmanTree(root);
    return result;
}

// ── LZ77 ──

QByteArray DataCompressor::compressLz77(const QByteArray& data)
{
    QByteArray result;
    result.reserve(data.size());
    int windowSize = LZ77_WINDOW_BASE * m_level / 6;
    int minMatch = (m_level >= 7) ? 2 : 3;
    int pos = 0;
    while (pos < data.size()) {
        int searchStart = qMax(0, pos - windowSize);
        LzMatch match = findLzMatch(data, pos, pos - searchStart);
        if (match.length >= minMatch) {
            quint16 off = static_cast<quint16>(match.offset);
            quint16 len = static_cast<quint16>(match.length);
            result.append(static_cast<char>((off >> 8) & 0xFF));
            result.append(static_cast<char>(off & 0xFF));
            result.append(static_cast<char>((len >> 8) & 0xFF));
            result.append(static_cast<char>(len & 0xFF));
            result.append(static_cast<char>(0x00));
            pos += match.length;
        } else {
            result.append(static_cast<char>(0x00));
            result.append(static_cast<char>(0x00));
            result.append(static_cast<char>(0x00));
            result.append(static_cast<char>(0x01));
            result.append(data[pos]);
            ++pos;
        }
    }
    return result;
}

QByteArray DataCompressor::decompressLz77(const QByteArray& data)
{
    QByteArray result;
    int i = 0;
    while (i + 4 < data.size()) {
        quint16 off = (static_cast<quint8>(data[i]) << 8) | static_cast<quint8>(data[i + 1]);
        quint16 len = (static_cast<quint8>(data[i + 2]) << 8) | static_cast<quint8>(data[i + 3]);
        quint8  flag = static_cast<quint8>(data[i + 4]);
        i += 5;
        if (flag == 0x00 && off > 0 && len > 0) {
            int startPos = result.size() - static_cast<int>(off);
            if (startPos < 0) return {};
            for (int j = 0; j < static_cast<int>(len); ++j) result.append(result[startPos + j]);
        } else {
            result.append(data[i - 1]);
        }
    }
    return result;
}

// ── Deflate ──

QByteArray DataCompressor::compressDeflate(const QByteArray& data)
{
    QByteArray lzData = compressLz77(data);
    if (lzData.isEmpty()) return {};
    return compressHuffman(lzData);
}

QByteArray DataCompressor::decompressDeflate(const QByteArray& data)
{
    QByteArray lzData = decompressHuffman(data);
    if (lzData.isEmpty()) return {};
    return decompressLz77(lzData);
}

// ── 辅助方法 ──

QVector<quint32> DataCompressor::buildFreqTable(const QByteArray& data) const
{
    QVector<quint32> freq(256, 0);
    for (char byte : data) ++freq[static_cast<quint8>(byte)];
    return freq;
}

DataCompressor::HuffNode* DataCompressor::buildHuffmanTree(const QVector<quint32>& freq)
{
    QList<HuffNode*> nodes;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            auto* node = new HuffNode;
            node->symbol = i;
            node->freq = freq[i];
            nodes.append(node);
        }
    }
    if (nodes.isEmpty()) return nullptr;
    if (nodes.size() == 1) return nodes.first();

    std::sort(nodes.begin(), nodes.end(), [](const HuffNode* a, const HuffNode* b) { return a->freq < b->freq; });
    while (nodes.size() > 1) {
        HuffNode* left = nodes.takeFirst();
        HuffNode* right = nodes.takeFirst();
        auto* parent = new HuffNode;
        parent->freq = left->freq + right->freq;
        parent->left = left;
        parent->right = right;
        int insertPos = 0;
        while (insertPos < nodes.size() && nodes[insertPos]->freq < parent->freq) ++insertPos;
        nodes.insert(insertPos, parent);
    }
    return nodes.first();
}

void DataCompressor::generateCodes(HuffNode* node, const QString& prefix)
{
    if (!node) return;
    if (node->symbol >= 0) { node->code = prefix.isEmpty() ? QStringLiteral("0") : prefix; return; }
    generateCodes(node->left, prefix + QLatin1Char('0'));
    generateCodes(node->right, prefix + QLatin1Char('1'));
}

void DataCompressor::freeHuffmanTree(HuffNode* node)
{
    if (!node) return;
    freeHuffmanTree(node->left);
    freeHuffmanTree(node->right);
    delete node;
}

QByteArray DataCompressor::bitsToBytes(const QString& bits) const
{
    QByteArray result;
    for (int i = 0; i < bits.size(); i += 8) {
        quint8 byte = 0;
        for (int j = 0; j < 8 && (i + j) < bits.size(); ++j)
            if (bits[i + j] == QLatin1Char('1')) byte |= (1 << (7 - j));
        result.append(static_cast<char>(byte));
    }
    return result;
}

QString DataCompressor::bytesToBits(const QByteArray& bytes, int bitCount) const
{
    QString result;
    result.reserve(bitCount);
    for (int i = 0; i < bitCount; ++i) {
        int byteIdx = i / 8;
        int bitIdx = 7 - (i % 8);
        if (byteIdx < bytes.size())
            result.append((static_cast<quint8>(bytes[byteIdx]) & (1 << bitIdx)) ? QLatin1Char('1') : QLatin1Char('0'));
    }
    return result;
}

DataCompressor::LzMatch DataCompressor::findLzMatch(const QByteArray& data, int pos, int windowSize) const
{
    LzMatch best;
    int searchStart = qMax(0, pos - windowSize);
    int maxLen = qMin(LZ77_MAX_MATCH, data.size() - pos);
    for (int i = searchStart; i < pos; ++i) {
        int len = 0;
        while (len < maxLen && data[i + (len % (pos - i))] == data[pos + len]) ++len;
        if (len > best.length) { best.length = len; best.offset = pos - i; }
    }
    return best;
}

double DataCompressor::calculateEntropy(const QByteArray& data) const
{
    if (data.isEmpty()) return 0.0;
    int freq[256] = {};
    for (char byte : data) ++freq[static_cast<quint8>(byte)];
    double entropy = 0.0;
    double total = static_cast<double>(data.size());
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) { double p = static_cast<double>(freq[i]) / total; entropy -= p * std::log2(p); }
    }
    return entropy;
}

QByteArray DataCompressor::writeHeader(Algorithm algo, quint32 originalSize) const
{
    QByteArray header;
    header.reserve(HEADER_SIZE);
    header.append(MAGIC_0);
    header.append(MAGIC_1);
    header.append(static_cast<char>(static_cast<int>(algo)));
    header.append(static_cast<char>((originalSize >> 24) & 0xFF));
    header.append(static_cast<char>((originalSize >> 16) & 0xFF));
    header.append(static_cast<char>((originalSize >> 8) & 0xFF));
    header.append(static_cast<char>(originalSize & 0xFF));
    header.append(static_cast<char>(m_level));
    return header;
}

int DataCompressor::readHeader(const QByteArray& data, Algorithm expectedAlgo) const
{
    if (data.size() < HEADER_SIZE) return -1;
    if (data[0] != MAGIC_0 || data[1] != MAGIC_1) return -1;
    if (static_cast<Algorithm>(static_cast<quint8>(data[2])) != expectedAlgo) return -1;
    quint32 originalSize = (static_cast<quint32>(static_cast<quint8>(data[3])) << 24) |
                           (static_cast<quint32>(static_cast<quint8>(data[4])) << 16) |
                           (static_cast<quint32>(static_cast<quint8>(data[5])) << 8) |
                            static_cast<quint32>(static_cast<quint8>(data[6]));
    return static_cast<int>(originalSize);
}
