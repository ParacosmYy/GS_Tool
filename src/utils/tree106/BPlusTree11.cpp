#include "BPlusTree11.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @file BPlusTree11.cpp
 * @brief B+树实现
 *
 * B+树特性:
 * - 所有数据存储在叶子节点，内部节点仅存键和子节点指针
 * - 叶子节点通过链表串联，支持高效范围查询
 * - 每个节点最多order个子节点，最少ceil(order/2)个
 * - 插入/删除可能导致节点分裂/合并
 */

/// B+树节点
struct BPlusNode {
    bool isLeaf;                  ///< 是否为叶节点
    QVector<double> keys;         ///< 键值数组
    QVector<int> values;          ///< 数据(仅叶节点)
    QVector<BPlusNode*> children; ///< 子节点指针(仅内部节点)
    BPlusNode* next;              ///< 叶节点链表后继
    BPlusNode* parent;            ///< 父节点

    explicit BPlusNode(bool leaf = false)
        : isLeaf(leaf), next(nullptr), parent(nullptr) {}
};

/// 全局树根和参数
static BPlusNode* g_bplusRoot = nullptr;
static int g_bplusOrder = 4;

/**
 * @brief 在有序数组中查找插入位置
 */
static int findInsertPos(const QVector<double>& keys, double key)
{
    int pos = 0;
    while (pos < keys.size() && keys[pos] < key) pos++;
    return pos;
}

/**
 * @brief 分裂叶节点
 */
static void splitLeaf(BPlusNode* leaf)
{
    const int mid = leaf->keys.size() / 2;
    BPlusNode* newLeaf = new BPlusNode(true);

    // 分配键值
    newLeaf->keys = leaf->keys.mid(mid);
    newLeaf->values = leaf->values.mid(mid);
    leaf->keys.resize(mid);
    leaf->values.resize(mid);

    // 链表连接
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;
    newLeaf->parent = leaf->parent;

    // 向父节点插入分裂键
    double splitKey = newLeaf->keys[0];
    BPlusNode* parent = leaf->parent;

    if (!parent) {
        parent = new BPlusNode(false);
        parent->keys.append(splitKey);
        parent->children.append(leaf);
        parent->children.append(newLeaf);
        leaf->parent = parent;
        newLeaf->parent = parent;
        g_bplusRoot = parent;
    } else {
        int pos = findInsertPos(parent->keys, splitKey);
        parent->keys.insert(pos, splitKey);
        parent->children.insert(pos + 1, newLeaf);

        // 检查父节点是否需要分裂
        if (parent->keys.size() >= g_bplusOrder) {
            // 简化: 内部节点分裂
            const int pMid = parent->keys.size() / 2;
            BPlusNode* newInternal = new BPlusNode(false);
            double upKey = parent->keys[pMid];

            newInternal->keys = parent->keys.mid(pMid + 1);
            parent->keys.resize(pMid);

            for (int i = pMid + 1; i < parent->children.size(); ++i) {
                newInternal->children.append(parent->children[i]);
                parent->children[i]->parent = newInternal;
            }
            parent->children.resize(pMid + 1);

            // 递归向上分裂(简化: 仅处理一层)
            if (parent->parent) {
                int pp = findInsertPos(parent->parent->keys, upKey);
                parent->parent->keys.insert(pp, upKey);
                parent->parent->children.insert(pp + 1, newInternal);
                newInternal->parent = parent->parent;
            } else {
                BPlusNode* newRoot = new BPlusNode(false);
                newRoot->keys.append(upKey);
                newRoot->children.append(parent);
                newRoot->children.append(newInternal);
                parent->parent = newRoot;
                newInternal->parent = newRoot;
                g_bplusRoot = newRoot;
            }
        }
    }
}

/**
 * @brief 构造函数
 * @param parent 父QObject对象指针
 */
BPlusTree11::BPlusTree11(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置B+树阶数
 * @param order 最大子节点数
 */
void BPlusTree11::setOrder(int order)
{
    g_bplusOrder = qMax(3, order);
}

/**
 * @brief 插入键值对
 * @param key 键值
 * @param value 关联数据
 */
void BPlusTree11::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    if (!g_bplusRoot) {
        g_bplusRoot = new BPlusNode(true);
    }

    // 找到目标叶节点
    BPlusNode* curr = g_bplusRoot;
    while (!curr->isLeaf) {
        int pos = findInsertPos(curr->keys, key);
        curr = curr->children[pos];
    }

    // 在叶节点中插入
    int pos = findInsertPos(curr->keys, key);
    curr->keys.insert(pos, key);
    curr->values.insert(pos, value);

    // 检查是否需要分裂
    if (curr->keys.size() >= g_bplusOrder) {
        splitLeaf(curr);
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit inserted(key);
}

/**
 * @brief 删除指定键
 * @param key 要删除的键值
 */
void BPlusTree11::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (!g_bplusRoot) {
        m_timeSum += timer.elapsed();
        return;
    }

    // 找到目标叶节点
    BPlusNode* curr = g_bplusRoot;
    while (!curr->isLeaf) {
        int pos = findInsertPos(curr->keys, key);
        curr = curr->children[pos];
    }

    // 在叶节点中查找并删除
    int pos = findInsertPos(curr->keys, key);
    if (pos < curr->keys.size() && curr->keys[pos] == key) {
        curr->keys.removeAt(pos);
        curr->values.removeAt(pos);
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 范围查询
 * @param minKey 范围下界
 * @param maxKey 范围上界
 * @return 范围内的所有键值对
 */
QVector<QPair<double, int>> BPlusTree11::rangeQuery(double minKey, double maxKey)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, int>> result;

    if (!g_bplusRoot) {
        m_timeSum += timer.elapsed();
        return result;
    }

    // 找到>=minKey的第一个叶节点
    BPlusNode* curr = g_bplusRoot;
    while (!curr->isLeaf) {
        int pos = findInsertPos(curr->keys, minKey);
        curr = curr->children[pos];
    }

    // 沿叶节点链表扫描
    while (curr) {
        for (int i = 0; i < curr->keys.size(); ++i) {
            if (curr->keys[i] > maxKey) {
                m_stats.totalOperations++;
                m_timeSum += timer.elapsed();
                m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
                return result;
            }
            if (curr->keys[i] >= minKey) {
                result.append(qMakePair(curr->keys[i], curr->values[i]));
            }
        }
        curr = curr->next;
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    return result;
}

/**
 * @brief 重置所有统计信息
 */
void BPlusTree11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    g_bplusRoot = nullptr;
}
