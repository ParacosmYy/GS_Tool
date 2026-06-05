/**
 * @file BStarTree.cpp
 * @brief BStarTree 实现
 *
 * 实现B*树：2→3分裂(两个满节点→三个新节点)、
 * 3→2合并(三个稀疏节点→两个)、插入/删除/查找/范围查询。
 */

#include "utils/tree162/BStarTree.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
BStarTree::BStarTree(QObject* parent)
    : QObject(parent)
{
    m_maxKeys = m_order - 1;
    m_minKeys = qCeil(2.0 * m_maxKeys / 3.0);
}

void BStarTree::setOrder(int order)
{
    m_order = qMax(3, order);
    m_maxKeys = m_order - 1;
    m_minKeys = qCeil(2.0 * m_maxKeys / 3.0);
}

int BStarTree::findPosition(BNode* node, double key) const
{
    int pos = 0;
    while (pos < node->keys.size() && node->keys[pos] < key) pos++;
    return pos;
}

/**
 * @brief 2→3分裂
 *
 * 当节点满时，尝试从兄弟借位；若兄弟也满，
 * 则将两个满节点(共2*maxKeys个键)分裂为3个节点。
 */
void BStarTree::splitTwoToThree(BNode* node)
{
    if (!node->parent) {
        /* 根节点分裂：创建新根 */
        BNode* newRoot = new BNode();
        newRoot->isLeaf = false;
        newRoot->children.append(node);
        node->parent = newRoot;
        m_root = newRoot;
    }

    BNode* parent = node->parent;
    int idx = parent->children.indexOf(node);

    /* 尝试找兄弟 */
    BNode* sibling = nullptr;
    int sibIdx = -1;
    if (idx + 1 < parent->children.size()) {
        sibling = parent->children[idx + 1];
        sibIdx = idx + 1;
    } else if (idx - 1 >= 0) {
        sibling = parent->children[idx - 1];
        sibIdx = idx - 1;
    }

    if (sibling && sibling->keys.size() < m_maxKeys) {
        /* 兄弟有空间：旋转借位 */
        if (sibIdx > idx) {
            /* 右兄弟：左旋转 */
            double parentKey = parent->keys[idx];
            node->keys.append(parentKey);
            node->values.append(parent->values[idx]);
            parent->keys[idx] = sibling->keys.first();
            parent->values[idx] = sibling->values.first();
            sibling->keys.removeFirst();
            sibling->values.removeFirst();

            if (!node->isLeaf && sibling->children.size() > 0) {
                BNode* moved = sibling->children.takeFirst();
                moved->parent = node;
                node->children.append(moved);
            }
        } else {
            /* 左兄弟：右旋转 */
            double parentKey = parent->keys[sibIdx];
            node->keys.prepend(parentKey);
            node->values.prepend(parent->values[sibIdx]);
            parent->keys[sibIdx] = sibling->keys.last();
            parent->values[sibIdx] = sibling->values.last();
            sibling->keys.removeLast();
            sibling->values.removeLast();

            if (!node->isLeaf && sibling->children.size() > 0) {
                BNode* moved = sibling->children.takeLast();
                moved->parent = node;
                node->children.prepend(moved);
            }
        }
        return;
    }

    /* 兄弟也满或不存在：2→3分裂 */
    m_stats.totalSplits++;

    /* 合并当前节点和兄弟的所有键 */
    QVector<double> allKeys;
    QVector<double> allValues;

    if (sibling && sibIdx < idx) {
        allKeys = sibling->keys;
        allValues = sibling->values;
        allKeys.append(parent->keys[sibIdx]);
        allValues.append(parent->values[sibIdx]);
    }
    allKeys.append(node->keys);
    allValues.append(node->values);
    if (sibling && sibIdx > idx) {
        allKeys.append(parent->keys[idx]);
        allValues.append(parent->values[idx]);
        allKeys.append(sibling->keys);
        allValues.append(sibling->values);
    }

    /* 分成3个节点 */
    int total = allKeys.size();
    int s1 = total / 3;
    int s2 = (total - s1) / 2;

    BNode* n1 = new BNode();
    BNode* n2 = new BNode();
    BNode* n3 = new BNode();
    n1->isLeaf = node->isLeaf;
    n2->isLeaf = node->isLeaf;
    n3->isLeaf = node->isLeaf;
    n1->parent = parent;
    n2->parent = parent;
    n3->parent = parent;

    for (int i = 0; i < s1; ++i) { n1->keys.append(allKeys[i]); n1->values.append(allValues[i]); }
    for (int i = s1; i < s1 + s2; ++i) { n2->keys.append(allKeys[i]); n2->values.append(allValues[i]); }
    for (int i = s1 + s2; i < total; ++i) { n3->keys.append(allKeys[i]); n3->values.append(allValues[i]); }

    /* 更新父节点 */
    int removeStart = qMin(idx, sibIdx >= 0 ? sibIdx : idx);
    if (sibling) {
        parent->keys.removeAt(removeStart);
        parent->values.removeAt(removeStart);
        parent->children.removeAt(removeStart);
        parent->children.removeAt(removeStart);
    }

    parent->keys.insert(removeStart, n2->keys.first());
    parent->values.insert(removeStart, n2->values.first());
    if (n2->keys.size() > 1) {
        n2->keys.removeFirst();
        n2->values.removeFirst();
    }

    parent->children.insert(removeStart, n1);
    parent->children.insert(removeStart + 1, n2);
    parent->children.insert(removeStart + 2, n3);

    /* 检查父节点是否需要分裂 */
    if (parent->keys.size() > m_maxKeys) {
        splitTwoToThree(parent);
    }
}

/**
 * @brief 递归插入
 */
void BStarTree::insertRecursive(BNode* node, double key, double value)
{
    int pos = findPosition(node, key);

    if (node->isLeaf) {
        node->keys.insert(pos, key);
        node->values.insert(pos, value);

        if (node->keys.size() > m_maxKeys) {
            splitTwoToThree(node);
        }
    } else {
        if (pos >= node->children.size()) pos = node->children.size() - 1;
        insertRecursive(node->children[pos], key, value);
    }
}

/**
 * @brief 3→2合并
 */
void BStarTree::mergeThreeToTwo(BNode* node)
{
    m_stats.totalMerges++;
    BNode* parent = node->parent;
    if (!parent) return;

    int idx = parent->children.indexOf(node);

    /* 找到可合并的兄弟 */
    BNode* sibling = nullptr;
    int sibIdx = -1;
    if (idx + 1 < parent->children.size()) {
        sibling = parent->children[idx + 1];
        sibIdx = idx + 1;
    } else if (idx - 1 >= 0) {
        sibling = parent->children[idx - 1];
        sibIdx = idx - 1;
    }

    if (!sibling) return;

    /* 合并节点 */
    QVector<double> allKeys;
    QVector<double> allValues;

    int lo = qMin(idx, sibIdx);
    int hi = qMax(idx, sibIdx);

    allKeys.append(parent->children[lo]->keys);
    allValues.append(parent->children[lo]->values);
    if (lo < parent->keys.size()) {
        allKeys.append(parent->keys[lo]);
        allValues.append(parent->values[lo]);
    }
    allKeys.append(parent->children[hi]->keys);
    allValues.append(parent->children[hi]->values);

    /* 分成2个节点 */
    int mid = allKeys.size() / 2;

    BNode* merged1 = new BNode();
    BNode* merged2 = new BNode();
    merged1->isLeaf = node->isLeaf;
    merged2->isLeaf = node->isLeaf;
    merged1->parent = parent;
    merged2->parent = parent;

    for (int i = 0; i < mid; ++i) { merged1->keys.append(allKeys[i]); merged1->values.append(allValues[i]); }
    for (int i = mid; i < allKeys.size(); ++i) { merged2->keys.append(allKeys[i]); merged2->values.append(allValues[i]); }

    parent->keys.removeAt(lo);
    parent->values.removeAt(lo);
    delete parent->children[lo];
    delete parent->children[hi];
    parent->children.removeAt(lo);
    parent->children.removeAt(lo);
    parent->children.insert(lo, merged1);
    parent->children.insert(lo + 1, merged2);

    /* 如果父节点键为空且是根 */
    if (parent->keys.isEmpty()) {
        if (parent == m_root) {
            m_root = merged1;
            merged1->parent = nullptr;
            delete parent;
        }
    }
}

/**
 * @brief 递归删除
 */
bool BStarTree::removeRecursive(BNode* node, double key)
{
    int pos = findPosition(node, key);

    if (node->isLeaf) {
        if (pos < node->keys.size() && node->keys[pos] == key) {
            node->keys.removeAt(pos);
            node->values.removeAt(pos);

            if (node->keys.size() < m_minKeys && node != m_root) {
                mergeThreeToTwo(node);
            }
            return true;
        }
        return false;
    }

    /* 内部节点 */
    if (pos < node->keys.size() && node->keys[pos] == key) {
        /* 用后继替换 */
        BNode* succ = node->children[pos + 1];
        while (!succ->isLeaf) succ = succ->children[0];
        node->keys[pos] = succ->keys[0];
        node->values[pos] = succ->values[0];
        return removeRecursive(succ, succ->keys[0]);
    }

    if (pos >= node->children.size()) return false;
    return removeRecursive(node->children[pos], key);
}

void BStarTree::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new BNode();
        m_root->keys.append(key);
        m_root->values.append(value);
    } else {
        insertRecursive(m_root, key, value);
    }

    updateStats();
    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInserts + m_stats.totalDeletes;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit insertCompleted(key, m_stats.height);
}

bool BStarTree::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return false;

    bool found = removeRecursive(m_root, key);

    updateStats();
    m_stats.totalDeletes++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalInserts + m_stats.totalDeletes;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit deleteCompleted(key, found);
    return found;
}

bool BStarTree::find(double key, double& value) const
{
    BNode* node = m_root;
    while (node) {
        int pos = findPosition(node, key);
        if (pos < node->keys.size() && node->keys[pos] == key) {
            value = node->values[pos];
            return true;
        }
        if (node->isLeaf) return false;
        if (pos >= node->children.size()) return false;
        node = node->children[pos];
    }
    return false;
}

void BStarTree::rangeQueryRecursive(BNode* node, double low, double high,
                                     QVector<QPair<double, double>>& result) const
{
    if (!node) return;

    for (int i = 0; i < node->keys.size(); ++i) {
        if (!node->isLeaf && i < node->children.size()) {
            rangeQueryRecursive(node->children[i], low, high, result);
        }
        if (node->keys[i] >= low && node->keys[i] <= high) {
            result.append({node->keys[i], node->values[i]});
        }
    }
    if (!node->isLeaf && node->children.size() > node->keys.size()) {
        rangeQueryRecursive(node->children[node->keys.size()], low, high, result);
    }
}

QVector<QPair<double, double>> BStarTree::rangeQuery(double low, double high) const
{
    QVector<QPair<double, double>> result;
    rangeQueryRecursive(m_root, low, high, result);
    return result;
}

bool BStarTree::isEmpty() const
{
    return m_root == nullptr || m_root->keys.isEmpty();
}

void BStarTree::updateStats()
{
    m_stats.nodeCount = 0;
    m_stats.height = 0;

    if (!m_root) return;

    /* BFS统计 */
    QList<BNode*> queue;
    queue.append(m_root);
    while (!queue.isEmpty()) {
        int levelSize = queue.size();
        m_stats.height++;
        for (int i = 0; i < levelSize; ++i) {
            BNode* node = queue.takeFirst();
            m_stats.nodeCount++;
            for (auto* child : node->children) {
                queue.append(child);
            }
        }
    }
}

void BStarTree::deleteNode(BNode* node)
{
    if (!node) return;
    for (auto* child : node->children) {
        deleteNode(child);
    }
    delete node;
}

void BStarTree::clear()
{
    deleteNode(m_root);
    m_root = nullptr;
}

void BStarTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
