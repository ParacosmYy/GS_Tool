/**
 * @file BTree4.cpp
 * @brief BTree4 实现
 *
 * 实现B树：可配置阶数、分裂/合并、范围扫描、批量顺序插入。
 */

#include "utils/tree176/BTree4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BTree4::BTree4(int minDegree, QObject *parent)
    : QObject(parent), m_t(qMax(2, minDegree))
{
}

BTree4::~BTree4() { clearRec(m_root); }

/* ---- Configuration ---- */

void BTree4::setMinDegree(int t) { m_t = qMax(2, t); }

/* ---- Split child ---- */

void BTree4::splitChild(BNode* parent, int idx)
{
    BNode* full = parent->children[idx];
    BNode* sibling = new BNode;
    sibling->leaf = full->leaf;

    int mid = m_t - 1;

    /* Move upper half keys to sibling */
    for (int i = 0; i < m_t - 1; ++i) {
        sibling->keys.append(full->keys[mid + 1 + i]);
        sibling->values.append(full->values[mid + 1 + i]);
    }

    /* Move upper half children */
    if (!full->leaf) {
        for (int i = 0; i < m_t; ++i)
            sibling->children.append(full->children[mid + 1 + i]);
    }

    /* Trim original node */
    full->keys.resize(mid);
    full->values.resize(mid);
    if (!full->leaf)
        full->children.resize(m_t);
    full->numKeys = mid;

    sibling->numKeys = m_t - 1;

    /* Insert median into parent */
    parent->keys.insert(idx, full->keys[mid]);
    parent->values.insert(idx, full->values[mid]);
    parent->children.insert(idx + 1, sibling);
    parent->numKeys++;

    emit splitPerformed(0);
}

/* ---- Insert non-full ---- */

void BTree4::insertNonFull(BNode* node, double key, double value)
{
    int i = node->numKeys - 1;

    if (node->leaf) {
        /* Find position and insert */
        while (i >= 0 && node->keys[i] > key) --i;

        /* Update if key exists */
        if (i >= 0 && node->keys[i] == key) {
            node->values[i] = value;
            return;
        }

        node->keys.insert(i + 1, key);
        node->values.insert(i + 1, value);
        node->numKeys++;
    } else {
        /* Find child to descend */
        while (i >= 0 && node->keys[i] > key) --i;
        ++i;

        /* Split if child is full */
        if (node->children[i]->numKeys == 2 * m_t - 1) {
            splitChild(node, i);
            if (key > node->keys[i]) ++i;
        }

        insertNonFull(node->children[i], key, value);
    }
}

/* ---- Insert ---- */

bool BTree4::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new BNode;
        m_root->keys.append(key);
        m_root->values.append(value);
        m_root->numKeys = 1;
    } else {
        if (m_root->numKeys == 2 * m_t - 1) {
            /* Root is full, create new root */
            BNode* newRoot = new BNode;
            newRoot->leaf = false;
            newRoot->children.append(m_root);
            splitChild(newRoot, 0);
            m_root = newRoot;
        }
        insertNonFull(m_root, key, value);
    }

    m_stats.totalInserts++;
    m_stats.currentSize++;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.nodeCount = countNodes(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries);

    emit insertCompleted(key, m_stats.currentSize);
    return true;
}

/* ---- Merge children ---- */

void BTree4::mergeChildren(BNode* node, int idx)
{
    BNode* left = node->children[idx];
    BNode* right = node->children[idx + 1];

    /* Pull down separator from parent */
    left->keys.append(node->keys[idx]);
    left->values.append(node->values[idx]);

    /* Append right's keys */
    for (int i = 0; i < right->numKeys; ++i) {
        left->keys.append(right->keys[i]);
        left->values.append(right->values[i]);
    }

    /* Append right's children */
    if (!right->leaf) {
        for (int i = 0; i <= right->numKeys; ++i)
            left->children.append(right->children[i]);
    }

    left->numKeys += 1 + right->numKeys;

    /* Remove separator and right child from parent */
    node->keys.removeAt(idx);
    node->values.removeAt(idx);
    node->children.removeAt(idx + 1);
    node->numKeys--;

    delete right;
}

/* ---- Predecessor / Successor ---- */

QPair<double, double> BTree4::predecessor(BNode* node)
{
    while (!node->leaf)
        node = node->children[node->numKeys];
    return {node->keys[node->numKeys - 1], node->values[node->numKeys - 1]};
}

QPair<double, double> BTree4::successor(BNode* node)
{
    while (!node->leaf)
        node = node->children[0];
    return {node->keys[0], node->values[0]};
}

/* ---- Remove recursive ---- */

bool BTree4::removeRec(BNode* node, double key)
{
    int idx = 0;
    while (idx < node->numKeys && node->keys[idx] < key) ++idx;

    if (idx < node->numKeys && node->keys[idx] == key) {
        /* Key found in this node */
        if (node->leaf) {
            node->keys.removeAt(idx);
            node->values.removeAt(idx);
            node->numKeys--;
            return true;
        }

        /* Internal node */
        if (node->children[idx]->numKeys >= m_t) {
            auto pred = predecessor(node->children[idx]);
            node->keys[idx] = pred.first;
            node->values[idx] = pred.second;
            return removeRec(node->children[idx], pred.first);
        }
        if (node->children[idx + 1]->numKeys >= m_t) {
            auto succ = successor(node->children[idx + 1]);
            node->keys[idx] = succ.first;
            node->values[idx] = succ.second;
            return removeRec(node->children[idx + 1], succ.first);
        }

        mergeChildren(node, idx);
        return removeRec(node->children[idx], key);
    }

    /* Key not in this node */
    if (node->leaf) return false;

    /* Ensure child has at least t keys */
    if (node->children[idx]->numKeys < m_t) {
        if (idx > 0 && node->children[idx - 1]->numKeys >= m_t) {
            /* Borrow from left sibling */
            BNode* child = node->children[idx];
            BNode* leftSib = node->children[idx - 1];
            child->keys.insert(0, node->keys[idx - 1]);
            child->values.insert(0, node->values[idx - 1]);
            node->keys[idx - 1] = leftSib->keys[leftSib->numKeys - 1];
            node->values[idx - 1] = leftSib->values[leftSib->numKeys - 1];
            if (!leftSib->leaf)
                child->children.insert(0, leftSib->children[leftSib->numKeys]);
            leftSib->keys.removeLast();
            leftSib->values.removeLast();
            if (!leftSib->leaf) leftSib->children.removeLast();
            leftSib->numKeys--;
            child->numKeys++;
        } else if (idx < node->numKeys && node->children[idx + 1]->numKeys >= m_t) {
            /* Borrow from right sibling */
            BNode* child = node->children[idx];
            BNode* rightSib = node->children[idx + 1];
            child->keys.append(node->keys[idx]);
            child->values.append(node->values[idx]);
            node->keys[idx] = rightSib->keys[0];
            node->values[idx] = rightSib->values[0];
            if (!rightSib->leaf)
                child->children.append(rightSib->children[0]);
            rightSib->keys.removeFirst();
            rightSib->values.removeFirst();
            if (!rightSib->leaf) rightSib->children.removeFirst();
            rightSib->numKeys--;
            child->numKeys++;
        } else {
            if (idx < node->numKeys)
                mergeChildren(node, idx);
            else
                mergeChildren(node, idx - 1);
        }
    }

    return removeRec(node->children[qMin(idx, node->numKeys)], key);
}

/* ---- Remove ---- */

bool BTree4::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return false;

    bool removed = removeRec(m_root, key);
    if (removed) {
        if (m_root->numKeys == 0 && !m_root->leaf) {
            BNode* old = m_root;
            m_root = m_root->children[0];
            old->children.clear();
            delete old;
        }
        m_stats.currentSize--;
    }

    m_stats.totalDeletes++;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.nodeCount = countNodes(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries);

    emit deleteCompleted(key);
    return removed;
}

/* ---- Find ---- */

bool BTree4::find(double key, double& value) const
{
    BNode* cur = m_root;
    while (cur) {
        int i = 0;
        while (i < cur->numKeys && cur->keys[i] < key) ++i;
        if (i < cur->numKeys && cur->keys[i] == key) {
            value = cur->values[i];
            return true;
        }
        if (cur->leaf) return false;
        cur = cur->children[i];
    }
    return false;
}

/* ---- Range scan ---- */

void BTree4::rangeRec(BNode* node, double lo, double hi,
                       QVector<QPair<double, double>>& result)
{
    if (!node) return;
    int i = 0;
    while (i < node->numKeys && node->keys[i] < lo) ++i;
    for (; i < node->numKeys && node->keys[i] <= hi; ++i) {
        if (!node->leaf)
            rangeRec(node->children[i], lo, hi, result);
        result.append({node->keys[i], node->values[i]});
    }
    if (!node->leaf && i <= node->numKeys)
        rangeRec(node->children[qMin(i, node->children.size() - 1)], lo, hi, result);
}

QVector<QPair<double, double>> BTree4::rangeScan(double keyLo, double keyHi) const
{
    QVector<QPair<double, double>> result;
    rangeRec(m_root, keyLo, keyHi, result);
    return result;
}

/* ---- Bulk insert ---- */

void BTree4::bulkInsert(const QVector<QPair<double, double>>& items)
{
    QVector<QPair<double, double>> sorted = items;
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    for (const auto& item : sorted)
        insert(item.first, item.second);
    emit bulkInsertCompleted(sorted.size());
}

/* ---- In-order ---- */

void BTree4::inOrderRec(BNode* node, QVector<QPair<double, double>>& result)
{
    if (!node) return;
    for (int i = 0; i < node->numKeys; ++i) {
        if (!node->leaf && i < node->children.size())
            inOrderRec(node->children[i], result);
        result.append({node->keys[i], node->values[i]});
    }
    if (!node->leaf && node->children.size() > node->numKeys)
        inOrderRec(node->children[node->numKeys], result);
}

QVector<QPair<double, double>> BTree4::inOrder() const
{
    QVector<QPair<double, double>> result;
    inOrderRec(m_root, result);
    return result;
}

/* ---- Level-order ---- */

QVector<QVector<QPair<double, double>>> BTree4::levelOrder() const
{
    QVector<QVector<QPair<double, double>>> levels;
    if (!m_root) return levels;

    QVector<BNode*> current, next;
    current.append(m_root);
    while (!current.isEmpty()) {
        QVector<QPair<double, double>> level;
        next.clear();
        for (BNode* n : current) {
            for (int i = 0; i < n->numKeys; ++i)
                level.append({n->keys[i], n->values[i]});
            for (BNode* c : n->children)
                next.append(c);
        }
        levels.append(level);
        current = next;
    }
    return levels;
}

/* ---- Utility ---- */

void BTree4::clearRec(BNode* node)
{
    if (!node) return;
    for (BNode* c : node->children) clearRec(c);
    delete node;
}

int BTree4::computeHeight(BNode* node)
{
    if (!node) return 0;
    return 1 + computeHeight(node->children.isEmpty() ? nullptr : node->children[0]);
}

int BTree4::countNodes(BNode* node)
{
    if (!node) return 0;
    int count = 1;
    for (BNode* c : node->children) count += countNodes(c);
    return count;
}

bool BTree4::isEmpty() const { return m_root == nullptr; }

void BTree4::clear()
{
    clearRec(m_root);
    m_root = nullptr;
    m_stats.currentSize = 0;
    m_stats.treeHeight = 0;
    m_stats.nodeCount = 0;
}

int BTree4::size() const { return m_stats.currentSize; }
int BTree4::height() const { return m_stats.treeHeight; }

void BTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
