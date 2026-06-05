/**
 * @file BPlusTree7.cpp
 * @brief B+树实现
 *
 * 实现支持插入、删除、范围查询的B+树数据结构。
 * 适用于大规模有序数据的索引和范围检索。
 */

#include "utils/tree70/BPlusTree7.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
BPlusTree7::BPlusTree7(QObject* parent)
    : QObject(parent)
{
    m_root = new BPNode{true, {}, {}, {}, nullptr, nullptr};
}

/**
 * @brief 设置B+树的阶数
 * @param order 阶数，每个节点最多包含order-1个键
 */
void BPlusTree7::setOrder(int order)
{
    m_order = qBound(3, order, 1024);
}

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void BPlusTree7::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    // 查找目标叶节点
    BPNode* leaf = findLeaf(key);

    // 在叶节点中找到插入位置
    int pos = 0;
    while (pos < leaf->keys.size() && leaf->keys[pos] < key) pos++;

    leaf->keys.insert(pos, key);
    leaf->vals.insert(pos, value);

    // 检查是否需要分裂
    if (leaf->keys.size() >= m_order) {
        splitLeaf(leaf);
    }

    m_size++;

    // 更新高度
    BPNode* cur = leaf;
    m_height = 0;
    while (cur != nullptr) {
        m_height++;
        cur = cur->parent;
    }

    qint64 elapsed = timer.elapsed();
    m_stats.totalInserts++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    emit inserted(key);
}

/**
 * @brief 删除键
 * @param key 要删除的键
 */
void BPlusTree7::remove(double key)
{
    BPNode* leaf = findLeaf(key);
    int pos = leaf->keys.indexOf(key);
    if (pos >= 0) {
        leaf->keys.remove(pos);
        leaf->vals.remove(pos);
        m_size--;

        // 简化：不从父节点级联合并（仅保证正确性）
        if (leaf->keys.isEmpty() && leaf == m_root && !leaf->leaf) {
            // 根节点为空且非叶子时收缩
        }
    }
}

/**
 * @brief 检查是否包含指定键
 * @param key 待查找的键
 * @return 是否存在
 */
bool BPlusTree7::contains(double key) const
{
    BPNode* leaf = findLeaf(key);
    return leaf->keys.contains(key);
}

/**
 * @brief 范围查询
 * @param lo 下界（包含）
 * @param hi 上界（包含）
 * @return 范围内所有值
 */
QVector<int> BPlusTree7::rangeQuery(double lo, double hi) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    BPNode* leaf = findLeaf(lo);

    // 从包含lo的叶节点开始扫描
    while (leaf != nullptr) {
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] > hi) {
                qint64 elapsed = timer.elapsed();
                const_cast<BPlusTree7*>(this)->m_stats.totalQueries++;
                const_cast<BPlusTree7*>(this)->m_timeSum += elapsed;
                return result;
            }
            if (leaf->keys[i] >= lo && leaf->keys[i] <= hi) {
                result.append(leaf->vals[i]);
            }
        }
        leaf = leaf->next;
    }

    qint64 elapsed = timer.elapsed();
    const_cast<BPlusTree7*>(this)->m_stats.totalQueries++;
    const_cast<BPlusTree7*>(this)->m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    return result;
}

/**
 * @brief 重置统计信息
 */
void BPlusTree7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 查找键所属的叶节点
 * @param key 待查找的键
 * @return 目标叶节点指针
 */
BPlusTree7::BPNode* BPlusTree7::findLeaf(double key) const
{
    BPNode* cur = m_root;
    while (!cur->leaf) {
        int i = 0;
        while (i < cur->keys.size() && key >= cur->keys[i]) i++;
        if (i < cur->children.size()) {
            cur = cur->children[i];
        } else {
            break;
        }
    }
    return cur;
}

/**
 * @brief 分裂叶节点
 * @param leaf 需要分裂的叶节点
 */
void BPlusTree7::splitLeaf(BPNode* leaf)
{
    int mid = leaf->keys.size() / 2;

    // 创建新叶节点
    BPNode* newLeaf = new BPNode{true, {}, {}, {}, nullptr, leaf->next};

    // 分裂键和值
    for (int i = mid; i < leaf->keys.size(); ++i) {
        newLeaf->keys.append(leaf->keys[i]);
        newLeaf->vals.append(leaf->vals[i]);
    }
    leaf->keys.resize(mid);
    leaf->vals.resize(mid);
    leaf->next = newLeaf;

    // 向父节点插入分割键
    double splitKey = newLeaf->keys[0];
    insertIntoParent(leaf, splitKey, newLeaf);
}

/**
 * @brief 向父节点插入分割键
 * @param left 左子节点
 * @param key 分割键
 * @param right 右子节点
 */
void BPlusTree7::insertIntoParent(BPNode* left, double key, BPNode* right)
{
    if (left == m_root) {
        // 创建新根
        BPNode* newRoot = new BPNode{false, {key}, {}, {left, right}, nullptr, nullptr};
        left->parent = newRoot;
        right->parent = newRoot;
        m_root = newRoot;
        return;
    }

    BPNode* parent = left->parent;
    int pos = 0;
    while (pos < parent->keys.size() && parent->keys[pos] < key) pos++;

    parent->keys.insert(pos, key);
    parent->children.insert(pos + 1, right);
    right->parent = parent;

    // 检查内部节点是否需要分裂
    if (parent->keys.size() >= m_order) {
        int mid = parent->keys.size() / 2;
        double upKey = parent->keys[mid];

        BPNode* newInternal = new BPNode{false, {}, {}, {}, parent->parent, nullptr};
        for (int i = mid + 1; i < parent->keys.size(); ++i) {
            newInternal->keys.append(parent->keys[i]);
        }
        for (int i = mid + 1; i < parent->children.size(); ++i) {
            parent->children[i]->parent = newInternal;
            newInternal->children.append(parent->children[i]);
        }

        parent->keys.resize(mid);
        parent->children.resize(mid + 1);

        insertIntoParent(parent, upKey, newInternal);
    }
}

/**
 * @brief 分裂内部节点
 * @param node 需要分裂的内部节点
 */
void BPlusTree7::splitInternal(BPNode* node)
{
    // 此逻辑已在insertIntoParent中处理
    (void)node;
}

/**
 * @brief 从叶节点扫描到上界
 * @param leaf 起始叶节点
 * @param hi 上界
 * @param res 结果容器
 */
void BPlusTree7::rangeQueryLeaf(BPNode* leaf, double hi, QVector<int>& res) const
{
    while (leaf != nullptr) {
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] > hi) return;
            res.append(leaf->vals[i]);
        }
        leaf = leaf->next;
    }
}
