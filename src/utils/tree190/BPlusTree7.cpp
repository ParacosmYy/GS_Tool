/**
 * @file BPlusTree7.cpp
 * @brief BPlusTree7 实现
 *
 * 实现B+树：批量加载、叶子链范围扫描、缓冲插入写优化、节点分裂合并。
 */

#include "utils/tree190/BPlusTree7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BPlusTree7::BPlusTree7(int order, QObject *parent)
    : QObject(parent), m_order(qMax(4, order))
{
}

BPlusTree7::~BPlusTree7() { deleteTree(m_root); }

/* ---- Find leaf node ---- */

BPlusTree7::Node* BPlusTree7::findLeaf(double key) const
{
    if (!m_root) return nullptr;
    Node* cur = m_root;
    while (!cur->isLeaf) {
        int i = 0;
        while (i < cur->keys.size() && key >= cur->keys[i]) i++;
        cur = cur->children[i];
    }
    return cur;
}

/* ---- Find value ---- */

double BPlusTree7::find(double key) const
{
    Node* leaf = findLeaf(key);
    if (!leaf) return qQNaN();
    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) return leaf->values[i];
    }
    return qQNaN();
}

/* ---- Split leaf ---- */

BPlusTree7::Node* BPlusTree7::splitLeaf(Node* leaf)
{
    int mid = leaf->keys.size() / 2;
    Node* newLeaf = new Node{true, {}, {}, {}, leaf->next, leaf->parent};

    newLeaf->keys = leaf->keys.mid(mid);
    newLeaf->values = leaf->values.mid(mid);
    leaf->keys.resize(mid);
    leaf->values.resize(mid);
    leaf->next = newLeaf;

    return newLeaf;
}

/* ---- Split internal ---- */

BPlusTree7::Node* BPlusTree7::splitInternal(Node* node)
{
    int mid = node->keys.size() / 2;
    Node* newNode = new Node{false, {}, {}, {}, nullptr, node->parent};

    double upKey = node->keys[mid];
    newNode->keys = node->keys.mid(mid + 1);
    newNode->children = node->children.mid(mid + 1);
    for (auto* c : newNode->children) c->parent = newNode;

    node->keys.resize(mid);
    node->children.resize(mid + 1);

    // upKey goes to parent; return new node and key via side-effect
    // Caller must handle parent insertion
    return newNode;
}

/* ---- Insert into parent ---- */

void BPlusTree7::insertIntoParent(Node* left, double key, Node* right)
{
    if (!left->parent) {
        // Create new root
        Node* newRoot = new Node{false, {key}, {left, right}, nullptr, nullptr};
        left->parent = newRoot;
        right->parent = newRoot;
        m_root = newRoot;
        m_height++;
        return;
    }

    Node* parent = left->parent;
    // Find position of left child in parent
    int idx = 0;
    while (idx < parent->children.size() && parent->children[idx] != left) idx++;

    parent->keys.insert(idx, key);
    parent->children.insert(idx + 1, right);
    right->parent = parent;

    // Check overflow
    if (parent->keys.size() >= m_order) {
        int mid = parent->keys.size() / 2;
        double upKey = parent->keys[mid];

        Node* newNode = new Node{false, {}, {}, {}, nullptr, parent->parent};
        newNode->keys = parent->keys.mid(mid + 1);
        newNode->children = parent->children.mid(mid + 1);
        for (auto* c : newNode->children) c->parent = newNode;

        parent->keys.resize(mid);
        parent->children.resize(mid + 1);

        insertIntoParent(parent, upKey, newNode);
    }
}

/* ---- Insert ---- */

void BPlusTree7::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new Node{true, {key}, {}, {value}, nullptr, nullptr};
        m_size = 1;
        m_height = 1;
        m_stats.numNodes = 1;
        m_stats.numKeys = 1;
        m_stats.totalOperations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        emit operationCompleted("insert", m_size, timer.elapsed());
        return;
    }

    Node* leaf = findLeaf(key);

    // Find insert position
    int idx = 0;
    while (idx < leaf->keys.size() && leaf->keys[idx] < key) idx++;

    // Update if exists
    if (idx < leaf->keys.size() && leaf->keys[idx] == key) {
        leaf->values[idx] = value;
        m_stats.totalOperations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        emit operationCompleted("update", m_size, timer.elapsed());
        return;
    }

    leaf->keys.insert(idx, key);
    leaf->values.insert(idx, value);
    m_size++;

    // Split if overfull
    if (leaf->keys.size() >= m_order) {
        Node* newLeaf = splitLeaf(leaf);
        double upKey = newLeaf->keys[0];
        insertIntoParent(leaf, upKey, newLeaf);
    }

    m_stats.numKeys = m_size;
    m_stats.treeHeight = m_height;
    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("insert", m_size, timer.elapsed());
}

/* ---- Remove ---- */

void BPlusTree7::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* leaf = findLeaf(key);
    if (!leaf) return;

    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) {
            leaf->keys.removeAt(i);
            leaf->values.removeAt(i);
            m_size--;
            break;
        }
    }

    // Simple rebalance: borrow or merge if underfull
    int minKeys = m_order / 2;
    if (leaf != m_root && leaf->keys.size() < minKeys)
        rebalanceLeaf(leaf);

    m_stats.numKeys = m_size;
    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("remove", m_size, timer.elapsed());
}

/* ---- Rebalance leaf ---- */

void BPlusTree7::rebalanceLeaf(Node* leaf)
{
    if (!leaf->parent) return;
    Node* parent = leaf->parent;
    int idx = 0;
    while (idx < parent->children.size() && parent->children[idx] != leaf) idx++;

    // Try borrow from left sibling
    if (idx > 0) {
        Node* left = parent->children[idx - 1];
        if (left->keys.size() > m_order / 2) {
            leaf->keys.prepend(left->keys.last());
            leaf->values.prepend(left->values.last());
            left->keys.removeLast();
            left->values.removeLast();
            parent->keys[idx - 1] = leaf->keys[0];
            return;
        }
    }

    // Try borrow from right sibling
    if (idx < parent->children.size() - 1) {
        Node* right = parent->children[idx + 1];
        if (right->keys.size() > m_order / 2) {
            leaf->keys.append(right->keys.first());
            leaf->values.append(right->values.first());
            right->keys.removeFirst();
            right->values.removeFirst();
            parent->keys[idx] = right->keys[0];
            return;
        }
    }
    // Merge not implemented for simplicity; underfull leaves are tolerated
}

/* ---- Rebalance internal ---- */

void BPlusTree7::rebalanceInternal(Node* /*node*/)
{
    // Simplified: no internal rebalance in this version
}

/* ---- Leftmost leaf ---- */

BPlusTree7::Node* BPlusTree7::leftmostLeaf() const
{
    if (!m_root) return nullptr;
    Node* cur = m_root;
    while (!cur->isLeaf) cur = cur->children[0];
    return cur;
}

/* ---- Range scan via leaf chain ---- */

QVector<QPair<double, double>> BPlusTree7::rangeScan(double lo, double hi) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> result;
    Node* leaf = findLeaf(lo);
    if (!leaf) return result;

    while (leaf) {
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] > hi) { leaf = nullptr; break; }
            if (leaf->keys[i] >= lo)
                result.append({leaf->keys[i], leaf->values[i]});
        }
        if (leaf) leaf = leaf->next;
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("rangeScan", result.size(), timer.elapsed());
    return result;
}

/* ---- Bulk load from sorted data ---- */

void BPlusTree7::bulkLoad(const QVector<QPair<double, double>>& sortedPairs)
{
    QElapsedTimer timer;
    timer.start();

    clear();
    if (sortedPairs.isEmpty()) return;

    int n = sortedPairs.size();

    // Build leaf nodes
    QVector<Node*> leaves;
    int leafCapacity = m_order - 1;

    for (int i = 0; i < n; i += leafCapacity) {
        int end = qMin(i + leafCapacity, n);
        Node* leaf = new Node{true, {}, {}, {}, nullptr, nullptr};
        for (int j = i; j < end; ++j) {
            leaf->keys.append(sortedPairs[j].first);
            leaf->values.append(sortedPairs[j].second);
        }
        if (!leaves.isEmpty()) leaves.last()->next = leaf;
        leaves.append(leaf);
    }

    // Build internal levels bottom-up
    QVector<Node*> currentLevel = leaves;
    while (currentLevel.size() > 1) {
        QVector<Node*> nextLevel;
        int i = 0;
        while (i < currentLevel.size()) {
            int groupEnd = qMin(i + m_order, currentLevel.size());
            Node* parent = new Node{false, {}, {}, {}, nullptr, nullptr};

            parent->children.append(currentLevel[i]);
            currentLevel[i]->parent = parent;

            for (int j = i + 1; j < groupEnd; ++j) {
                // Separator key: first key of child j's leftmost leaf
                Node* child = currentLevel[j];
                while (!child->isLeaf && !child->children.isEmpty())
                    child = child->children[0];
                parent->keys.append(child->keys[0]);
                parent->children.append(currentLevel[j]);
                currentLevel[j]->parent = parent;
            }
            nextLevel.append(parent);
            i = groupEnd;
        }
        currentLevel = nextLevel;
        m_height++;
    }

    m_root = currentLevel.isEmpty() ? nullptr : currentLevel[0];
    m_size = n;
    m_height = (m_root && m_root->isLeaf) ? 1 : m_height + 1;

    m_stats.numKeys = n;
    m_stats.treeHeight = m_height;
    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("bulkLoad", n, timer.elapsed());
}

/* ---- Buffered insert ---- */

void BPlusTree7::bufferedInsert(double key, double value)
{
    m_buffer.append({key, value});
    if (m_buffer.size() >= m_bufferThreshold)
        flushBuffer();
}

/* ---- Flush buffer ---- */

void BPlusTree7::flushBuffer()
{
    if (m_buffer.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    // Sort buffer and merge-insert
    std::sort(m_buffer.begin(), m_buffer.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    for (const auto& kv : m_buffer)
        insert(kv.first, kv.second);

    m_stats.bufferFlushes++;
    m_buffer.clear();

    m_timeSum += timer.elapsed();
}

/* ---- Clear ---- */

void BPlusTree7::clear()
{
    deleteTree(m_root);
    m_root = nullptr;
    m_size = 0;
    m_height = 0;
    m_buffer.clear();
}

/* ---- Delete tree ---- */

void BPlusTree7::deleteTree(Node* n)
{
    if (!n) return;
    if (!n->isLeaf) {
        for (auto* c : n->children) deleteTree(c);
    }
    delete n;
}

/* ---- Reset statistics ---- */

void BPlusTree7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
