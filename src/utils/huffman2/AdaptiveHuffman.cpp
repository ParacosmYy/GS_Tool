/**
 * @file AdaptiveHuffman.cpp
 * @brief 自适应霍夫曼编码器(FGK算法)实现
 */

#include "AdaptiveHuffman.h"

#include <QElapsedTimer>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

AdaptiveHuffman::AdaptiveHuffman(QObject* parent)
    : QObject(parent)
{
    initTree();
}

AdaptiveHuffman::~AdaptiveHuffman() = default;

// ═══════════════════════════════════════════════════════════
// 编码
// ═══════════════════════════════════════════════════════════

QByteArray AdaptiveHuffman::encode(const QByteArray& data)
{
    if (data.isEmpty()) {
        m_stats.totalEncodes += 1;
        emit encoded({}, 0.0);
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    // 重新初始化树
    initTree();

    QVector<bool> allBits;
    allBits.reserve(data.size() * 8);

    for (int i = 0; i < data.size(); ++i) {
        const int sym = static_cast<quint8>(data[i]);
        int nodeIdx = findSymbol(sym);

        if (nodeIdx < 0) {
            // 符号未出现: 输出NYT编码 + 8位原始值
            QVector<bool> nytCode = getCode(m_nytIndex);
            allBits.append(nytCode);
            for (int b = 7; b >= 0; --b) {
                allBits.append((sym >> b) & 1);
            }

            // 创建新节点: 将NYT拆分为内部节点+新叶节点+新NYT
            int nytPos = m_nytIndex;
            int internalIdx = m_tree.size();
            int leafIdx = internalIdx + 1;
            int newNytIdx = internalIdx + 2;

            // 内部节点继承NYT的位置
            m_tree[nytPos].symbol = -2; // 内部节点

            Node internalNode;
            internalNode.symbol = -2;
            internalNode.weight = 0;
            internalNode.order = m_nextOrder++;
            internalNode.parent = nytPos;
            internalNode.left = newNytIdx;
            internalNode.right = leafIdx;

            Node leafNode;
            leafNode.symbol = sym;
            leafNode.weight = 1;
            leafNode.order = m_nextOrder++;
            leafNode.parent = internalIdx;

            Node newNyt;
            newNyt.symbol = -1;
            newNyt.weight = 0;
            newNyt.order = m_nextOrder++;
            newNyt.parent = internalIdx;

            m_tree.append(internalNode);
            m_tree.append(leafNode);
            m_tree.append(newNyt);

            m_tree[nytPos].left = internalIdx;
            m_tree[nytPos].right = -1;

            // 实际上, 标准FGK: NYT节点变成内部节点, 左=NYT, 右=叶
            m_tree[nytPos].left = newNytIdx;
            m_tree[nytPos].right = leafIdx;

            m_symbolMap[sym] = leafIdx;
            m_nytIndex = newNytIdx;

            // 更新树
            updateTree(nytPos);
        } else {
            // 符号已存在: 输出编码
            QVector<bool> code = getCode(nodeIdx);
            allBits.append(code);
            updateTree(nodeIdx);
        }
    }

    QByteArray result = packBits(allBits);

    // 头部: 原始长度 + 比特数
    QByteArray header;
    header.append(static_cast<char>((data.size() >> 24) & 0xFF));
    header.append(static_cast<char>((data.size() >> 16) & 0xFF));
    header.append(static_cast<char>((data.size() >> 8) & 0xFF));
    header.append(static_cast<char>(data.size() & 0xFF));
    header.append(static_cast<char>((allBits.size() >> 24) & 0xFF));
    header.append(static_cast<char>((allBits.size() >> 16) & 0xFF));
    header.append(static_cast<char>((allBits.size() >> 8) & 0xFF));
    header.append(static_cast<char>(allBits.size() & 0xFF));

    result = header + result;

    m_stats.totalEncodes += 1;
    m_stats.totalBitsOut += static_cast<quint64>(allBits.size());
    m_stats.treeNodes = m_tree.size();

    const double bps = static_cast<double>(allBits.size()) /
                       static_cast<double>(std::max(data.size(), qsizetype(1)));
    if (m_stats.totalEncodes == 1) {
        m_stats.avgBitsPerSymbol = bps;
    } else {
        m_stats.avgBitsPerSymbol += (bps - m_stats.avgBitsPerSymbol) /
                                    static_cast<double>(m_stats.totalEncodes);
    }

    emit encoded(result, bps);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 解码
// ═══════════════════════════════════════════════════════════

QByteArray AdaptiveHuffman::decode(const QByteArray& data, int originalSize)
{
    if (data.size() < 8 || originalSize <= 0) {
        emit error(tr("自适应霍夫曼解码错误: 数据不足或长度无效"));
        return {};
    }

    initTree();

    // 读取头部
    int bitCount = (static_cast<quint8>(data[4]) << 24) |
                   (static_cast<quint8>(data[5]) << 16) |
                   (static_cast<quint8>(data[6]) << 8) |
                   static_cast<quint8>(data[7]);

    QVector<bool> bits = unpackBits(data.mid(8), bitCount);

    QByteArray result;
    result.reserve(originalSize);

    int bitPos = 0;

    while (result.size() < originalSize && bitPos < bits.size()) {
        // 从根遍历树
        int nodeIdx = 0; // 根节点

        while (m_tree[nodeIdx].left >= 0 || m_tree[nodeIdx].right >= 0) {
            if (bitPos >= bits.size()) {
                emit error(tr("自适应霍夫曼解码错误: 比特流提前结束"));
                return result;
            }
            if (bits[bitPos]) {
                nodeIdx = m_tree[nodeIdx].right;
            } else {
                nodeIdx = m_tree[nodeIdx].left;
            }
            bitPos++;
            if (nodeIdx < 0 || nodeIdx >= m_tree.size()) {
                emit error(tr("自适应霍夫曼解码错误: 树遍历越界"));
                return result;
            }
        }

        int sym;
        if (m_tree[nodeIdx].symbol == -1) {
            // NYT: 读取8位原始值
            sym = 0;
            for (int b = 0; b < 8 && bitPos < bits.size(); ++b) {
                sym = (sym << 1) | (bits[bitPos] ? 1 : 0);
                bitPos++;
            }

            // 创建新节点(同编码逻辑)
            int internalIdx = m_tree.size();
            int leafIdx = internalIdx + 1;
            int newNytIdx = internalIdx + 2;
            int nytPos = m_nytIndex;

            m_tree[nytPos].symbol = -2;
            m_tree[nytPos].left = newNytIdx;
            m_tree[nytPos].right = leafIdx;

            Node internalNode;
            internalNode.symbol = -2;
            internalNode.weight = 0;
            internalNode.order = m_nextOrder++;
            internalNode.parent = nytPos;

            Node leafNode;
            leafNode.symbol = sym;
            leafNode.weight = 1;
            leafNode.order = m_nextOrder++;
            leafNode.parent = internalIdx;

            Node newNyt;
            newNyt.symbol = -1;
            newNyt.weight = 0;
            newNyt.order = m_nextOrder++;
            newNyt.parent = internalIdx;

            m_tree.append(internalNode);
            m_tree.append(leafNode);
            m_tree.append(newNyt);

            m_symbolMap[sym] = leafIdx;
            m_nytIndex = newNytIdx;

            updateTree(nytPos);
        } else {
            sym = m_tree[nodeIdx].symbol;
            updateTree(nodeIdx);
        }

        result.append(static_cast<char>(sym));
    }

    m_stats.totalDecodes += 1;
    m_stats.treeNodes = m_tree.size();
    emit decoded(result);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

AdaptiveHuffman::Stats AdaptiveHuffman::stats() const { return m_stats; }

void AdaptiveHuffman::resetStatistics() { m_stats = Stats{}; }

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

void AdaptiveHuffman::initTree()
{
    m_tree.clear();
    m_symbolMap.clear();
    m_nextOrder = 0;

    // 根节点 = NYT
    Node root;
    root.symbol = -1;
    root.weight = 0;
    root.order = m_nextOrder++;
    root.parent = -1;
    root.left = -1;
    root.right = -1;

    m_tree.append(root);
    m_nytIndex = 0;
}

int AdaptiveHuffman::findSymbol(int sym) const
{
    auto it = m_symbolMap.find(sym);
    return (it != m_symbolMap.end()) ? it.value() : -1;
}

int AdaptiveHuffman::findLeader(int nodeIdx) const
{
    const int w = m_tree[nodeIdx].weight;
    int best = nodeIdx;
    for (int i = 0; i < m_tree.size(); ++i) {
        if (m_tree[i].weight == w && m_tree[i].order > m_tree[best].order) {
            best = i;
        }
    }
    return best;
}

void AdaptiveHuffman::swapNodes(int a, int b)
{
    if (a == b) return;

    // 交换父节点的子指针
    int pa = m_tree[a].parent;
    int pb = m_tree[b].parent;

    if (pa >= 0) {
        if (m_tree[pa].left == a) m_tree[pa].left = b;
        else m_tree[pa].right = b;
    }
    if (pb >= 0) {
        if (m_tree[pb].left == b) m_tree[pb].left = a;
        else m_tree[pb].right = a;
    }

    m_tree[a].parent = pb;
    m_tree[b].parent = pa;

    // 更新符号映射
    if (m_tree[a].symbol >= 0) m_symbolMap[m_tree[a].symbol] = b;
    if (m_tree[b].symbol >= 0) m_symbolMap[m_tree[b].symbol] = a;
    if (a == m_nytIndex) m_nytIndex = b;
    else if (b == m_nytIndex) m_nytIndex = a;

    std::swap(m_tree[a], m_tree[b]);
}

void AdaptiveHuffman::updateTree(int nodeIdx)
{
    while (nodeIdx >= 0) {
        int leader = findLeader(nodeIdx);
        if (leader != nodeIdx) {
            swapNodes(nodeIdx, leader);
            nodeIdx = leader;
        }
        m_tree[nodeIdx].weight++;
        nodeIdx = m_tree[nodeIdx].parent;
    }
}

QVector<bool> AdaptiveHuffman::getCode(int nodeIdx) const
{
    QVector<bool> code;
    int cur = nodeIdx;
    while (m_tree[cur].parent >= 0) {
        int p = m_tree[cur].parent;
        code.prepend(m_tree[p].right == cur);
        cur = p;
    }
    return code;
}

QByteArray AdaptiveHuffman::packBits(const QVector<bool>& bits) const
{
    QByteArray result;
    int byteVal = 0;
    int bitPos = 0;
    for (bool b : bits) {
        byteVal = (byteVal << 1) | (b ? 1 : 0);
        bitPos++;
        if (bitPos == 8) {
            result.append(static_cast<char>(byteVal));
            byteVal = 0;
            bitPos = 0;
        }
    }
    if (bitPos > 0) {
        result.append(static_cast<char>(byteVal << (8 - bitPos)));
    }
    return result;
}

QVector<bool> AdaptiveHuffman::unpackBits(const QByteArray& data,
                                          int bitCount) const
{
    QVector<bool> result;
    result.reserve(bitCount);
    for (int i = 0; i < bitCount; ++i) {
        int byteIdx = i / 8;
        int bitIdx = 7 - (i % 8);
        if (byteIdx < data.size()) {
            result.append((static_cast<quint8>(data[byteIdx]) >> bitIdx) & 1);
        }
    }
    return result;
}
