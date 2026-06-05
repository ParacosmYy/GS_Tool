/**
 * @file CanonicalHuffman.cpp
 * @brief 规范霍夫曼编码实现
 */

#include "utils/huffman3/CanonicalHuffman.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <deque>

/** @brief 构造函数 @param parent 父对象 */
CanonicalHuffman::CanonicalHuffman(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 根据频率构建码表
 *  @param frequencies 符号→频率映射 */
void CanonicalHuffman::buildCodeTable(const QMap<int, double>& frequencies)
{
    if (frequencies.size() < 2) return;

    /* 按频率排序的符号列表 */
    struct SymbolFreq {
        int symbol;
        double freq;
    };
    std::deque<SymbolFreq> symbols;
    for (auto it = frequencies.begin(); it != frequencies.end(); ++it) {
        symbols.push_back({it.key(), it.value()});
    }
    std::sort(symbols.begin(), symbols.end(),
              [](const SymbolFreq& a, const SymbolFreq& b) {
                  return a.freq > b.freq; /* 降序: 频率高的在前 */
              });

    /* 使用简化方法分配码长: 按频率分层 */
    int n = static_cast<int>(symbols.size());

    /* 构建霍夫曼树获取码长 */
    struct TreeNode {
        double weight;
        int symbol;  /* -1=内部节点 */
        int left, right; /* 索引 */
    };
    std::deque<TreeNode> nodes;

    /* 创建叶子节点 */
    for (int i = 0; i < n; ++i) {
        nodes.push_back({symbols[i].freq, symbols[i].symbol, -1, -1});
    }

    /* 逐步合并 */
    while (nodes.size() > 1) {
        /* 找最小的两个 */
        int minIdx1 = 0, minIdx2 = 1;
        if (nodes[1].weight < nodes[0].weight) std::swap(minIdx1, minIdx2);
        for (int i = 2; i < static_cast<int>(nodes.size()); ++i) {
            if (nodes[i].weight < nodes[minIdx1].weight) {
                minIdx2 = minIdx1;
                minIdx1 = i;
            } else if (nodes[i].weight < nodes[minIdx2].weight) {
                minIdx2 = i;
            }
        }

        /* 确保minIdx1 < minIdx2以便安全删除 */
        if (minIdx1 > minIdx2) std::swap(minIdx1, minIdx2);

        TreeNode merged;
        merged.weight = nodes[minIdx1].weight + nodes[minIdx2].weight;
        merged.symbol = -1;
        merged.left = minIdx1;
        merged.right = minIdx2;

        /* 移除两个节点并加入合并节点 */
        nodes.erase(nodes.begin() + minIdx2);
        nodes.erase(nodes.begin() + minIdx1);
        nodes.push_back(merged);
    }

    /* 递归计算码长 */
    m_codeLengths.clear();
    if (nodes.empty()) return;

    struct StackEntry {
        int nodeIdx;
        int depth;
    };
    std::deque<StackEntry> stack;
    stack.push_back({0, 0});

    while (!stack.empty()) {
        StackEntry entry = stack.back();
        stack.pop_back();
        const TreeNode& node = nodes[entry.nodeIdx];
        if (node.symbol >= 0) {
            m_codeLengths[node.symbol] = entry.depth;
        } else {
            if (node.left >= 0) stack.push_back({node.left, entry.depth + 1});
            if (node.right >= 0) stack.push_back({node.right, entry.depth + 1});
        }
    }

    buildCanonicalCodes();
}

/** @brief 编码数据
 *  @param data 待编码数据
 *  @return 编码后的比特流 */
QByteArray CanonicalHuffman::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    int currentBit = 0;
    unsigned char currentByte = 0;

    for (int sym : data) {
        QByteArray code = m_codeWords.value(sym, QByteArray());
        for (int i = 0; i < code.size(); ++i) {
            if (code[i] == '1') {
                currentByte |= (1 << (7 - currentBit));
            }
            ++currentBit;
            if (currentBit == 8) {
                result.append(static_cast<char>(currentByte));
                currentByte = 0;
                currentBit = 0;
            }
        }
    }

    /* 剩余比特 */
    if (currentBit > 0) {
        result.append(static_cast<char>(currentByte));
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalEncodes;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit encodeCompleted(data.size(), result.size() * 8);
    return result;
}

/** @brief 解码数据
 *  @param encoded 编码后的比特流
 *  @param symbolCount 原始符号数量
 *  @return 解码后的符号列表 */
QVector<int> CanonicalHuffman::decode(const QByteArray& encoded, int symbolCount)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    QByteArray currentCode;
    int totalBits = encoded.size() * 8;

    for (int bitPos = 0; bitPos < totalBits && result.size() < symbolCount; ++bitPos) {
        int byteIdx = bitPos / 8;
        int bitIdx = 7 - (bitPos % 8);
        bool bit = (static_cast<unsigned char>(encoded[byteIdx]) >> bitIdx) & 1;
        currentCode.append(bit ? '1' : '0');

        if (m_decodeMap.contains(currentCode)) {
            result.append(m_decodeMap[currentCode]);
            currentCode.clear();
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDecodes;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit decodeCompleted(result.size());
    return result;
}

/** @brief 获取符号的码字 */
QByteArray CanonicalHuffman::codeWord(int symbol) const
{
    return m_codeWords.value(symbol, QByteArray());
}

/** @brief 重置统计 */
void CanonicalHuffman::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 从码长生成规范码 */
void CanonicalHuffman::buildCanonicalCodes()
{
    /* 按码长排序符号 */
    std::deque<std::pair<int, int>> sortedByLength; /* (symbol, codeLength) */
    for (auto it = m_codeLengths.begin(); it != m_codeLengths.end(); ++it) {
        sortedByLength.push_back({it.key(), it.value()});
    }
    std::sort(sortedByLength.begin(), sortedByLength.end(),
              [](const auto& a, const auto& b) {
                  return (a.second == b.second) ? (a.first < b.first) : (a.second < b.second);
              });

    m_codeWords.clear();
    m_decodeMap.clear();

    if (sortedByLength.empty()) return;

    /* 第一个符号: 全0 */
    int prevLen = sortedByLength[0].second;
    QByteArray currentCode(prevLen, '0');
    m_codeWords[sortedByLength[0].first] = currentCode;
    m_decodeMap[currentCode] = sortedByLength[0].first;

    /* 后续符号: 递增 */
    for (int i = 1; i < static_cast<int>(sortedByLength.size()); ++i) {
        int symbol = sortedByLength[i].first;
        int codeLen = sortedByLength[i].second;

        /* 当前码+1 */
        /* 将码字视为二进制数并+1 */
        int carry = 1;
        for (int j = currentCode.size() - 1; j >= 0 && carry; --j) {
            if (currentCode[j] == '0') {
                currentCode[j] = '1';
                carry = 0;
            } else {
                currentCode[j] = '0';
            }
        }

        /* 如果码长增加，追加0 */
        while (static_cast<int>(currentCode.size()) < codeLen) {
            currentCode.append('0');
        }

        m_codeWords[symbol] = currentCode;
        m_decodeMap[currentCode] = symbol;
        prevLen = codeLen;
    }
}
