/**
 * @file BPlusTree2.cpp
 * @brief B+树实现 — 范围查询与有序键值存储
 */

#include "utils/tree5/BPlusTree2.h"

#include <QtMath>
#include <algorithm>
#include <QVariant>

/** @brief 构造函数 @param order 阶数 @param parent 父对象 */
BPlusTree2::BPlusTree2(int order, QObject* parent)
    : QObject(parent)
    , m_order(qMax(3, order))
    , m_count(0)
    , m_root(nullptr)
    , m_timeSum(0.0)
    , m_totalOps(0)
{
    m_root = new BPlusNode(true);
}

/** @brief 析构函数 */
BPlusTree2::~BPlusTree2()
{
    deleteTree(m_root);
}

/** @brief 插入键值对 @param key 键 @param value 值 */
void BPlusTree2::insert(double key, const QVariant& value)
{
    m_timer.start();

    BPlusNode* leaf = findLeaf(key);

    /* 在叶节点中找到插入位置 */
    int pos = 0;
    while (pos < leaf->keys.size() && leaf->keys[pos] < key) {
        ++pos;
    }

    leaf->keys.insert(pos, key);
    leaf->values.insert(pos, value);
    ++m_count;

    /* 检查是否需要分裂 (叶节点最多 m_order-1 个键) */
    if (leaf->keys.size() >= m_order) {
        splitLeaf(leaf);
    }

    ++m_stats.totalInserts;
    updateStats();

    emit elementInserted(key);
}

/** @brief 删除指定键 @param key 键 @return 是否成功 */
bool BPlusTree2::remove(double key)
{
    m_timer.start();

    BPlusNode* leaf = findLeaf(key);

    /* 在叶节点中查找键 */
    int pos = -1;
    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (qFuzzyCompare(leaf->keys[i], key)) {
            pos = i;
            break;
        }
    }

    if (pos < 0) {
        ++m_stats.totalSearches;
        updateStats();
        emit elementRemoved(key, false);
        return false;
    }

    leaf->keys.removeAt(pos);
    leaf->values.removeAt(pos);
    --m_count;

    ++m_stats.totalRemoves;
    updateStats();

    emit elementRemoved(key, true);
    return true;
}

/** @brief 精确查找 @param key 键 @return 值 */
QVariant BPlusTree2::find(double key) const
{
    m_timer.start();

    BPlusNode* leaf = findLeaf(key);

    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (qFuzzyCompare(leaf->keys[i], key)) {
            ++m_stats.totalSearches;
            updateStats();
            return leaf->values[i];
        }
    }

    ++m_stats.totalSearches;
    updateStats();
    return QVariant();
}

/** @brief 范围查询 [lo, hi] @param lo 下界 @param hi 上界 @return 键值对列表 */
QVector<QPair<double, QVariant>> BPlusTree2::rangeQuery(
    double lo, double hi) const
{
    m_timer.start();

    QVector<QPair<double, QVariant>> result;

    BPlusNode* leaf = findLeaf(lo);

    /* 从起始叶节点开始，沿链表扫描 */
    while (leaf != nullptr) {
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] > hi) {
                /* 已超过上界，停止扫描 */
                ++m_stats.totalSearches;
                updateStats();
                return result;
            }
            if (leaf->keys[i] >= lo) {
                result.append(qMakePair(leaf->keys[i], leaf->values[i]));
            }
        }
        leaf = leaf->next;
    }

    ++m_stats.totalSearches;
    updateStats();
    return result;
}

/** @brief 重置统计 */
void BPlusTree2::resetStatistics()
{
    m_stats    = Stats{};
    m_timeSum  = 0.0;
    m_totalOps = 0;
}

/** @brief 查找键所属的叶节点 @param key 目标键 @return 叶节点指针 */
BPlusNode* BPlusTree2::findLeaf(double key) const
{
    BPlusNode* node = m_root;
    while (!node->isLeaf) {
        int i = 0;
        while (i < node->keys.size() && key >= node->keys[i]) {
            ++i;
        }
        node = node->children[i];
    }
    return node;
}

/** @brief 分裂叶节点 @param node 待分裂节点 */
void BPlusTree2::splitLeaf(BPlusNode* node)
{
    int mid = node->keys.size() / 2;

    BPlusNode* newNode = new BPlusNode(true);
    newNode->next = node->next;
    node->next = newNode;

    /* 后半部分移到新节点 */
    for (int i = mid; i < node->keys.size(); ++i) {
        newNode->keys.append(node->keys[i]);
        newNode->values.append(node->values[i]);
    }

    /* 截断原节点 */
    while (node->keys.size() > mid) {
        node->keys.removeLast();
        node->values.removeLast();
    }

    /* 向上传播分隔键 */
    double splitKey = newNode->keys.first();
    insertIntoParent(node, splitKey, newNode);
}

/** @brief 分裂内部节点 @param node 待分裂节点 */
void BPlusTree2::splitInternal(BPlusNode* node)
{
    int mid = node->keys.size() / 2;

    BPlusNode* newNode = new BPlusNode(false);

    /* 分隔键(上移到父节点) */
    double splitKey = node->keys[mid];

    /* 后半部分移到新节点 */
    for (int i = mid + 1; i < node->keys.size(); ++i) {
        newNode->keys.append(node->keys[i]);
    }
    for (int i = mid + 1; i < node->children.size(); ++i) {
        newNode->children.append(node->children[i]);
        node->children[i]->parent = newNode;
    }

    /* 截断原节点 */
    while (node->keys.size() > mid) {
        node->keys.removeLast();
    }
    while (node->children.size() > mid + 1) {
        node->children.removeLast();
    }

    insertIntoParent(node, splitKey, newNode);
}

/** @brief 在父节点中插入分隔键 @param parent 父节点 @param key 分隔键 @param right 右子节点 */
void BPlusTree2::insertIntoParent(BPlusNode* left, double key, BPlusNode* right)
{
    if (left == m_root) {
        /* 创建新根节点 */
        BPlusNode* newRoot = new BPlusNode(false);
        newRoot->keys.append(key);
        newRoot->children.append(left);
        newRoot->children.append(right);
        left->parent  = newRoot;
        right->parent = newRoot;
        m_root = newRoot;
        return;
    }

    BPlusNode* parent = left->parent;

    /* 找到left在parent中的位置 */
    int pos = 0;
    while (pos < parent->children.size() && parent->children[pos] != left) {
        ++pos;
    }

    parent->keys.insert(pos, key);
    parent->children.insert(pos + 1, right);
    right->parent = parent;

    /* 检查内部节点是否需要分裂 */
    if (parent->keys.size() >= m_order) {
        splitInternal(parent);
    }
}

/** @brief 递归删除节点树 @param node 根节点 */
void BPlusTree2::deleteTree(BPlusNode* node)
{
    if (node == nullptr) return;
    if (!node->isLeaf) {
        for (BPlusNode* child : node->children) {
            deleteTree(child);
        }
    }
    delete node;
}

/** @brief 更新计时统计 */
void BPlusTree2::updateStats() const
{
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_totalOps = m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalSearches;
    if (m_totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(m_totalOps);
    }
}
