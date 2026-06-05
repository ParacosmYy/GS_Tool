/**
 * @file HuffmanTreeBuilder.cpp
 * @brief 通用霍夫曼树构建器实现
 */

#include "utils/shannon2/HuffmanTreeBuilder.h"

#include <QElapsedTimer>
#include <QQueue>
#include <algorithm>
#include <queue>

HuffmanTreeBuilder::HuffmanTreeBuilder(QObject* parent)
    : QObject(parent), m_root(nullptr), m_timeSum(0.0) {}

HuffmanTreeBuilder::~HuffmanTreeBuilder()
{
    if (m_root) deleteTree(m_root);
}

bool HuffmanTreeBuilder::buildFromFrequency(const QMap<int, quint64>& freq)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root) {
        deleteTree(m_root);
        m_root = nullptr;
    }
    m_codeTable.clear();

    if (freq.size() < 2) return false;

    /* 创建叶节点列表 */
    QVector<HuffNode*> nodes;
    for (auto it = freq.constBegin(); it != freq.constEnd(); ++it) {
        HuffNode* n = new HuffNode{it.key(), it.value(), nullptr, nullptr};
        nodes.append(n);
    }

    /* 按频率排序，每次取最小的两个合并 */
    auto cmp = [](HuffNode* a, HuffNode* b) { return a->freq > b->freq; };
    std::priority_queue<HuffNode*, QVector<HuffNode*>, decltype(cmp)> pq(cmp, nodes);

    while (pq.size() > 1) {
        HuffNode* left = pq.top(); pq.pop();
        HuffNode* right = pq.top(); pq.pop();
        HuffNode* parent = new HuffNode{-1, left->freq + right->freq, left, right};
        pq.push(parent);
    }

    m_root = pq.top();

    /* 从树生成编码表: BFS遍历 */
    struct Frame {
        HuffNode* node;
        quint32   code;
        int       length;
    };
    QQueue<Frame> queue;
    queue.enqueue({m_root, 0, 0});

    while (!queue.isEmpty()) {
        Frame f = queue.dequeue();
        if (f.node->symbol >= 0) {
            /* 叶节点 */
            m_codeTable[f.node->symbol] = {f.code, f.length};
        } else {
            if (f.node->left)  queue.enqueue({f.node->left,  (f.code << 1),     f.length + 1});
            if (f.node->right) queue.enqueue({f.node->right, (f.code << 1) | 1, f.length + 1});
        }
    }

    int maxDepth = computeDepth(m_root, 0);
    m_stats.totalTreesBuilt++;
    m_stats.maxTreeDepth = maxDepth;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalTreesBuilt, 1ULL);

    emit treeBuilt(m_codeTable.size(), maxDepth);
    return true;
}

QMap<int, HuffmanTreeBuilder::CodeEntry> HuffmanTreeBuilder::canonicalCodes() const
{
    /* 规范编码: 按码长排序，同码长内按符号值排序 */
    QMap<int, CodeEntry> result;

    /* 按码长分组 */
    QMap<int, QVector<int>> byLength;
    for (auto it = m_codeTable.constBegin(); it != m_codeTable.constEnd(); ++it) {
        byLength[it->length].append(it.key());
    }

    quint32 code = 0;
    int prevLen = 0;
    for (auto it = byLength.constBegin(); it != byLength.constEnd(); ++it) {
        int len = it.key();
        code <<= (len - prevLen);
        QVector<int> syms = it.value();
        std::sort(syms.begin(), syms.end());
        for (int sym : syms) {
            result[sym] = {code, len};
            ++code;
        }
        prevLen = len;
    }
    return result;
}

QByteArray HuffmanTreeBuilder::encodeSymbols(const QVector<int>& symbols) const
{
    QByteArray result;
    if (m_codeTable.isEmpty()) return result;

    unsigned char currentByte = 0;
    int bitPos = 7;

    for (int sym : symbols) {
        auto it = m_codeTable.constFind(sym);
        if (it == m_codeTable.constEnd()) continue;

        quint32 code = it->code;
        int len = it->length;
        for (int i = len - 1; i >= 0; --i) {
            if ((code >> i) & 1) currentByte |= (1 << bitPos);
            --bitPos;
            if (bitPos < 0) {
                result.append(static_cast<char>(currentByte));
                currentByte = 0;
                bitPos = 7;
            }
        }
    }

    /* 刷新剩余位 */
    if (bitPos < 7) result.append(static_cast<char>(currentByte));

    return result;
}

QVector<int> HuffmanTreeBuilder::decodeSymbols(const QByteArray& data, int count) const
{
    QVector<int> result;
    if (!m_root || data.isEmpty()) return result;

    HuffNode* node = m_root;
    int decoded = 0;

    for (int i = 0; i < data.size() && decoded < count; ++i) {
        unsigned char byte = static_cast<unsigned char>(data[i]);
        for (int b = 7; b >= 0 && decoded < count; --b) {
            int bit = (byte >> b) & 1;
            node = bit ? node->right : node->left;
            if (!node) {
                node = m_root;
                continue;
            }
            if (node->symbol >= 0) {
                result.append(node->symbol);
                ++decoded;
                node = m_root;
            }
        }
    }

    return result;
}

void HuffmanTreeBuilder::deleteTree(HuffNode* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

int HuffmanTreeBuilder::computeDepth(HuffNode* node, int depth)
{
    if (!node) return depth;
    int left  = computeDepth(node->left, depth + 1);
    int right = computeDepth(node->right, depth + 1);
    return std::max(left, right);
}

void HuffmanTreeBuilder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
