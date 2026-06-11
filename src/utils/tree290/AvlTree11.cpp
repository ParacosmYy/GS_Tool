/**
 * @file AvlTree11.cpp
 * @brief AvlTree11 实现
 *
 * 实现AVL树：迭代再平衡与父指针增强支持锁友好的并发读路径优化。
 */

#include "utils/tree290/AvlTree11.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AvlTree11::AvlTree11(QObject *parent)
    : QObject(parent) {}

AvlTree11::~AvlTree11() = default;

/* ---- Node allocation ---- */

int AvlTree11::allocNode(int key, int value, int parent)
{
    int idx;
    if (!m_freeList.isEmpty()) {
        idx = m_freeList.takeLast();
        m_nodes[idx] = AVLNode{};
    } else {
        idx = m_nodes.size();
        m_nodes.append(AVLNode{});
    }
    m_nodes[idx].key = key;
    m_nodes[idx].value = value;
    m_nodes[idx].height = 1;
    m_nodes[idx].balanceFactor = 0;
    m_nodes[idx].left = NULL_NODE;
    m_nodes[idx].right = NULL_NODE;
    m_nodes[idx].parent = parent;
    return idx;
}

void AvlTree11::freeNode(int idx) { m_freeList.append(idx); }

/* ---- Update height and balance factor ---- */

void AvlTree11::updateHeight(int idx)
{
    if (idx == NULL_NODE) return;
    int lh = (m_nodes[idx].left != NULL_NODE) ? m_nodes[m_nodes[idx].left].height : 0;
    int rh = (m_nodes[idx].right != NULL_NODE) ? m_nodes[m_nodes[idx].right].height : 0;
    m_nodes[idx].height = 1 + qMax(lh, rh);
    m_nodes[idx].balanceFactor = lh - rh;
}

/* ---- Update parent's child link ---- */

void AvlTree11::updateChildLink(int parent, int oldChild, int newChild)
{
    if (parent == NULL_NODE) {
        m_root = newChild;
    } else {
        if (m_nodes[parent].left == oldChild)
            m_nodes[parent].left = newChild;
        else
            m_nodes[parent].right = newChild;
    }
    if (newChild != NULL_NODE)
        m_nodes[newChild].parent = parent;
}

/* ---- Right rotation ----
 *
 *     y              x
 *    / \            / \
 *   x   C   =>    A   y
 *  / \                / \
 * A   B              B   C
 */
int AvlTree11::rotateRight(int y)
{
    int x = m_nodes[y].left;
    int B = m_nodes[x].right;
    int parent = m_nodes[y].parent;

    m_nodes[x].right = y;
    m_nodes[y].left = B;
    m_nodes[y].parent = x;
    m_nodes[x].parent = parent;

    if (B != NULL_NODE) m_nodes[B].parent = y;

    updateChildLink(parent, y, x);
    updateHeight(y);
    updateHeight(x);
    return x;
}

/* ---- Left rotation ---- */

int AvlTree11::rotateLeft(int x)
{
    int y = m_nodes[x].right;
    int B = m_nodes[y].left;
    int parent = m_nodes[x].parent;

    m_nodes[y].left = x;
    m_nodes[x].right = B;
    m_nodes[x].parent = y;
    m_nodes[y].parent = parent;

    if (B != NULL_NODE) m_nodes[B].parent = x;

    updateChildLink(parent, x, y);
    updateHeight(x);
    updateHeight(y);
    return y;
}

/* ---- Iterative rebalance from startIdx up to root ---- */

void AvlTree11::rebalanceUpward(int startIdx)
{
    int idx = startIdx;
    while (idx != NULL_NODE) {
        updateHeight(idx);
        int bf = m_nodes[idx].balanceFactor;
        int parent = m_nodes[idx].parent;

        if (bf > 1) {
            // Left-heavy
            int leftChild = m_nodes[idx].left;
            if (m_nodes[leftChild].balanceFactor < 0) {
                // LR case: rotate left on left child first
                rotateLeft(leftChild);
            }
            rotateRight(idx);
            // After rotation, idx is no longer the root of this subtree
        } else if (bf < -1) {
            // Right-heavy
            int rightChild = m_nodes[idx].right;
            if (m_nodes[rightChild].balanceFactor > 0) {
                // RL case: rotate right on right child first
                rotateRight(rightChild);
            }
            rotateLeft(idx);
        }

        idx = parent;
    }
}

/* ---- Iterative insert ---- */

void AvlTree11::insertIter(int key, int value)
{
    if (m_root == NULL_NODE) {
        m_root = allocNode(key, value, NULL_NODE);
        m_size++;
        return;
    }

    int cur = m_root;
    int parent = NULL_NODE;

    // Find insertion point
    while (cur != NULL_NODE) {
        parent = cur;
        if (key < m_nodes[cur].key) {
            cur = m_nodes[cur].left;
        } else if (key > m_nodes[cur].key) {
            cur = m_nodes[cur].right;
        } else {
            // Key exists: update value
            m_nodes[cur].value = value;
            return;
        }
    }

    // Insert new node
    int newNode = allocNode(key, value, parent);
    if (key < m_nodes[parent].key)
        m_nodes[parent].left = newNode;
    else
        m_nodes[parent].right = newNode;

    m_size++;

    // Iterative rebalance upward
    rebalanceUpward(parent);
}

/* ---- Find minimum in subtree ---- */

int AvlTree11::findMin(int idx) const
{
    while (idx != NULL_NODE && m_nodes[idx].left != NULL_NODE)
        idx = m_nodes[idx].left;
    return idx;
}

/* ---- Iterative remove ---- */

void AvlTree11::removeIter(int key)
{
    // Find node to remove
    int cur = m_root;
    while (cur != NULL_NODE && m_nodes[cur].key != key) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else cur = m_nodes[cur].right;
    }
    if (cur == NULL_NODE) return; // Key not found

    int rebalanceStart = m_nodes[cur].parent;

    if (m_nodes[cur].left == NULL_NODE && m_nodes[cur].right == NULL_NODE) {
        // Leaf node
        updateChildLink(m_nodes[cur].parent, cur, NULL_NODE);
        freeNode(cur);
    } else if (m_nodes[cur].left == NULL_NODE) {
        // Only right child
        int child = m_nodes[cur].right;
        updateChildLink(m_nodes[cur].parent, cur, child);
        freeNode(cur);
        rebalanceStart = m_nodes[child].parent;
    } else if (m_nodes[cur].right == NULL_NODE) {
        // Only left child
        int child = m_nodes[cur].left;
        updateChildLink(m_nodes[cur].parent, cur, child);
        freeNode(cur);
        rebalanceStart = m_nodes[child].parent;
    } else {
        // Two children: replace with in-order successor
        int succ = findMin(m_nodes[cur].right);
        rebalanceStart = m_nodes[succ].parent;
        if (rebalanceStart == cur) rebalanceStart = succ;

        // Copy successor data
        m_nodes[cur].key = m_nodes[succ].key;
        m_nodes[cur].value = m_nodes[succ].value;

        // Remove successor (it has at most one right child)
        int succChild = m_nodes[succ].right;
        updateChildLink(m_nodes[succ].parent, succ, succChild);
        freeNode(succ);
    }

    m_size--;

    // Iterative rebalance upward from the point of deletion
    rebalanceUpward(rebalanceStart);
}

/* ---- Public insert ---- */

void AvlTree11::insert(int key, int value)
{
    QElapsedTimer timer;
    timer.start();

    insertIter(key, value);

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("insert"), key, elapsed);
}

/* ---- Public remove ---- */

void AvlTree11::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    removeIter(key);

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("remove"), key, elapsed);
}

/* ---- Search (lock-friendly: read-only traversal using parent pointers) ---- */

int AvlTree11::search(int key) const
{
    int cur = m_root;
    while (cur != NULL_NODE) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key) cur = m_nodes[cur].right;
        else return m_nodes[cur].value;
    }
    return -1;
}

/* ---- Contains ---- */

bool AvlTree11::contains(int key) const
{
    int cur = m_root;
    while (cur != NULL_NODE) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key) cur = m_nodes[cur].right;
        else return true;
    }
    return false;
}

/* ---- In-order traversal ---- */

void AvlTree11::inOrder(int idx, QVector<QPair<int,int>>& result) const
{
    if (idx == NULL_NODE) return;
    inOrder(m_nodes[idx].left, result);
    result.append({m_nodes[idx].key, m_nodes[idx].value});
    inOrder(m_nodes[idx].right, result);
}

QVector<QPair<int,int>> AvlTree11::toVector() const
{
    QVector<QPair<int,int>> result;
    result.reserve(m_size);
    inOrder(m_root, result);
    return result;
}

QVector<int> AvlTree11::keys() const
{
    QVector<QPair<int,int>> pairs = toVector();
    QVector<int> k;
    k.reserve(pairs.size());
    for (const auto& p : pairs) k.append(p.first);
    return k;
}

/* ---- Compute height ---- */

int AvlTree11::computeHeight(int idx) const
{
    if (idx == NULL_NODE) return 0;
    int lh = computeHeight(m_nodes[idx].left);
    int rh = computeHeight(m_nodes[idx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Reset ---- */

void AvlTree11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_freeList.clear();
    m_root = NULL_NODE;
    m_size = 0;
}
