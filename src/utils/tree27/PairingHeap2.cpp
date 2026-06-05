/**
 * @file PairingHeap2.cpp
 * @brief 配对堆实现 — 两路合并+两趟配对+decrease-key剪切
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tree27/PairingHeap2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
PairingHeap2::PairingHeap2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 析构函数 — 释放所有节点
 */
PairingHeap2::~PairingHeap2()
{
    clear();
}

/**
 * @brief 插入元素
 *
 * 创建新节点并通过link操作合并到堆中。
 * 新节点作为单节点子堆与当前根合并。
 *
 * @param key 键值
 * @param value 关联值
 * @return 节点句柄(用于后续decreaseKey)
 */
int PairingHeap2::insert(double key, int value)
{
    Node node;
    node.key = key;
    node.value = value;
    node.child = -1;
    node.sibling = -1;
    node.parent = -1;
    node.active = true;

    int handle = m_nodes.size();
    m_nodes.append(node);
    m_size++;

    /* 新节点与当前根合并 */
    m_root = link(m_root, handle);

    m_stats.totalInsertions++;
    m_stats.avgProcessingTimeMs = 0.0;

    return handle;
}

/**
 * @brief 获取最小键及其关联值
 *
 * 配对堆的根节点即为最小元素。
 *
 * @return (最小键, 关联值), 堆空时返回(qInf, -1)
 */
QPair<double, int> PairingHeap2::findMin() const
{
    if (m_root < 0 || m_root >= m_nodes.size()) {
        return {qInf(), -1};
    }
    return {m_nodes[m_root].key, m_nodes[m_root].value};
}

/**
 * @brief 删除最小元素
 *
 * 删除根节点后,对其子树执行两趟配对(two-pass pairing):
 * 1. 第一趟: 从左到右,相邻子树两两合并
 * 2. 第二趟: 从右到左,将合并结果依次合并
 *
 * @return 被删除的(键, 值), 堆空时返回(qInf, -1)
 */
QPair<double, int> PairingHeap2::deleteMin()
{
    if (m_root < 0) {
        return {qInf(), -1};
    }

    QElapsedTimer timer;
    timer.start();

    double minKey = m_nodes[m_root].key;
    int minValue = m_nodes[m_root].value;
    int firstChild = m_nodes[m_root].child;

    /* 标记删除的节点为非活跃 */
    m_nodes[m_root].active = false;
    m_size--;

    /* 对子链执行两趟配对 */
    m_root = mergePairs(firstChild);

    m_stats.totalDeletes++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalInsertions + m_stats.totalDeletes
                 + m_stats.totalDecreaseKeys;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit minDeleted(minKey);
    return {minKey, minValue};
}

/**
 * @brief 减键操作
 *
 * 将指定节点的键减小到newKey。
 * 实现: 剪切节点及其子树,从父节点的子链中移除,
 * 然后作为独立子堆与堆根合并。
 *
 * @param handle 节点句柄(insert()返回值)
 * @param newKey 新键值(必须小于当前键)
 */
void PairingHeap2::decreaseKey(int handle, double newKey)
{
    if (handle < 0 || handle >= m_nodes.size() || !m_nodes[handle].active) {
        return;
    }

    if (newKey >= m_nodes[handle].key) {
        return;
    }

    m_nodes[handle].key = newKey;

    /* 若该节点是根,无需剪切 */
    if (handle == m_root) {
        m_stats.totalDecreaseKeys++;
        return;
    }

    /* 从父节点的子链中移除该节点 */
    int parent = m_nodes[handle].parent;
    if (parent >= 0 && parent < m_nodes.size()) {
        int child = m_nodes[parent].child;
        if (child == handle) {
            /* handle是第一个子节点 */
            m_nodes[parent].child = m_nodes[handle].sibling;
        } else {
            /* 在兄弟链中找到handle的前驱 */
            int prev = child;
            while (prev >= 0 && m_nodes[prev].sibling != handle) {
                prev = m_nodes[prev].sibling;
            }
            if (prev >= 0) {
                m_nodes[prev].sibling = m_nodes[handle].sibling;
            }
        }
    }

    /* 清除剪切节点的兄弟和父指针 */
    m_nodes[handle].sibling = -1;
    m_nodes[handle].parent = -1;

    /* 与堆根合并 */
    m_root = link(m_root, handle);

    m_stats.totalDecreaseKeys++;
}

/**
 * @brief 合并另一个配对堆
 *
 * 将other堆的所有节点合并到当前堆中。
 * 合并后other堆变为空。
 *
 * @param other 另一个PairingHeap2(合并后变空)
 */
void PairingHeap2::meld(PairingHeap2& other)
{
    if (other.isEmpty()) {
        return;
    }

    m_root = link(m_root, other.m_root);

    /* 将other的节点移动到当前堆 */
    for (auto& node : other.m_nodes) {
        m_nodes.append(node);
    }

    m_size += other.m_size;
    m_stats.totalMelds++;

    /* 清空other */
    other.m_root = -1;
    other.m_size = 0;
    other.m_nodes.clear();
}

/**
 * @brief 清空堆
 *
 * 释放所有节点并重置堆状态。
 */
void PairingHeap2::clear()
{
    m_nodes.clear();
    m_root = -1;
    m_size = 0;
}

/**
 * @brief 获取所有元素按键排序
 *
 * 通过反复deleteMin收集所有元素。
 * 注意: 此操作会清空堆。
 *
 * @return 按键升序排列的(键,值)列表
 */
QVector<QPair<double, int>> PairingHeap2::toSortedVector() const
{
    QVector<QPair<double, int>> result;
    result.reserve(m_size);

    /* 复制堆状态用于排序 */
    PairingHeap2 temp;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].active) {
            temp.insert(m_nodes[i].key, m_nodes[i].value);
        }
    }

    while (!temp.isEmpty()) {
        result.append(temp.deleteMin());
    }
    return result;
}

/**
 * @brief 两趟配对合并
 *
 * deleteMin后的核心操作:
 * 1. 第一趟(left-to-right): 相邻兄弟两两link
 * 2. 第二趟(right-to-left): 将所有中间结果依次link到最终根
 *
 * @param first 子链的第一个节点索引
 * @return 配对后的新根索引
 */
int PairingHeap2::mergePairs(int first)
{
    if (first < 0) {
        return -1;
    }
    if (first >= m_nodes.size() || !m_nodes[first].active) {
        return -1;
    }

    /* 收集兄弟链中的所有活跃子节点 */
    QVector<int> siblings;
    int cur = first;
    while (cur >= 0 && cur < m_nodes.size()) {
        if (m_nodes[cur].active) {
            siblings.append(cur);
            /* 断开兄弟和父指针 */
            int next = m_nodes[cur].sibling;
            m_nodes[cur].sibling = -1;
            m_nodes[cur].parent = -1;
            cur = next;
        } else {
            cur = m_nodes[cur].sibling;
        }
    }

    if (siblings.isEmpty()) {
        return -1;
    }
    if (siblings.size() == 1) {
        return siblings[0];
    }

    /* 第一趟: left-to-right两两合并 */
    QVector<int> merged;
    for (int i = 0; i + 1 < siblings.size(); i += 2) {
        merged.append(link(siblings[i], siblings[i + 1]));
    }
    /* 奇数个时最后一个直接加入 */
    if (siblings.size() % 2 == 1) {
        merged.append(siblings.last());
    }

    /* 第二趟: right-to-left依次合并 */
    int result = merged.last();
    for (int i = merged.size() - 2; i >= 0; --i) {
        result = link(result, merged[i]);
    }

    return result;
}

/**
 * @brief 合并两个子树
 *
 * 比较两个根节点的键,较小的成为新根,较大的作为其最左子节点。
 * 较大节点的sibling指向原最左子节点。
 *
 * @param a 子树A的根索引
 * @param b 子树B的根索引
 * @return 合并后的根索引
 */
int PairingHeap2::link(int a, int b)
{
    /* 处理空树 */
    if (a < 0) return b;
    if (b < 0) return a;
    if (a >= m_nodes.size() || b >= m_nodes.size()) {
        return (a >= m_nodes.size()) ? b : a;
    }

    /* 保证a的键更小 */
    if (m_nodes[a].key > m_nodes[b].key) {
        std::swap(a, b);
    }

    /* b成为a的最左子节点 */
    m_nodes[b].sibling = m_nodes[a].child;
    m_nodes[b].parent = a;
    m_nodes[a].child = b;

    m_stats.totalMelds++;
    return a;
}

/**
 * @brief 重置统计信息
 */
void PairingHeap2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
