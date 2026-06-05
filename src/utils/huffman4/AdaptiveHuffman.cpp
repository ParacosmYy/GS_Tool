/**
 * @file AdaptiveHuffmanV2.cpp
 * @brief 自适应Huffman编码实现(FGK算法)
 */

#include "AdaptiveHuffmanV2.h"
#include <QElapsedTimer>
#include <cstring>

AdaptiveHuffmanV2::AdaptiveHuffmanV2(int alphabetSize, QObject* parent)
    : QObject(parent)
    , m_alphabetSize(alphabetSize)
    , m_timeSum(0.0)
{
    resetTree();
}

QByteArray AdaptiveHuffmanV2::encode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    resetTree();
    m_bitBuffer.clear();
    m_bitPos = 0;

    for (int i = 0; i < data.size(); ++i) {
        quint8 symbol = static_cast<quint8>(data[i]);

        if (m_symbolNodes.contains(symbol)) {
            /* 已有符号: 输出其编码 */
            writeCode(m_symbolNodes[symbol]);
        } else {
            /* 新符号: 输出NYT编码 + 符号值 */
            writeCode(m_nyt);
            for (int b = 7; b >= 0; --b)
                writeBit((symbol >> b) & 1);
        }

        /* 更新树 */
        Node* node = m_symbolNodes.value(symbol, nullptr);
        if (!node) {
            /* 创建新节点: NYT -> 内部节点 + 新叶节点 + 新NYT */
            Node* internal = createNode(-1, 1, m_nyt->order - 1, m_nyt->parent);
            Node* leaf = createNode(symbol, 1, m_nyt->order - 2, internal);

            internal->left = m_nyt;
            internal->right = leaf;
            m_nyt->parent = internal;
            leaf->parent = internal;

            if (internal->parent) {
                if (internal->parent->left == m_nyt)
                    internal->parent->left = internal;
                else
                    internal->parent->right = internal;
            } else {
                m_root = internal;
            }

            m_symbolNodes[symbol] = leaf;
            updateTree(internal);
        } else {
            updateTree(node);
        }
    }

    m_stats.totalEncoded++;
    m_stats.totalBytesIn += data.size();
    m_stats.totalBytesOut += m_bitBuffer.size();
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit encodingCompleted(data.size(), m_bitBuffer.size());
    return m_bitBuffer;
}

QByteArray AdaptiveHuffmanV2::decode(const QByteArray& data, int originalSize)
{
    QElapsedTimer timer;
    timer.start();

    resetTree();
    QByteArray result;
    m_readBitPos = 0;
    int decoded = 0;

    while (decoded < originalSize && m_readBitPos < data.size() * 8) {
        Node* current = m_root;

        /* 沿树遍历 */
        while (current->left || current->right) {
            int bit = readBit(data);
            if (bit < 0) break;
            current = (bit == 0) ? current->left : current->right;
        }

        int symbol;
        if (current == m_nyt || (!current->left && !current->right && current->symbol < 0)) {
            /* NYT: 读取8位符号 */
            symbol = 0;
            for (int b = 0; b < 8; ++b) {
                int bit = readBit(data);
                if (bit < 0) break;
                symbol = (symbol << 1) | bit;
            }
        } else {
            symbol = current->symbol;
        }

        result.append(static_cast<char>(symbol));
        decoded++;

        /* 更新树(与编码相同) */
        Node* node = m_symbolNodes.value(symbol, nullptr);
        if (!node) {
            Node* internal = createNode(-1, 1, m_nyt->order - 1, m_nyt->parent);
            Node* leaf = createNode(symbol, 1, m_nyt->order - 2, internal);
            internal->left = m_nyt;
            internal->right = leaf;
            m_nyt->parent = internal;
            leaf->parent = internal;
            if (internal->parent) {
                if (internal->parent->left == m_nyt)
                    internal->parent->left = internal;
                else
                    internal->parent->right = internal;
            } else {
                m_root = internal;
            }
            m_symbolNodes[symbol] = leaf;
            updateTree(internal);
        } else {
            updateTree(node);
        }
    }

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

double AdaptiveHuffmanV2::compressionRatio() const
{
    if (m_stats.totalBytesOut == 0) return 0.0;
    return static_cast<double>(m_stats.totalBytesOut) / m_stats.totalBytesIn;
}

AdaptiveHuffmanV2::Node* AdaptiveHuffmanV2::createNode(int symbol, int weight,
                                                     int order, Node* parent)
{
    Node* n = new Node();
    n->symbol = symbol;
    n->weight = weight;
    n->order = order;
    n->parent = parent;
    n->left = nullptr;
    n->right = nullptr;
    return n;
}

void AdaptiveHuffmanV2::updateTree(Node* node)
{
    while (node) {
        Node* leader = findLeader(node);
        if (leader != node && leader != node->parent) {
            swapNodes(node, leader);
        }
        node->weight++;
        node = node->parent;
    }
}

AdaptiveHuffmanV2::Node* AdaptiveHuffmanV2::findLeader(Node* node)
{
    /* 简化: 在同权重节点中找最高order */
    Node* leader = node;
    /* 遍历整棵树 */
    QVector<Node*> stack;
    stack.push_back(m_root);
    while (!stack.isEmpty()) {
        Node* cur = stack.takeLast();
        if (cur->weight == node->weight && cur->order > leader->order)
            leader = cur;
        if (cur->right) stack.push_back(cur->right);
        if (cur->left) stack.push_back(cur->left);
    }
    return leader;
}

void AdaptiveHuffmanV2::swapNodes(Node* a, Node* b)
{
    std::swap(a->order, b->order);
    if (a->parent) {
        if (a->parent->left == a) a->parent->left = b;
        else a->parent->right = b;
    }
    if (b->parent) {
        if (b->parent->left == b) b->parent->left = a;
        else b->parent->right = a;
    }
    std::swap(a->parent, b->parent);
    if (!a->parent) m_root = a;
    if (!b->parent) m_root = b;
}

void AdaptiveHuffmanV2::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

void AdaptiveHuffmanV2::resetTree()
{
    deleteTree(m_root);
    m_symbolNodes.clear();
    m_root = createNode(-1, 0, 2 * m_alphabetSize, nullptr);
    m_nyt = m_root;
}

void AdaptiveHuffmanV2::writeBit(int bit)
{
    if (m_bitPos == 0)
        m_bitBuffer.append(static_cast<char>(0));
    if (bit)
        m_bitBuffer[m_bitBuffer.size() - 1] |= (1 << (7 - m_bitPos));
    m_bitPos = (m_bitPos + 1) % 8;
}

void AdaptiveHuffmanV2::writeCode(Node* node)
{
    if (!node || !node->parent) return;
    QVector<int> bits;
    Node* cur = node;
    while (cur->parent) {
        bits.prepend(cur->parent->left == cur ? 0 : 1);
        cur = cur->parent;
    }
    for (int b : bits) writeBit(b);
}

int AdaptiveHuffmanV2::readBit(const QByteArray& data)
{
    int byteIdx = m_readBitPos / 8;
    int bitIdx = 7 - (m_readBitPos % 8);
    m_readBitPos++;
    if (byteIdx >= data.size()) return -1;
    return (static_cast<quint8>(data[byteIdx]) >> bitIdx) & 1;
}

AdaptiveHuffmanV2::Stats AdaptiveHuffmanV2::stats() const { return m_stats; }

void AdaptiveHuffmanV2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
