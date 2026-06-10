/**
 * @file BPlusTree12.cpp
 * @brief BPlusTree12 实现
 *
 * 实现B+树：排序输入批量加载链式叶子高效范围查询扫描。
 */

#include "utils/tree263/BPlusTree12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BPlusTree12::BPlusTree12(int order, QObject *parent)
    : QObject(parent), m_order(qMax(3, order)) {}

BPlusTree12::~BPlusTree12() { clearTree(m_root); }

/* ---- Find leaf for key ---- */

BPlusTree12::Node* BPlusTree12::findLeaf(int key) const
{
    if (!m_root) return nullptr;
    Node* cur = m_root;
    while (!cur->isLeaf) {
        int idx = 0;
        while (idx < cur->keys.size() && key >= cur->keys[idx])
            idx++;
        cur = cur->children[idx];
    }
    return cur;
}

/* ---- Split leaf ---- */

BPlusTree12::Node* BPlusTree12::splitLeaf(Node* leaf)
{
    int mid = leaf->keys.size() / 2;
    Node* newLeaf = new Node;
    newLeaf->isLeaf = true;

    newLeaf->keys = leaf->keys.mid(mid);
    newLeaf->values = leaf->values.mid(mid);
    leaf->keys.resize(mid);
    leaf->values.resize(mid);

    // Maintain leaf chain
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;
    newLeaf->parent = leaf->parent;

    // Insert split key into parent
    insertIntoParent(leaf, newLeaf->keys[0], newLeaf);
    return newLeaf;
}

/* ---- Split internal ---- */

BPlusTree12::Node* BPlusTree12::splitInternal(Node* node)
{
    int mid = node->keys.size() / 2;
    int upKey = node->keys[mid];

    Node* newNode = new Node;
    newNode->isLeaf = false;

    newNode->keys = node->keys.mid(mid + 1);
    newNode->children = node->children.mid(mid + 1);
    node->keys.resize(mid);
    node->children.resize(mid + 1);

    for (auto* child : newNode->children)
        child->parent = newNode;

    insertIntoParent(node, upKey, newNode);
    return newNode;
}

/* ---- Insert into parent ---- */

void BPlusTree12::insertIntoParent(Node* left, int key, Node* right)
{
    if (left == m_root) {
        Node* newRoot = new Node;
        newRoot->isLeaf = false;
        newRoot->keys.append(key);
        newRoot->children.append(left);
        newRoot->children.append(right);
        left->parent = newRoot;
        right->parent = newRoot;
        m_root = newRoot;
        return;
    }

    Node* parent = left->parent;
    int idx = 0;
    while (idx < parent->keys.size() && key > parent->keys[idx])
        idx++;

    parent->keys.insert(idx, key);
    parent->children.insert(idx + 1, right);
    right->parent = parent;

    // Check if parent needs splitting
    if (parent->children.size() > m_order)
        splitInternal(parent);
}

/* ---- Insert ---- */

void BPlusTree12::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new Node;
        m_root->isLeaf = true;
        m_root->keys.append(key);
        m_root->values.append(value);
        m_leftmostLeaf = m_root;
        m_size++;
    } else {
        Node* leaf = findLeaf(key);
        if (!leaf) return;

        // Find insertion position
        int idx = 0;
        while (idx < leaf->keys.size() && leaf->keys[idx] < key)
            idx++;

        if (idx < leaf->keys.size() && leaf->keys[idx] == key) {
            leaf->values[idx] = value;  // Update existing
        } else {
            leaf->keys.insert(idx, key);
            leaf->values.insert(idx, value);
            m_size++;

            // Check overflow
            if (leaf->keys.size() >= m_order)
                splitLeaf(leaf);
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numKeys = m_size;
    m_stats.numNodes = countNodes(m_root);
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.order = m_order;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_size, m_stats.treeHeight, m_stats.numNodes, elapsed);
}

/* ---- Remove ---- */

void BPlusTree12::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    Node* leaf = findLeaf(key);
    if (!leaf) return;

    int idx = leaf->keys.indexOf(key);
    if (idx < 0) return;

    removeFromLeaf(leaf, idx);

    double elapsed = timer.elapsed();
    m_stats.numKeys = m_size;
    m_stats.numNodes = countNodes(m_root);
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_size, m_stats.treeHeight, m_stats.numNodes, elapsed);
}

void BPlusTree12::removeFromLeaf(Node* leaf, int idx)
{
    leaf->keys.removeAt(idx);
    leaf->values.removeAt(idx);
    m_size--;

    // Handle underflow
    if (leaf != m_root && leaf->keys.size() < (m_order - 1) / 2)
        handleUnderflow(leaf);
}

void BPlusTree12::handleUnderflow(Node* node)
{
    if (!node->parent) return;

    Node* parent = node->parent;
    int idx = parent->children.indexOf(node);

    // Try borrow from left sibling
    if (idx > 0 && parent->children[idx - 1]->keys.size() > (m_order - 1) / 2) {
        Node* sib = parent->children[idx - 1];
        if (node->isLeaf) {
            int bk = sib->keys.takeLast();
            double bv = sib->values.takeLast();
            node->keys.prepend(bk);
            node->values.prepend(bv);
            parent->keys[idx - 1] = node->keys[0];
        } else {
            node->keys.prepend(parent->keys[idx - 1]);
            parent->keys[idx - 1] = sib->keys.takeLast();
            Node* child = sib->children.takeLast();
            child->parent = node;
            node->children.prepend(child);
        }
        return;
    }

    // Try borrow from right sibling
    if (idx < parent->children.size() - 1 &&
        parent->children[idx + 1]->keys.size() > (m_order - 1) / 2) {
        Node* sib = parent->children[idx + 1];
        if (node->isLeaf) {
            int bk = sib->keys.takeFirst();
            double bv = sib->values.takeFirst();
            node->keys.append(bk);
            node->values.append(bv);
            parent->keys[idx] = sib->keys[0];
        } else {
            node->keys.append(parent->keys[idx]);
            parent->keys[idx] = sib->keys.takeFirst();
            Node* child = sib->children.takeFirst();
            child->parent = node;
            node->children.append(child);
        }
        return;
    }

    // Merge with sibling (simplified: merge into left)
    if (idx > 0) {
        Node* sib = parent->children[idx - 1];
        if (node->isLeaf) {
            sib->keys += node->keys;
            sib->values += node->values;
            sib->next = node->next;
        } else {
            sib->keys.append(parent->keys[idx - 1]);
            sib->keys += node->keys;
            sib->children += node->children;
            for (auto* c : node->children) c->parent = sib;
        }
        parent->keys.removeAt(idx - 1);
        parent->children.removeAt(idx);
        delete node;
        if (parent == m_root && parent->children.size() == 1) {
            m_root = parent->children[0];
            m_root->parent = nullptr;
            delete parent;
        } else if (parent != m_root && parent->keys.size() < (m_order - 1) / 2) {
            handleUnderflow(parent);
        }
    }
}

/* ---- Search ---- */

double BPlusTree12::search(int key) const
{
    Node* leaf = findLeaf(key);
    if (!leaf) return qQNaN();
    int idx = leaf->keys.indexOf(key);
    return (idx >= 0) ? leaf->values[idx] : qQNaN();
}

bool BPlusTree12::contains(int key) const
{
    Node* leaf = findLeaf(key);
    return leaf && leaf->keys.contains(key);
}

/* ---- Range query via leaf chain scan ---- */

QVector<QPair<int, double>> BPlusTree12::rangeQuery(int lo, int hi) const
{
    QVector<QPair<int, double>> result;
    Node* leaf = findLeaf(lo);

    while (leaf) {
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] > hi) return result;
            if (leaf->keys[i] >= lo)
                result.append(qMakePair(leaf->keys[i], leaf->values[i]));
        }
        leaf = leaf->next;
    }
    return result;
}

/* ---- Bulk load from sorted data ---- */

void BPlusTree12::bulkLoad(const QVector<QPair<int, double>>& sortedData)
{
    QElapsedTimer timer;
    timer.start();

    clearTree(m_root);
    m_root = nullptr;
    m_size = 0;

    if (sortedData.isEmpty()) return;

    // Create leaf nodes from sorted data
    int maxLeafKeys = m_order - 1;
    QVector<Node*> leaves;

    for (int i = 0; i < sortedData.size(); i += maxLeafKeys) {
        Node* leaf = new Node;
        leaf->isLeaf = true;
        int end = qMin(i + maxLeafKeys, sortedData.size());
        for (int j = i; j < end; ++j) {
            leaf->keys.append(sortedData[j].first);
            leaf->values.append(sortedData[j].second);
        }
        if (!leaves.isEmpty())
            leaves.last()->next = leaf;
        leaves.append(leaf);
        m_size += end - i;
    }

    m_leftmostLeaf = leaves[0];

    // Build internal levels bottom-up
    QVector<Node*> currentLevel = leaves;
    while (currentLevel.size() > 1) {
        QVector<Node*> nextLevel;
        int maxKeys = m_order - 1;

        for (int i = 0; i < currentLevel.size(); i += maxKeys + 1) {
            int end = qMin(i + maxKeys + 1, currentLevel.size());
            Node* parent = new Node;
            parent->isLeaf = false;

            parent->children.append(currentLevel[i]);
            currentLevel[i]->parent = parent;

            for (int j = i + 1; j < end; ++j) {
                // First key of child becomes separator
                Node* child = currentLevel[j];
                int sepKey = child->isLeaf ? child->keys[0] : child->keys[0];
                parent->keys.append(sepKey);
                parent->children.append(child);
                child->parent = parent;
            }
            nextLevel.append(parent);
        }
        currentLevel = nextLevel;
    }

    m_root = currentLevel[0];

    double elapsed = timer.elapsed();
    m_stats.numKeys = m_size;
    m_stats.numNodes = countNodes(m_root);
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.order = m_order;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_size, m_stats.treeHeight, m_stats.numNodes, elapsed);
}

/* ---- Keys in sorted order ---- */

QVector<int> BPlusTree12::keys() const
{
    QVector<int> result;
    Node* leaf = m_leftmostLeaf;
    while (leaf) {
        result += leaf->keys;
        leaf = leaf->next;
    }
    return result;
}

/* ---- Helpers ---- */

BPlusTree12::Node* BPlusTree12::findLeftmost() const
{
    if (!m_root) return nullptr;
    Node* cur = m_root;
    while (!cur->isLeaf) cur = cur->children[0];
    return cur;
}

int BPlusTree12::computeHeight(Node* root) const
{
    if (!root) return 0;
    int h = 1;
    Node* cur = root;
    while (!cur->isLeaf) {
        cur = cur->children[0];
        h++;
    }
    return h;
}

int BPlusTree12::height() const { return computeHeight(m_root); }
int BPlusTree12::size() const { return m_size; }

int BPlusTree12::countNodes(Node* node) const
{
    if (!node) return 0;
    int count = 1;
    if (!node->isLeaf) {
        for (auto* child : node->children)
            count += countNodes(child);
    }
    return count;
}

void BPlusTree12::clearTree(Node* node)
{
    if (!node) return;
    if (!node->isLeaf) {
        for (auto* child : node->children)
            clearTree(child);
    }
    delete node;
}

/* ---- Reset ---- */

void BPlusTree12::resetStatistics()
{
    clearTree(m_root);
    m_root = nullptr;
    m_leftmostLeaf = nullptr;
    m_size = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
