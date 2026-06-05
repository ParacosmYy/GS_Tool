/**
 * @file BPlusTree4.cpp
 * @brief B+树增强实现 — 范围查询/批量加载/延迟分裂/节点压缩
 */

#include "utils/tree31/BPlusTree4.h"

#include <QtAlgorithms>
#include <algorithm>
#include <queue>
#include <cstring>

BPlusTree4::BPlusTree4(int order, bool enablePrefixCompression, QObject* parent)
    : QObject(parent), m_order(qMax(4, order)), m_compress(enablePrefixCompression),
      m_root(new Node(true))
{
}

BPlusTree4::~BPlusTree4()
{
    deleteTree(m_root);
}

void BPlusTree4::insert(int key, int value)
{
    m_timing.start();
    ++m_stats.totalInsertions;
    bool causedSplit = false;

    Node* leaf = findLeaf(key);
    /* 延迟分裂: 如果节点满但还能插入(不超过order+1)，先插入 */
    int maxKeys = m_order - 1;
    bool overflow = (leaf->keys.size() >= m_order);

    /* 插入到叶节点 */
    int pos = 0;
    while (pos < leaf->keys.size() && leaf->keys[pos] < key) ++pos;
    leaf->keys.insert(pos, key);
    leaf->values.insert(pos, value);

    /* 延迟分裂标记 */
    if (overflow) {
        leaf->delayedSplit = true;
    }

    /* 实际分裂: 超过order时执行 */
    if (leaf->keys.size() > m_order) {
        Node* newLeaf = splitLeaf(leaf);
        ++m_stats.totalSplits;
        causedSplit = true;
        insertIntoParent(leaf, newLeaf->keys[0], newLeaf);
    }

    if (m_compress) compressKeys(leaf);
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInsertions > 0)
        ? m_timeSum / m_stats.totalInsertions : 0.0;
    emit inserted(key, causedSplit);
}

QVector<int> BPlusTree4::query(int key) const
{
    ++m_stats.totalQueries;
    Node* leaf = findLeaf(key);
    QVector<int> result;
    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key)
            result.append(leaf->values[i]);
    }
    return result;
}

QVector<QPair<int, int>> BPlusTree4::rangeQuery(int lower, int upper) const
{
    m_timing.start();
    ++m_stats.totalRangeQueries;

    QVector<QPair<int, int>> result;
    Node* leaf = findLeaf(lower);
    while (leaf) {
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] > upper) {
                /* 收集结果完成 */
                const_cast<BPlusTree4*>(this)->m_timeSum += const_cast<BPlusTree4*>(this)->m_timing.elapsed();
                emit const_cast<BPlusTree4*>(this)->rangeQueryComplete(
                    result.size(), 0);
                return result;
            }
            if (leaf->keys[i] >= lower)
                result.append({leaf->keys[i], leaf->values[i]});
        }
        leaf = leaf->next;
    }
    const_cast<BPlusTree4*>(this)->m_timeSum += const_cast<BPlusTree4*>(this)->m_timing.elapsed();
    emit const_cast<BPlusTree4*>(this)->rangeQueryComplete(result.size(), 0);
    return result;
}

void BPlusTree4::bulkLoad(const QVector<int>& keys, const QVector<int>& values)
{
    m_timing.start();
    deleteTree(m_root);
    m_root = new Node(true);
    int n = qMin(keys.size(), values.size());
    if (n == 0) return;

    /* 从有序数据构建叶节点链表 */
    int maxKeys = m_order - 1;
    QVector<Node*> leaves;
    Node* prevLeaf = nullptr;
    for (int i = 0; i < n; i += maxKeys) {
        Node* leaf = new Node(true);
        int end = qMin(i + maxKeys, n);
        for (int j = i; j < end; ++j) {
            leaf->keys.append(keys[j]);
            leaf->values.append(values[j]);
        }
        leaf->next = nullptr;
        if (prevLeaf) prevLeaf->next = leaf;
        prevLeaf = leaf;
        leaves.append(leaf);
    }

    /* 逐层构建内部节点 */
    QVector<Node*> currentLevel = leaves;
    while (currentLevel.size() > 1) {
        QVector<Node*> nextLevel;
        for (int i = 0; i < currentLevel.size(); i += m_order) {
            Node* parent = new Node(false);
            int end = qMin(i + m_order, currentLevel.size());
            for (int j = i; j < end; ++j) {
                parent->children.append(currentLevel[j]);
                currentLevel[j]->parent = parent;
                if (j > i) {
                    /* 取子节点的最小键作为分隔键 */
                    parent->keys.append(currentLevel[j]->keys[0]);
                }
            }
            nextLevel.append(parent);
        }
        currentLevel = nextLevel;
    }
    m_root = currentLevel[0];
    m_root->parent = nullptr;

    emit bulkLoadComplete(n, height());
}

bool BPlusTree4::remove(int key)
{
    Node* leaf = findLeaf(key);
    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) {
            leaf->keys.erase(leaf->keys.begin() + i);
            leaf->values.erase(leaf->values.begin() + i);
            /* 如果节点太空则合并 */
            int minKeys = m_order / 2;
            if (leaf != m_root && leaf->keys.size() < minKeys) {
                mergeOrRedistribute(leaf);
                ++m_stats.totalMerges;
            }
            return true;
        }
    }
    return false;
}

int BPlusTree4::height() const
{
    return computeHeight(m_root);
}

int BPlusTree4::nodeCount() const
{
    return countNodes(m_root);
}

void BPlusTree4::clear()
{
    deleteTree(m_root);
    m_root = new Node(true);
}

BPlusTree4::Node* BPlusTree4::findLeaf(int key) const
{
    Node* node = m_root;
    while (!node->isLeaf) {
        int i = 0;
        while (i < node->keys.size() && key >= node->keys[i]) ++i;
        node = node->children[i];
    }
    return node;
}

BPlusTree4::Node* BPlusTree4::splitLeaf(Node* leaf)
{
    int mid = leaf->keys.size() / 2;
    Node* newLeaf = new Node(true);
    newLeaf->keys = leaf->keys.mid(mid);
    newLeaf->values = leaf->values.mid(mid);
    leaf->keys.resize(mid);
    leaf->values.resize(mid);
    /* 维护叶节点链表 */
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;
    newLeaf->parent = leaf->parent;
    leaf->delayedSplit = false;
    return newLeaf;
}

BPlusTree4::Node* BPlusTree4::splitInternal(Node* node)
{
    int mid = node->keys.size() / 2;
    Node* newNode = new Node(false);
    /* 中间键提升到父节点 */
    int promoteKey = node->keys[mid];
    newNode->keys = node->keys.mid(mid + 1);
    newNode->children = node->children.mid(mid + 1);
    for (Node* child : newNode->children) child->parent = newNode;
    node->keys.resize(mid);
    node->children.resize(mid + 1);
    /* 插入到父节点 */
    if (!node->parent) {
        Node* newRoot = new Node(false);
        newRoot->keys.append(promoteKey);
        newRoot->children.append(node);
        newRoot->children.append(newNode);
        node->parent = newRoot;
        newNode->parent = newRoot;
        m_root = newRoot;
    } else {
        newNode->parent = node->parent;
        insertIntoParent(node, promoteKey, newNode);
    }
    return newNode;
}

void BPlusTree4::insertIntoParent(Node* left, int key, Node* right)
{
    Node* parent = left->parent;
    if (!parent) {
        /* 创建新根 */
        parent = new Node(false);
        parent->keys.append(key);
        parent->children.append(left);
        parent->children.append(right);
        left->parent = parent;
        right->parent = parent;
        m_root = parent;
        return;
    }
    int pos = 0;
    while (pos < parent->keys.size() && parent->keys[pos] < key) ++pos;
    parent->keys.insert(pos, key);
    parent->children.insert(pos + 1, right);
    right->parent = parent;

    if (parent->keys.size() >= m_order) {
        splitInternal(parent);
        ++m_stats.totalSplits;
    }
}

void BPlusTree4::mergeOrRedistribute(Node* node)
{
    if (node == m_root) {
        if (node->children.size() == 1) {
            m_root = node->children[0];
            m_root->parent = nullptr;
            delete node;
        }
        return;
    }
    Node* parent = node->parent;
    int idx = parent->children.indexOf(node);
    /* 尝试从左兄弟借 */
    if (idx > 0) {
        Node* leftSib = parent->children[idx - 1];
        if (leftSib->keys.size() > m_order / 2) {
            if (node->isLeaf) {
                node->keys.prepend(leftSib->keys.last());
                node->values.prepend(leftSib->values.last());
                leftSib->keys.removeLast();
                leftSib->values.removeLast();
                parent->keys[idx - 1] = node->keys[0];
            } else {
                node->keys.prepend(parent->keys[idx - 1]);
                parent->keys[idx - 1] = leftSib->keys.last();
                leftSib->keys.removeLast();
                node->children.prepend(leftSib->children.last());
                leftSib->children.removeLast();
                node->children[0]->parent = node;
            }
            return;
        }
    }
    /* 尝试从右兄弟借 */
    if (idx < parent->children.size() - 1) {
        Node* rightSib = parent->children[idx + 1];
        if (rightSib->keys.size() > m_order / 2) {
            if (node->isLeaf) {
                node->keys.append(rightSib->keys.first());
                node->values.append(rightSib->values.first());
                rightSib->keys.erase(rightSib->keys.begin());
                rightSib->values.erase(rightSib->values.begin());
                parent->keys[idx] = rightSib->keys[0];
            } else {
                node->keys.append(parent->keys[idx]);
                parent->keys[idx] = rightSib->keys.first();
                rightSib->keys.erase(rightSib->keys.begin());
                node->children.append(rightSib->children.first());
                rightSib->children.erase(rightSib->children.begin());
                node->children.last()->parent = node;
            }
            return;
        }
    }
    /* 合并(简化: 合并到左节点) */
    if (idx > 0) {
        Node* leftSib = parent->children[idx - 1];
        if (node->isLeaf) {
            leftSib->keys.append(node->keys);
            leftSib->values.append(node->values);
            leftSib->next = node->next;
        } else {
            leftSib->keys.append(parent->keys[idx - 1]);
            leftSib->keys.append(node->keys);
            leftSib->children.append(node->children);
            for (Node* c : node->children) c->parent = leftSib;
        }
        parent->keys.erase(parent->keys.begin() + idx - 1);
        parent->children.erase(parent->children.begin() + idx);
        delete node;
    }
}

int BPlusTree4::computeHeight(Node* node) const
{
    if (!node) return 0;
    if (node->isLeaf) return 1;
    return 1 + computeHeight(node->children[0]);
}

int BPlusTree4::countNodes(Node* node) const
{
    if (!node) return 0;
    int count = 1;
    for (Node* child : node->children)
        count += countNodes(child);
    return count;
}

void BPlusTree4::deleteTree(Node* node)
{
    if (!node) return;
    for (Node* child : node->children)
        deleteTree(child);
    delete node;
}

void BPlusTree4::compressKeys(Node* node)
{
    /* 前缀压缩: 存储相邻键的公共前缀长度+差异后缀 */
    if (!m_compress || node->keys.size() < 2) return;
    /* 简化实现: 对整数键存储差值而非绝对值 */
    for (int i = node->keys.size() - 1; i > 0; --i)
        node->keys[i] -= node->keys[i - 1];
}

void BPlusTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
