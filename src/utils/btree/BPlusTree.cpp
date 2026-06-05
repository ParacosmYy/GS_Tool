/**
 * @file BPlusTree.cpp
 * @brief B+树实现
 */

#include "utils/btree/BPlusTree.h"

#include <QElapsedTimer>
#include <algorithm>

BPlusTree::BPlusTree(int order, QObject* parent)
    : QObject(parent), m_order(qMax(3, order)), m_size(0),
      m_root(new Node(true)), m_timeSum(0.0) {}

BPlusTree::~BPlusTree() { clearTree(m_root); }

void BPlusTree::clearTree(Node* node)
{
    if (!node) return;
    if (!node->isLeaf) {
        for (auto* child : node->children)
            clearTree(child);
    }
    delete node;
}

BPlusTree::Node* BPlusTree::findLeaf(int key) const
{
    Node* cur = m_root;
    while (!cur->isLeaf) {
        int idx = 0;
        while (idx < cur->keys.size() && key >= cur->keys[idx])
            ++idx;
        cur = cur->children[idx];
    }
    return cur;
}

void BPlusTree::insert(int key, int value)
{
    QElapsedTimer timer;
    timer.start();

    Node* leaf = findLeaf(key);

    /* 叶子未满，直接插入 */
    if (leaf->keys.size() < m_order - 1) {
        int idx = 0;
        while (idx < leaf->keys.size() && leaf->keys[idx] < key) ++idx;
        /* 重复键更新值 */
        if (idx < leaf->keys.size() && leaf->keys[idx] == key) {
            leaf->values[idx] = value;
            return;
        }
        leaf->keys.insert(idx, key);
        leaf->values.insert(idx, value);
        ++m_size;
        m_stats.totalInserts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum /
            qMax(m_stats.totalInserts + m_stats.totalRemoves +
                 m_stats.totalSearches + m_stats.totalRangeQueries, 1ULL);
        return;
    }

    /* 叶子已满，需分裂 */
    splitLeaf(leaf, key, value);
    ++m_size;
    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInserts + m_stats.totalRemoves +
             m_stats.totalSearches + m_stats.totalRangeQueries, 1ULL);
    emit nodeSplit(key);
}

void BPlusTree::splitLeaf(Node* leaf, int key, int value)
{
    /* 临时数组 */
    QVector<int> tmpK = leaf->keys;
    QVector<int> tmpV = leaf->values;
    int idx = 0;
    while (idx < tmpK.size() && tmpK[idx] < key) ++idx;
    if (idx < tmpK.size() && tmpK[idx] == key) {
        tmpV[idx] = value;
        leaf->keys = tmpK;
        leaf->values = tmpV;
        return;
    }
    tmpK.insert(idx, key);
    tmpV.insert(idx, value);

    int mid = tmpK.size() / 2;
    auto* newLeaf = new Node(true);

    leaf->keys = tmpK.mid(0, mid);
    leaf->values = tmpV.mid(0, mid);
    newLeaf->keys = tmpK.mid(mid);
    newLeaf->values = tmpV.mid(mid);
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    int promotingKey = newLeaf->keys.first();
    splitInternal(leaf, newLeaf, promotingKey);
}

void BPlusTree::splitInternal(Node* node, Node* newChild, int promotingKey)
{
    if (node == m_root) {
        auto* newRoot = new Node(false);
        newRoot->keys.append(promotingKey);
        newRoot->children.append(node);
        newRoot->children.append(newChild);
        node->parent = newRoot;
        newChild->parent = newRoot;
        m_root = newRoot;
        return;
    }

    Node* par = node->parent;
    int idx = 0;
    while (idx < par->children.size() && par->children[idx] != node) ++idx;
    par->keys.insert(idx, promotingKey);
    par->children.insert(idx + 1, newChild);
    newChild->parent = par;

    /* 内部节点溢出(键数 >= order)，继续分裂 */
    if (par->keys.size() >= m_order) {
        int mid = par->keys.size() / 2;
        int upKey = par->keys[mid];

        auto* newInternal = new Node(false);
        newInternal->keys = par->keys.mid(mid + 1);
        newInternal->children = par->children.mid(mid + 1);
        for (auto* c : newInternal->children) c->parent = newInternal;

        par->keys = par->keys.mid(0, mid);
        par->children = par->children.mid(0, mid + 1);

        splitInternal(par, newInternal, upKey);
    }
}

bool BPlusTree::search(int key, int& value)
{
    QElapsedTimer timer;
    timer.start();

    Node* leaf = findLeaf(key);
    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) {
            value = leaf->values[i];
            m_stats.totalSearches++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum /
                qMax(m_stats.totalInserts + m_stats.totalRemoves +
                     m_stats.totalSearches + m_stats.totalRangeQueries, 1ULL);
            return true;
        }
    }

    m_stats.totalSearches++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInserts + m_stats.totalRemoves +
             m_stats.totalSearches + m_stats.totalRangeQueries, 1ULL);
    return false;
}

QVector<QPair<int, int>> BPlusTree::rangeQuery(int lo, int hi)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, int>> result;
    Node* leaf = findLeaf(lo);

    while (leaf) {
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] > hi) {
                m_stats.totalRangeQueries++;
                m_timeSum += timer.elapsed();
                m_stats.avgProcessingTimeMs = m_timeSum /
                    qMax(m_stats.totalInserts + m_stats.totalRemoves +
                         m_stats.totalSearches + m_stats.totalRangeQueries, 1ULL);
                return result;
            }
            if (leaf->keys[i] >= lo)
                result.append({leaf->keys[i], leaf->values[i]});
        }
        leaf = leaf->next;
    }

    m_stats.totalRangeQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInserts + m_stats.totalRemoves +
             m_stats.totalSearches + m_stats.totalRangeQueries, 1ULL);
    return result;
}

bool BPlusTree::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    Node* leaf = findLeaf(key);
    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) {
            leaf->keys.removeAt(i);
            leaf->values.removeAt(i);
            --m_size;
            rebalanceLeaf(leaf, key);
            m_stats.totalRemoves++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum /
                qMax(m_stats.totalInserts + m_stats.totalRemoves +
                     m_stats.totalSearches + m_stats.totalRangeQueries, 1ULL);
            emit nodesMerged(key);
            return true;
        }
    }

    m_stats.totalRemoves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInserts + m_stats.totalRemoves +
             m_stats.totalSearches + m_stats.totalRangeQueries, 1ULL);
    return false;
}

void BPlusTree::rebalanceLeaf(Node* leaf, int key)
{
    int minKeys = (m_order - 1) / 2;
    if (leaf->keys.size() >= minKeys || leaf == m_root) return;

    Node* par = leaf->parent;
    if (!par) return;
    int idx = 0;
    while (idx < par->children.size() && par->children[idx] != leaf) ++idx;

    /* 尝试从左兄弟借 */
    if (idx > 0) {
        Node* left = par->children[idx - 1];
        if (left->keys.size() > minKeys) {
            leaf->keys.prepend(left->keys.takeLast());
            leaf->values.prepend(left->values.takeLast());
            par->keys[idx - 1] = leaf->keys.first();
            return;
        }
    }
    /* 尝试从右兄弟借 */
    if (idx < par->children.size() - 1) {
        Node* right = par->children[idx + 1];
        if (right->keys.size() > minKeys) {
            leaf->keys.append(right->keys.takeFirst());
            leaf->values.append(right->values.takeFirst());
            par->keys[idx] = right->keys.first();
            return;
        }
    }
    /* 合并: 简化处理，合并到左兄弟 */
    if (idx > 0) {
        Node* left = par->children[idx - 1];
        left->keys += leaf->keys;
        left->values += leaf->values;
        left->next = leaf->next;
        par->keys.removeAt(idx - 1);
        par->children.removeAt(idx);
        delete leaf;
        rebalanceInternal(par, idx - 1);
    } else if (idx < par->children.size() - 1) {
        Node* right = par->children[idx + 1];
        leaf->keys += right->keys;
        leaf->values += right->values;
        leaf->next = right->next;
        par->keys.removeAt(idx);
        par->children.removeAt(idx + 1);
        delete right;
        rebalanceInternal(par, idx);
    }
}

void BPlusTree::rebalanceInternal(Node* node, int idx)
{
    if (node == m_root) {
        if (node->keys.isEmpty()) {
            m_root = node->children.first();
            m_root->parent = nullptr;
            delete node;
        }
        return;
    }
    int minKeys = (m_order - 1) / 2;
    if (node->keys.size() >= minKeys) return;
    /* 简化: 不递归继续合并 */
    if (node->parent && node->keys.isEmpty()) {
        Node* par = node->parent;
        m_root = node->children.first();
        m_root->parent = nullptr;
        int pidx = par->children.indexOf(node);
        if (pidx >= 0) par->children.removeAt(pidx);
        delete node;
    }
}

void BPlusTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
