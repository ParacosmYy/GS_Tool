/**
 * @file ShannonCoding.cpp
 * @brief Shannon-Fano编码实现 — 含Huffman对比
 */

#include "ShannonCoding.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>

/* ---------- 构造函数 ---------- */

ShannonCoding::ShannonCoding(QObject* parent)
    : QObject(parent)
{
}

/* ---------- Shannon-Fano编码 ---------- */

ShannonCoding::EncodingResult ShannonCoding::encodeShannon(
    const QString& data) const
{
    QElapsedTimer timer;
    timer.start();

    EncodingResult result;
    if (data.isEmpty()) {
        m_stats.totalEncodings++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalEncodings > 0)
            ? m_timeSum / m_stats.totalEncodings : 0.0;
        return result;
    }

    auto probs = calculateFrequencies(data);
    auto codeTable = buildShannonCode(probs);
    result = performEncoding(data, codeTable, probs);

    m_stats.totalEncodings++;
    m_stats.totalSymbols += data.size();
    m_stats.totalBitsEncoded += result.encodedBits;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodings > 0)
        ? m_timeSum / m_stats.totalEncodings : 0.0;

    emit encodingCompleted(QStringLiteral("Shannon-Fano"),
                           data.size(), result.compressionRatio);
    return result;
}

/* ---------- Huffman编码 ---------- */

ShannonCoding::EncodingResult ShannonCoding::encodeHuffman(
    const QString& data) const
{
    QElapsedTimer timer;
    timer.start();

    EncodingResult result;
    if (data.isEmpty()) {
        m_stats.totalEncodings++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalEncodings > 0)
            ? m_timeSum / m_stats.totalEncodings : 0.0;
        return result;
    }

    auto probs = calculateFrequencies(data);
    auto codeTable = buildHuffmanCode(probs);
    result = performEncoding(data, codeTable, probs);

    m_stats.totalEncodings++;
    m_stats.totalSymbols += data.size();
    m_stats.totalBitsEncoded += result.encodedBits;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodings > 0)
        ? m_timeSum / m_stats.totalEncodings : 0.0;

    emit encodingCompleted(QStringLiteral("Huffman"),
                           data.size(), result.compressionRatio);
    return result;
}

/* ---------- 对比 ---------- */

ShannonCoding::ComparisonResult ShannonCoding::compare(
    const QString& data) const
{
    ComparisonResult cmp;
    cmp.shannon = encodeShannon(data);
    cmp.huffman = encodeHuffman(data);

    cmp.shannonCodeLength = 0.0;
    cmp.huffmanCodeLength = 0.0;
    for (auto it = cmp.shannon.codeTable.begin();
         it != cmp.shannon.codeTable.end(); ++it) {
        Q_UNUSED(it);
    }

    /* 计算加权平均码长 */
    auto probs = calculateFrequencies(data);
    auto shannonCodes = buildShannonCode(probs);
    auto huffmanCodes = buildHuffmanCode(probs);

    for (const auto& entry : shannonCodes) {
        cmp.shannonCodeLength += entry.probability * entry.codeLength;
    }
    for (const auto& entry : huffmanCodes) {
        cmp.huffmanCodeLength += entry.probability * entry.codeLength;
    }

    cmp.lengthDifference = cmp.shannonCodeLength - cmp.huffmanCodeLength;
    return cmp;
}

/* ---------- 构建Shannon-Fano编码表 ---------- */

QVector<ShannonCoding::CodeEntry> ShannonCoding::buildShannonCode(
    const QMap<QChar, double>& probabilities) const
{
    QVector<CodeEntry> entries;
    for (auto it = probabilities.begin(); it != probabilities.end(); ++it) {
        entries.append({it.key(), it.value(), QString(), 0});
    }

    /* 按概率降序排列 */
    std::sort(entries.begin(), entries.end(),
              [](const CodeEntry& a, const CodeEntry& b) {
                  return a.probability > b.probability;
              });

    if (entries.size() > 1) {
        shannonSplit(entries, 0, entries.size() - 1);
    } else if (entries.size() == 1) {
        entries[0].code = QStringLiteral("0");
        entries[0].codeLength = 1;
    }

    return entries;
}

/* ---------- 构建Huffman编码表 ---------- */

QVector<ShannonCoding::CodeEntry> ShannonCoding::buildHuffmanCode(
    const QMap<QChar, double>& probabilities) const
{
    QVector<CodeEntry> entries;

    if (probabilities.size() == 0) return entries;
    if (probabilities.size() == 1) {
        auto it = probabilities.begin();
        entries.append({it.key(), it.value(),
                        QStringLiteral("0"), 1});
        return entries;
    }

    HuffNode* root = buildHuffmanTree(probabilities);
    if (root == nullptr) return entries;

    generateHuffCodes(root, QString(), entries);
    freeHuffTree(root);

    return entries;
}

/* ---------- 计算信源熵 ---------- */

double ShannonCoding::calculateEntropy(
    const QMap<QChar, double>& probabilities) const
{
    double entropy = 0.0;
    for (auto it = probabilities.begin(); it != probabilities.end(); ++it) {
        double p = it.value();
        if (p > 0.0) {
            entropy -= p * qLn(p) / qLn(2.0);
        }
    }
    return entropy;
}

/* ---------- 统计符号频率 ---------- */

QMap<QChar, double> ShannonCoding::calculateFrequencies(
    const QString& data) const
{
    QMap<QChar, int> counts;
    int total = data.size();

    for (const QChar& ch : data) {
        counts[ch]++;
    }

    QMap<QChar, double> probs;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        probs[it.key()] = static_cast<double>(it.value()) / total;
    }
    return probs;
}

/* ---------- 统计 ---------- */

ShannonCoding::Stats ShannonCoding::stats() const { return m_stats; }

void ShannonCoding::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ---------- 私有: Shannon-Fano递归分表 ---------- */

void ShannonCoding::shannonSplit(QVector<CodeEntry>& entries,
                                  int start, int end) const
{
    if (start >= end) return;

    /* 计算从start到end的概率总和 */
    double totalProb = 0.0;
    for (int i = start; i <= end; ++i) {
        totalProb += entries[i].probability;
    }

    /* 找到最佳分割点: 使左右概率尽可能相等 */
    double leftSum = 0.0;
    int splitPoint = start;
    double minDiff = totalProb;

    for (int i = start; i < end; ++i) {
        leftSum += entries[i].probability;
        double diff = qAbs(totalProb - 2.0 * leftSum);
        if (diff < minDiff) {
            minDiff = diff;
            splitPoint = i;
        }
    }

    /* 左半部分编码前缀加'0' */
    for (int i = start; i <= splitPoint; ++i) {
        entries[i].code += QStringLiteral("0");
        entries[i].codeLength++;
    }

    /* 右半部分编码前缀加'1' */
    for (int i = splitPoint + 1; i <= end; ++i) {
        entries[i].code += QStringLiteral("1");
        entries[i].codeLength++;
    }

    /* 递归分割 */
    shannonSplit(entries, start, splitPoint);
    shannonSplit(entries, splitPoint + 1, end);
}

/* ---------- 私有: 构建Huffman树 ---------- */

ShannonCoding::HuffNode* ShannonCoding::buildHuffmanTree(
    const QMap<QChar, double>& probabilities) const
{
    /* 优先队列(最小堆): 按概率排序 */
    auto cmp = [](HuffNode* a, HuffNode* b) {
        return a->probability > b->probability;
    };
    std::priority_queue<HuffNode*, std::vector<HuffNode*>, decltype(cmp)> pq(cmp);

    for (auto it = probabilities.begin(); it != probabilities.end(); ++it) {
        pq.push(new HuffNode(it.key(), it.value()));
    }

    while (pq.size() > 1) {
        HuffNode* left = pq.top(); pq.pop();
        HuffNode* right = pq.top(); pq.pop();

        HuffNode* parent = new HuffNode(QChar(), left->probability + right->probability);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }

    return pq.empty() ? nullptr : pq.top();
}

/* ---------- 私有: 生成Huffman编码 ---------- */

void ShannonCoding::generateHuffCodes(HuffNode* node, const QString& prefix,
                                       QVector<CodeEntry>& entries) const
{
    if (node == nullptr) return;

    if (node->left == nullptr && node->right == nullptr) {
        /* 叶子节点 */
        entries.append({node->symbol, node->probability,
                        prefix.isEmpty() ? QStringLiteral("0") : prefix,
                        prefix.isEmpty() ? 1 : prefix.length()});
        return;
    }

    generateHuffCodes(node->left, prefix + QStringLiteral("0"), entries);
    generateHuffCodes(node->right, prefix + QStringLiteral("1"), entries);
}

/* ---------- 私有: 释放Huffman树 ---------- */

void ShannonCoding::freeHuffTree(HuffNode* node) const
{
    if (node == nullptr) return;
    freeHuffTree(node->left);
    freeHuffTree(node->right);
    delete node;
}

/* ---------- 私有: 执行编码 ---------- */

ShannonCoding::EncodingResult ShannonCoding::performEncoding(
    const QString& data, const QVector<CodeEntry>& codeTable,
    const QMap<QChar, double>& probs) const
{
    EncodingResult result;

    /* 构建快速查找表 */
    QMap<QChar, QString> lookup;
    for (const auto& entry : codeTable) {
        lookup[entry.symbol] = entry.code;
    }

    result.codeTable = lookup;
    result.originalBits = data.size() * 16; /* QChar = 16 bit */

    /* 编码 */
    QString bitString;
    bitString.reserve(data.size() * 8);
    for (const QChar& ch : data) {
        bitString += lookup.value(ch, QString());
    }
    result.encodedBits = bitString.length();

    /* 打包为QByteArray (8位一组) */
    QByteArray encoded;
    for (int i = 0; i < bitString.length(); i += 8) {
        quint8 byte = 0;
        for (int j = 0; j < 8 && (i + j) < bitString.length(); ++j) {
            if (bitString[i + j] == QLatin1Char('1')) {
                byte |= (1 << (7 - j));
            }
        }
        encoded.append(static_cast<char>(byte));
    }
    result.encodedData = encoded;

    result.compressionRatio = (result.originalBits > 0)
        ? static_cast<double>(result.encodedBits) / result.originalBits : 0.0;

    result.entropy = calculateEntropy(probs);

    /* 编码效率 = 熵 / 平均码长 */
    double avgCodeLen = 0.0;
    for (const auto& entry : codeTable) {
        avgCodeLen += entry.probability * entry.codeLength;
    }
    result.efficiency = (avgCodeLen > 0.0) ? result.entropy / avgCodeLen : 0.0;

    /* 冗余度 = 1 - 效率 */
    result.redundancy = 1.0 - result.efficiency;

    return result;
}
