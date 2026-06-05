/**
 * @file BTreeMap.cpp
 * @brief B树映射容器实现 — 批量加载/范围查询
 */

#include "utils/btree5/BTreeMap.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <queue>

/* ================================================================
 *  构造 / 析构
 * ================================================================ */

/** @brief 构造函数 @param order B树阶数 @param parent 父对象 */
BTreeMap::BTreeMap(int order, QObject* parent)
    : QObject(parent)
    , m_order(qMax(3, order))
    , m_root(new BTreeNode(true))
{
}

/** @brief 析构函数 — 释放所有节点 */
BTreeMap::~BTreeMap()
{
    destroyNode(m_root);
}

/* ================================================================
 *  插入
 * ================================================================ */

/** @brief 插入键值对
 *  @param key 键
 *  @param value 值
 *  @return 是否成功 */
bool BTreeMap::insert(double key, const QString& value)
{
    QElapsedTimer timer;
    timer.start();

    /* 检查是否已存在 */
    QString existing;
    if (find(key, existing)) {
        return false;
    }

    BTreeNode* root = m_root;
    if (root->keyCount == m_order - 1) {
        /* 根节点已满，创建新根并分裂 */
        BTreeNode* newRoot = new BTreeNode(false);
        newRoot->children.append(root);
        splitChild(newRoot, 0, root);
        m_root = newRoot;
        insertNonFull(newRoot, key, value);
    } else {
        insertNonFull(root, key, value);
    }

    ++m_size;
    ++m_stats.totalInsertions;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    double totalOps = static_cast<double>(
        m_stats.totalInsertions + m_stats.totalDeletions +
        m_stats.totalLookups + m_stats.totalRangeQueries);
    if (totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / totalOps;
    }

    emit inserted(key, m_size);
    return true;
}

/** @brief 分裂子节点 */
void BTreeMap::splitChild(BTreeNode* parent, int index, BTreeNode* child)
{
    int t = (m_order - 1) / 2;  /* 最小度数 */
    BTreeNode* sibling = new BTreeNode(child->isLeaf);

    /* 复制后半部分键值到兄弟节点 */
    for (int i = 0; i < t; ++i) {
        if (i + t + 1 < child->keys.size()) {
            sibling->keys.append(child->keys[i + t + 1]);
            sibling->values.append(child->values[i + t + 1]);
        }
    }

    /* 如果不是叶节点，复制子节点 */
    if (!child->isLeaf) {
        for (int i = 0; i <= t; ++i) {
            if (i + t + 1 < child->children.size()) {
                sibling->children.append(child->children[i + t + 1]);
            }
        }
    }

    /* 提升中间键到父节点 */
    double midKey = child->keys[t];
    QString midVal = child->values[t];

    /* 裁剪原节点 */
    child->keys.resize(t);
    child->values.resize(t);
    if (!child->isLeaf) {
        child->children.resize(t + 1);
    }
    child->keyCount = t;

    /* 插入到父节点 */
    parent->keys.insert(index, midKey);
    parent->values.insert(index, midVal);
    parent->children.insert(index + 1, sibling);
    parent->keyCount = parent->keys.size();

    emit rebalanced(height());
}

/** @brief 插入到非满节点 */
void BTreeMap::insertNonFull(BTreeNode* node, double key,
                             const QString& value)
{
    int i = node->keys.size() - 1;

    if (node->isLeaf) {
        /* 叶节点: 找到插入位置并移动元素 */
        while (i >= 0 && node->keys[i] > key) {
            --i;
        }
        node->keys.insert(i + 1, key);
        node->values.insert(i + 1, value);
        node->keyCount = node->keys.size();
    } else {
        /* 内部节点: 递归到子节点 */
        while (i >= 0 && node->keys[i] > key) {
            --i;
        }
        ++i;

        if (i < node->children.size() &&
            node->children[i]->keyCount == m_order - 1) {
            splitChild(node, i, node->children[i]);
            if (key > node->keys[i]) {
                ++i;
            }
        }

        if (i < node->children.size()) {
            insertNonFull(node->children[i], key, value);
        }
    }
}

/* ================================================================
 *  删除
 * ================================================================ */

/** @brief 删除键
 *  @param key 要删除的键
 *  @return 是否成功 */
bool BTreeMap::remove(double key)
{
    if (m_root == nullptr) return false;

    bool found = removeHelper(m_root, key);

    /* 如果根节点变空且不是叶节点 */
    if (!m_root->isLeaf && m_root->keys.isEmpty()) {
        BTreeNode* old = m_root;
        m_root = m_root->children[0];
        old->children.clear();
        delete old;
    }

    if (found) {
        --m_size;
        ++m_stats.totalDeletions;
        emit removed(key);
    }

    return found;
}

/** @brief 递归删除 */
bool BTreeMap::removeHelper(BTreeNode* node, double key)
{
    int idx = findKeyIndex(node, key);

    if (idx < node->keys.size() && qFuzzyCompare(node->keys[idx], key)) {
        /* 找到键 */
        if (node->isLeaf) {
            /* 叶节点直接删除 */
            node->keys.removeAt(idx);
            node->values.removeAt(idx);
            node->keyCount = node->keys.size();
            return true;
        } else {
            /* 内部节点: 用前驱替换 */
            BTreeNode* pred = node->children[idx];
            while (!pred->isLeaf) {
                pred = pred->children[pred->keys.size()];
            }
            node->keys[idx] = pred->keys[pred->keys.size() - 1];
            node->values[idx] = pred->values[pred->values.size() - 1];
            return removeHelper(node->children[idx],
                                pred->keys[pred->keys.size() - 1]);
        }
    } else {
        if (node->isLeaf) return false;

        /* 递归到子节点 */
        if (idx < node->children.size()) {
            return removeHelper(node->children[idx], key);
        }
    }
    return false;
}

/** @brief 找到键在节点中的位置 */
int BTreeMap::findKeyIndex(BTreeNode* node, double key) const
{
    int idx = 0;
    while (idx < node->keys.size() && node->keys[idx] < key) {
        ++idx;
    }
    return idx;
}

/* ================================================================
 *  查找
 * ================================================================ */

/** @brief 查找键对应的值 */
bool BTreeMap::find(double key, QString& value) const
{
    ++m_stats.totalLookups;
    return findHelper(m_root, key, value);
}

/** @brief 递归查找 */
bool BTreeMap::findHelper(BTreeNode* node, double key,
                          QString& value) const
{
    if (node == nullptr) return false;

    int i = 0;
    while (i < node->keys.size() && node->keys[i] < key) {
        ++i;
    }

    if (i < node->keys.size() && qFuzzyCompare(node->keys[i], key)) {
        value = node->values[i];
        return true;
    }

    if (node->isLeaf) return false;

    if (i < node->children.size()) {
        return findHelper(node->children[i], key, value);
    }
    return false;
}

/* ================================================================
 *  批量加载
 * ================================================================ */

/** @brief 批量加载预排序数据
 *  @param sortedPairs 已排序的键值对列表 */
void BTreeMap::bulkLoad(const QVector<KeyValuePair>& sortedPairs)
{
    QElapsedTimer timer;
    timer.start();

    clear();

    if (sortedPairs.isEmpty()) return;

    /* 递归构建平衡B树 */
    m_root = buildBulkLoad(sortedPairs, 0, sortedPairs.size() - 1, true);
    m_size = sortedPairs.size();

    m_stats.totalInsertions += static_cast<quint64>(sortedPairs.size());

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
}

/** @brief 递归构建批量加载 */
BTreeMap::BTreeNode* BTreeMap::buildBulkLoad(
    const QVector<KeyValuePair>& pairs, int start, int end, bool isLeaf)
{
    if (start > end) return nullptr;

    int count = end - start + 1;
    int mid = start + count / 2;

    BTreeNode* node = new BTreeNode(isLeaf);
    int t = (m_order - 1) / 2;

    /* 将范围内的键分配给此节点 */
    int nodeStart = qMax(start, mid - t);
    int nodeEnd = qMin(end, mid + t);

    for (int i = nodeStart; i <= nodeEnd; ++i) {
        node->keys.append(pairs[i].first);
        node->values.append(pairs[i].second);
    }
    node->keyCount = node->keys.size();

    /* 递归构建子节点 */
    if (!isLeaf && nodeStart > start) {
        /* 左子树 */
        BTreeNode* left = buildBulkLoad(pairs, start, nodeStart - 1, false);
        if (left) node->children.append(left);
    }

    if (!isLeaf && nodeEnd < end) {
        /* 右子树 */
        BTreeNode* right = buildBulkLoad(pairs, nodeEnd + 1, end, false);
        if (right) node->children.append(right);
    }

    return node;
}

/* ================================================================
 *  范围查询 / 遍历
 * ================================================================ */

/** @brief 范围查询 */
QList<BTreeMap::KeyValuePair> BTreeMap::rangeQuery(
    double low, double high) const
{
    QElapsedTimer timer;
    timer.start();
    ++m_stats.totalRangeQueries;

    QList<KeyValuePair> result;
    rangeQueryHelper(m_root, low, high, result);

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);

    return result;
}

/** @brief 递归范围查询 */
void BTreeMap::rangeQueryHelper(BTreeNode* node, double low, double high,
                                QList<KeyValuePair>& result) const
{
    if (node == nullptr) return;

    for (int i = 0; i < node->keys.size(); ++i) {
        /* 先访问小于当前键的子树 */
        if (!node->isLeaf && i < node->children.size() &&
            node->keys[i] > low) {
            rangeQueryHelper(node->children[i], low, high, result);
        }

        /* 当前键在范围内 */
        if (node->keys[i] >= low && node->keys[i] <= high) {
            result.append({node->keys[i], node->values[i]});
        }
    }

    /* 最右子树 */
    if (!node->isLeaf && !node->children.isEmpty()) {
        int lastIdx = node->keys.size();
        if (lastIdx < node->children.size()) {
            rangeQueryHelper(node->children[lastIdx], low, high, result);
        }
    }
}

/** @brief 有序遍历所有键值对 */
QList<BTreeMap::KeyValuePair> BTreeMap::inOrderTraversal() const
{
    QList<KeyValuePair> result;
    inOrderHelper(m_root, result);
    return result;
}

/** @brief 递归中序遍历 */
void BTreeMap::inOrderHelper(BTreeNode* node,
                             QList<KeyValuePair>& result) const
{
    if (node == nullptr) return;

    for (int i = 0; i < node->keys.size(); ++i) {
        if (!node->isLeaf && i < node->children.size()) {
            inOrderHelper(node->children[i], result);
        }
        result.append({node->keys[i], node->values[i]});
    }
    if (!node->isLeaf && !node->children.isEmpty()) {
        inOrderHelper(node->children[node->keys.size()], result);
    }
}

/* ================================================================
 *  辅助方法
 * ================================================================ */

/** @brief 获取树中元素数量 */
int BTreeMap::size() const { return m_size; }

/** @brief 树是否为空 */
bool BTreeMap::isEmpty() const { return m_size == 0; }

/** @brief 获取树的高度 */
int BTreeMap::height() const
{
    return heightHelper(m_root);
}

/** @brief 递归计算高度 */
int BTreeMap::heightHelper(BTreeNode* node) const
{
    if (node == nullptr) return 0;
    if (node->isLeaf) return 1;
    if (node->children.isEmpty()) return 1;
    return 1 + heightHelper(node->children[0]);
}

/** @brief 清空树 */
void BTreeMap::clear()
{
    destroyNode(m_root);
    m_root = new BTreeNode(true);
    m_size = 0;
}

/** @brief 递归销毁 */
void BTreeMap::destroyNode(BTreeNode* node)
{
    if (node == nullptr) return;
    for (auto* child : node->children) {
        destroyNode(child);
    }
    delete node;
}

/** @brief 获取统计信息 */
BTreeMap::Stats BTreeMap::stats() const { return m_stats; }

/** @brief 重置统计 */
void BTreeMap::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
