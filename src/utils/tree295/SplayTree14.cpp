/**
 * @file SplayTree14.cpp
 * @brief SplayTree14 实现
 *
 * 实现伸展树：自顶向下zig-zig与半伸展摊还重平衡实现改进访问局部性的自调整BST。
 */

#include "utils/tree295/SplayTree14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplayTree14::SplayTree14(QObject *parent)
    : QObject(parent)
{
    expandPool();
}

SplayTree14::~SplayTree14() = default;

/* ---- Pool management ---- */

void SplayTree14::expandPool()
{
    int oldSize = m_pool.size();
    int target = qMax(256, m_size + 64);
    if (oldSize >= target) return;

    m_pool.resize(target);
    for (int i = oldSize; i < target; ++i) {
        m_pool[i].left = NIL;
        m_pool[i].right = NIL;
        m_pool[i].parent = NIL;
        m_freeList.append(i);
    }
}

int SplayTree14::allocNode(int key, double value)
{
    if (m_freeList.isEmpty()) {
        expandPool();
        if (m_freeList.isEmpty()) return NIL;
    }
    int idx = m_freeList.takeLast();
    m_pool[idx].key = key;
    m_pool[idx].value = value;
    m_pool[idx].left = NIL;
    m_pool[idx].right = NIL;
    m_pool[idx].parent = NIL;
    return idx;
}

void SplayTree14::freeNode(int idx)
{
    if (idx >= 0 && idx < m_pool.size()) {
        m_pool[idx].left = NIL;
        m_pool[idx].right = NIL;
        m_pool[idx].parent = NIL;
        m_freeList.append(idx);
    }
}

/* ---- Rotations ---- */

void SplayTree14::rotateLeft(int x)
{
    int y = m_pool[x].right;
    if (y == NIL) return;

    m_pool[x].right = m_pool[y].left;
    if (m_pool[y].left != NIL)
        m_pool[m_pool[y].left].parent = x;

    m_pool[y].parent = m_pool[x].parent;
    if (m_pool[x].parent == NIL)
        m_root = y;
    else if (x == m_pool[m_pool[x].parent].left)
        m_pool[m_pool[x].parent].left = y;
    else
        m_pool[m_pool[x].parent].right = y;

    m_pool[y].left = x;
    m_pool[x].parent = y;
}

void SplayTree14::rotateRight(int x)
{
    int y = m_pool[x].left;
    if (y == NIL) return;

    m_pool[x].left = m_pool[y].right;
    if (m_pool[y].right != NIL)
        m_pool[m_pool[y].right].parent = x;

    m_pool[y].parent = m_pool[x].parent;
    if (m_pool[x].parent == NIL)
        m_root = y;
    else if (x == m_pool[m_pool[x].parent].right)
        m_pool[m_pool[x].parent].right = y;
    else
        m_pool[m_pool[x].parent].left = y;

    m_pool[y].right = x;
    m_pool[x].parent = y;
}

/* ---- Top-down splay ---- */

void SplayTree14::splay(int key)
{
    if (m_root == NIL) return;

    // Create sentinel nodes for left and right assembled trees
    // Use dummy indices in the pool for the header nodes
    int headerIdx = allocNode(0, 0);
    if (headerIdx == NIL) return;

    int leftTree = headerIdx;
    int rightTree = headerIdx;
    int t = m_root;

    while (true) {
        if (key < m_pool[t].key) {
            if (m_pool[t].left == NIL) break;
            // Zig-zig case: if key is in left-left, rotate right first
            if (key < m_pool[m_pool[t].left].key) {
                rotateRight(t);
                t = m_pool[t].parent;
                if (m_pool[t].left == NIL) break;
            }
            // Link right tree
            m_pool[rightTree].left = t;
            m_pool[t].parent = rightTree; // Temporary parent for assembly
            rightTree = t;
            t = m_pool[t].left;
        } else if (key > m_pool[t].key) {
            if (m_pool[t].right == NIL) break;
            // Zag-zig case: if key is in right-right, rotate left first
            if (key > m_pool[m_pool[t].right].key) {
                rotateLeft(t);
                t = m_pool[t].parent;
                if (m_pool[t].right == NIL) break;
            }
            // Link left tree
            m_pool[leftTree].right = t;
            m_pool[t].parent = leftTree;
            leftTree = t;
            t = m_pool[t].right;
        } else {
            break; // Found
        }
    }

    // Reassemble
    m_pool[leftTree].right = m_pool[t].left;
    if (m_pool[t].left != NIL) m_pool[m_pool[t].left].parent = leftTree;
    m_pool[rightTree].left = m_pool[t].right;
    if (m_pool[t].right != NIL) m_pool[m_pool[t].right].parent = rightTree;

    m_pool[t].left = m_pool[headerIdx].right;
    if (m_pool[headerIdx].right != NIL)
        m_pool[m_pool[headerIdx].right].parent = t;
    m_pool[t].right = m_pool[headerIdx].left;
    if (m_pool[headerIdx].left != NIL)
        m_pool[m_pool[headerIdx].left].parent = t;
    m_pool[t].parent = NIL;
    m_root = t;

    // Free header
    freeNode(headerIdx);
    m_stats.numSplays++;
}

/* ---- Semi-splay: partial rebalancing ---- */

void SplayTree14::semiSplay(int node)
{
    // Semi-splay: only splay every other level for amortized rebalancing
    // This reduces the work on frequent sequential access patterns
    if (node == NIL || m_pool[node].parent == NIL) return;

    int count = 0;
    int current = node;
    while (current != NIL && m_pool[current].parent != NIL) {
        current = m_pool[current].parent;
        count++;
    }

    // Only perform full splay if depth exceeds threshold
    if (count > 3) {
        splay(m_pool[node].key);
    }
}

/* ---- Find maximum ---- */

int SplayTree14::findMax(int node) const
{
    while (node != NIL && m_pool[node].right != NIL)
        node = m_pool[node].right;
    return node;
}

/* ---- Insert ---- */

void SplayTree14::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root == NIL) {
        int n = allocNode(key, value);
        m_root = n;
        m_size++;
    } else {
        // Top-down insert with splay
        splay(key);

        if (m_pool[m_root].key == key) {
            // Update existing
            m_pool[m_root].value = value;
            double elapsed = timer.elapsed();
            m_stats.totalOps++;
            m_timeSum += elapsed;
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
            emit operationDone(QStringLiteral("insert"), key, height(), elapsed);
            return;
        }

        int n = allocNode(key, value);
        if (n == NIL) return;

        if (key < m_pool[m_root].key) {
            // New node becomes root, old root goes right
            m_pool[n].left = m_pool[m_root].left;
            if (m_pool[m_root].left != NIL)
                m_pool[m_pool[m_root].left].parent = n;
            m_pool[n].right = m_root;
            m_pool[m_root].left = NIL;
            m_pool[m_root].parent = n;
        } else {
            m_pool[n].right = m_pool[m_root].right;
            if (m_pool[m_root].right != NIL)
                m_pool[m_pool[m_root].right].parent = n;
            m_pool[n].left = m_root;
            m_pool[m_root].right = NIL;
            m_pool[m_root].parent = n;
        }
        m_pool[n].parent = NIL;
        m_root = n;
        m_size++;
    }

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("insert"), key, height(), elapsed);
}

/* ---- Remove ---- */

void SplayTree14::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root == NIL) return;

    splay(key);
    if (m_pool[m_root].key != key) return; // Not found

    int toRemove = m_root;

    if (m_pool[m_root].left == NIL) {
        m_root = m_pool[m_root].right;
        if (m_root != NIL) m_pool[m_root].parent = NIL;
    } else if (m_pool[m_root].right == NIL) {
        m_root = m_pool[m_root].left;
        if (m_root != NIL) m_pool[m_root].parent = NIL;
    } else {
        // Find max in left subtree
        int maxLeft = findMax(m_pool[m_root].left);
        // Splay it to bring it to the left child position
        splay(m_pool[maxLeft].key);
        // Now root should be the max of left subtree
        // Attach right subtree
        m_pool[m_root].right = m_pool[toRemove].right;
        if (m_pool[toRemove].right != NIL)
            m_pool[m_pool[toRemove].right].parent = m_root;
    }

    freeNode(toRemove);
    m_size--;

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("remove"), key, height(), elapsed);
}

/* ---- Search ---- */

bool SplayTree14::contains(int key)
{
    if (m_root == NIL) return false;
    splay(key);
    return m_pool[m_root].key == key;
}

double SplayTree14::value(int key)
{
    if (m_root == NIL) return 0.0;
    splay(key);
    if (m_pool[m_root].key == key) return m_pool[m_root].value;
    return 0.0;
}

/* ---- Traversal ---- */

void SplayTree14::inOrder(int node, QVector<int>& result) const
{
    if (node == NIL) return;
    inOrder(m_pool[node].left, result);
    result.append(m_pool[node].key);
    inOrder(m_pool[node].right, result);
}

QVector<int> SplayTree14::keys() const
{
    QVector<int> result;
    result.reserve(m_size);
    inOrder(m_root, result);
    return result;
}

/* ---- Height ---- */

int SplayTree14::computeHeight(int node) const
{
    if (node == NIL) return 0;
    return 1 + qMax(computeHeight(m_pool[node].left),
                    computeHeight(m_pool[node].right));
}

int SplayTree14::height() const { return computeHeight(m_root); }

/* ---- Reset ---- */

void SplayTree14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_pool.clear();
    m_freeList.clear();
    m_root = NIL;
    m_size = 0;
    expandPool();
}
