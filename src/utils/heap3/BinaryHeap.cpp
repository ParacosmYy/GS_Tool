/**
 * @file BinaryHeap.cpp
 * @brief 通用二叉最小堆实现 — 支持decrease-key操作
 */

#include "BinaryHeap.h"

#include <QElapsedTimer>
#include <limits>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

BinaryHeap::BinaryHeap(QObject* parent)
    : QObject(parent)
{
}

BinaryHeap::~BinaryHeap() = default;

// ═══════════════════════════════════════════════════════════
// 插入
// ═══════════════════════════════════════════════════════════

BinaryHeap::Handle BinaryHeap::insert(double key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    /* 创建新节点 */
    int nodeId = m_nextHandle++;
    Node node{key, value, static_cast<int>(m_heap.size())};
    m_nodes.append(node);

    /* 加入堆尾 */
    m_heap.append(nodeId);

    /* 上浮 */
    siftUp(static_cast<int>(m_heap.size()) - 1);

    /* 更新统计 */
    m_stats.totalInserts++;
    const qint64 elapsed = timer.nsecsElapsed() / 1000000;
    const auto total = m_stats.totalInserts + m_stats.totalExtracts;
    if (total == 1) {
        m_stats.avgProcessingTimeMs = static_cast<double>(elapsed);
    } else {
        m_stats.avgProcessingTimeMs =
            m_stats.avgProcessingTimeMs * (total - 1) / total +
            static_cast<double>(elapsed) / total;
    }

    emit elementInserted(nodeId);
    return nodeId;
}

// ═══════════════════════════════════════════════════════════
// 提取最小值
// ═══════════════════════════════════════════════════════════

std::pair<double, QVariant> BinaryHeap::extractMin()
{
    if (m_heap.isEmpty()) {
        return {std::numeric_limits<double>::max(), QVariant()};
    }

    QElapsedTimer timer;
    timer.start();

    /* 取堆顶 */
    int topId = m_heap[0];
    double topKey = m_nodes[topId].key;
    QVariant topValue = m_nodes[topId].value;

    /* 将堆尾移到堆顶 */
    int lastId = m_heap.last();
    m_heap[0] = lastId;
    m_nodes[lastId].heapIndex = 0;
    m_heap.removeLast();

    /* 下沉 */
    if (!m_heap.isEmpty()) {
        siftDown(0);
    }

    /* 标记已删除 */
    m_nodes[topId].heapIndex = -1;

    /* 更新统计 */
    m_stats.totalExtracts++;
    const qint64 elapsed = timer.nsecsElapsed() / 1000000;
    const auto total = m_stats.totalInserts + m_stats.totalExtracts;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * (total - 1) / total +
        static_cast<double>(elapsed) / total;

    emit elementExtracted(topKey);
    return {topKey, topValue};
}

// ═══════════════════════════════════════════════════════════
// Decrease-Key
// ═══════════════════════════════════════════════════════════

void BinaryHeap::decreaseKey(Handle handle, double newKey)
{
    if (handle < 0 || handle >= m_nodes.size()) {
        return;
    }

    Node& node = m_nodes[handle];
    if (node.heapIndex < 0 || newKey > node.key) {
        return;  /* 无效操作 */
    }

    node.key = newKey;
    siftUp(node.heapIndex);
}

// ═══════════════════════════════════════════════════════════
// 查看堆顶
// ═══════════════════════════════════════════════════════════

std::pair<double, QVariant> BinaryHeap::peek() const
{
    if (m_heap.isEmpty()) {
        return {std::numeric_limits<double>::max(), QVariant()};
    }

    const Node& top = m_nodes[m_heap[0]];
    return {top.key, top.value};
}

bool BinaryHeap::isEmpty() const
{
    return m_heap.isEmpty();
}

int BinaryHeap::size() const
{
    return m_heap.size();
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

BinaryHeap::Stats BinaryHeap::stats() const
{
    return m_stats;
}

void BinaryHeap::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部实现
// ═══════════════════════════════════════════════════════════

void BinaryHeap::siftUp(int index)
{
    while (index > 0) {
        int parent = (index - 1) / 2;
        int idIdx = m_heap[index];
        int idPar = m_heap[parent];

        if (m_nodes[idIdx].key < m_nodes[idPar].key) {
            swapNodes(index, parent);
            index = parent;
        } else {
            break;
        }
    }
}

void BinaryHeap::siftDown(int index)
{
    const int heapSize = m_heap.size();

    while (true) {
        int smallest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < heapSize &&
            m_nodes[m_heap[left]].key < m_nodes[m_heap[smallest]].key) {
            smallest = left;
        }
        if (right < heapSize &&
            m_nodes[m_heap[right]].key < m_nodes[m_heap[smallest]].key) {
            smallest = right;
        }

        if (smallest != index) {
            swapNodes(index, smallest);
            index = smallest;
        } else {
            break;
        }
    }
}

void BinaryHeap::swapNodes(int i, int j)
{
    int idI = m_heap[i];
    int idJ = m_heap[j];

    m_heap[i] = idJ;
    m_heap[j] = idI;

    m_nodes[idI].heapIndex = j;
    m_nodes[idJ].heapIndex = i;
}
