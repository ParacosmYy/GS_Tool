/**
 * @file RedBlackTree17.cpp
 * @brief RedBlackTree17 实现
 *
 * 实现红黑树：迭代插入修复与节点池化实现缓存友好内存分配的平衡BST操作。
 */

#include "utils/tree294/RedBlackTree17.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RedBlackTree17::RedBlackTree17(QObject *parent)
    : QObject(parent)
{
    expandPool();
}

RedBlackTree17::~RedBlackTree17() = default;

/* ---- Configuration ---- */

void RedBlackTree17::setPoolCapacity(int capacity)
{
    m_poolCap = qBound(16, capacity, 1000000);
    expandPool();
}

/* ---- Pool management ---- */

void RedBlackTree17::expandPool()
{
    int oldSize = m_pool.size();
    int target = qMax(m_poolCap, m_size + 64);
    if (oldSize >= target) return;

    m_pool.resize(target);
    // Add new slots to free list
    for (int i = oldSize; i < target; ++i) {
        m_pool[i].color = Black;
        m_pool[i].left = NIL;
        m_pool[i].right = NIL;
        m_pool[i].parent = NIL;
        m_freeList.append(i);
    }
    m_stats.poolCapacity = m_pool.size();
}

int RedBlackTree17::allocNode(int key, double value)
{
    if (m_freeList.isEmpty()) {
        expandPool();
        if (m_freeList.isEmpty()) return NIL;
    }
    int idx = m_freeList.takeLast();
    m_pool[idx].key = key;
    m_pool[idx].value = value;
    m_pool[idx].color = Red;
    m_pool[idx].left = NIL;
    m_pool[idx].right = NIL;
    m_pool[idx].parent = NIL;
    return idx;
}

void RedBlackTree17::freeNode(int idx)
{
    if (idx >= 0 && idx < m_pool.size()) {
        m_pool[idx].left = NIL;
        m_pool[idx].right = NIL;
        m_pool[idx].parent = NIL;
        m_freeList.append(idx);
    }
}

/* ---- Rotations ---- */

void RedBlackTree17::rotateLeft(int x)
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

void RedBlackTree17::rotateRight(int x)
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

/* ---- Iterative insertion fixup ---- */

void RedBlackTree17::insertFixup(int z)
{
    // Iterative (non-recursive) fixup for cache-friendly access pattern
    while (m_pool[z].parent != NIL && m_pool[m_pool[z].parent].color == Red) {
        int parent = m_pool[z].parent;
        int grandparent = m_pool[parent].parent;
        if (grandparent == NIL) break;

        if (parent == m_pool[grandparent].left) {
            int uncle = m_pool[grandparent].right;
            if (uncle != NIL && m_pool[uncle].color == Red) {
                // Case 1: Red uncle
                m_pool[parent].color = Black;
                m_pool[uncle].color = Black;
                m_pool[grandparent].color = Red;
                z = grandparent;
            } else {
                if (z == m_pool[parent].right) {
                    // Case 2: Triangle
                    z = parent;
                    rotateLeft(z);
                    parent = m_pool[z].parent;
                    grandparent = m_pool[parent].parent;
                }
                // Case 3: Line
                m_pool[parent].color = Black;
                m_pool[grandparent].color = Red;
                rotateRight(grandparent);
            }
        } else {
            int uncle = m_pool[grandparent].left;
            if (uncle != NIL && m_pool[uncle].color == Red) {
                m_pool[parent].color = Black;
                m_pool[uncle].color = Black;
                m_pool[grandparent].color = Red;
                z = grandparent;
            } else {
                if (z == m_pool[parent].left) {
                    z = parent;
                    rotateRight(z);
                    parent = m_pool[z].parent;
                    grandparent = m_pool[parent].parent;
                }
                m_pool[parent].color = Black;
                m_pool[grandparent].color = Red;
                rotateLeft(grandparent);
            }
        }
    }
    m_pool[m_root].color = Black;
}

/* ---- Transplant ---- */

void RedBlackTree17::transplant(int u, int v)
{
    if (m_pool[u].parent == NIL)
        m_root = v;
    else if (u == m_pool[m_pool[u].parent].left)
        m_pool[m_pool[u].parent].left = v;
    else
        m_pool[m_pool[u].parent].right = v;
    if (v != NIL)
        m_pool[v].parent = m_pool[u].parent;
}

/* ---- Minimum ---- */

int RedBlackTree17::minimum(int x) const
{
    while (x != NIL && m_pool[x].left != NIL)
        x = m_pool[x].left;
    return x;
}

/* ---- Delete fixup ---- */

void RedBlackTree17::deleteFixup(int x)
{
    while (x != m_root && (x == NIL || m_pool[x].color == Black)) {
        if (x == m_pool[m_pool[x].parent != NIL ? m_pool[x].parent : m_root].left) {
            int w = m_pool[m_pool[x].parent].right;
            if (w != NIL && m_pool[w].color == Red) {
                m_pool[w].color = Black;
                m_pool[m_pool[x].parent].color = Red;
                rotateLeft(m_pool[x].parent);
                w = m_pool[m_pool[x].parent].right;
            }
            if (w != NIL &&
                (m_pool[w].left == NIL || m_pool[m_pool[w].left].color == Black) &&
                (m_pool[w].right == NIL || m_pool[m_pool[w].right].color == Black)) {
                m_pool[w].color = Red;
                x = m_pool[x].parent;
            } else {
                if (w != NIL && (m_pool[w].right == NIL || m_pool[m_pool[w].right].color == Black)) {
                    if (m_pool[w].left != NIL) m_pool[m_pool[w].left].color = Black;
                    m_pool[w].color = Red;
                    rotateRight(w);
                    w = m_pool[m_pool[x].parent].right;
                }
                if (w != NIL) {
                    m_pool[w].color = m_pool[m_pool[x].parent].color;
                    m_pool[m_pool[x].parent].color = Black;
                    if (m_pool[w].right != NIL) m_pool[m_pool[w].right].color = Black;
                }
                rotateLeft(m_pool[x].parent);
                x = m_root;
            }
        } else {
            int w = m_pool[m_pool[x].parent].left;
            if (w != NIL && m_pool[w].color == Red) {
                m_pool[w].color = Black;
                m_pool[m_pool[x].parent].color = Red;
                rotateRight(m_pool[x].parent);
                w = m_pool[m_pool[x].parent].left;
            }
            if (w != NIL &&
                (m_pool[w].right == NIL || m_pool[m_pool[w].right].color == Black) &&
                (m_pool[w].left == NIL || m_pool[m_pool[w].left].color == Black)) {
                m_pool[w].color = Red;
                x = m_pool[x].parent;
            } else {
                if (w != NIL && (m_pool[w].left == NIL || m_pool[m_pool[w].left].color == Black)) {
                    if (m_pool[w].right != NIL) m_pool[m_pool[w].right].color = Black;
                    m_pool[w].color = Red;
                    rotateLeft(w);
                    w = m_pool[m_pool[x].parent].left;
                }
                if (w != NIL) {
                    m_pool[w].color = m_pool[m_pool[x].parent].color;
                    m_pool[m_pool[x].parent].color = Black;
                    if (m_pool[w].left != NIL) m_pool[m_pool[w].left].color = Black;
                }
                rotateRight(m_pool[x].parent);
                x = m_root;
            }
        }
    }
    if (x != NIL) m_pool[x].color = Black;
}

/* ---- Insert ---- */

void RedBlackTree17::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    // Iterative BST insert
    int z = allocNode(key, value);
    if (z == NIL) return;

    int y = NIL;
    int x = m_root;
    while (x != NIL) {
        y = x;
        if (key < m_pool[x].key)
            x = m_pool[x].left;
        else if (key > m_pool[x].key)
            x = m_pool[x].right;
        else {
            // Duplicate: update value
            m_pool[x].value = value;
            freeNode(z);
            double elapsed = timer.elapsed();
            m_stats.totalOps++;
            m_timeSum += elapsed;
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
            emit operationDone(QStringLiteral("insert"), key, elapsed);
            return;
        }
    }

    m_pool[z].parent = y;
    if (y == NIL)
        m_root = z;
    else if (key < m_pool[y].key)
        m_pool[y].left = z;
    else
        m_pool[y].right = z;

    m_size++;
    insertFixup(z);

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("insert"), key, elapsed);
}

/* ---- Remove ---- */

void RedBlackTree17::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Find node
    int z = m_root;
    while (z != NIL && m_pool[z].key != key) {
        if (key < m_pool[z].key) z = m_pool[z].left;
        else z = m_pool[z].right;
    }
    if (z == NIL) return;

    Color origColor = m_pool[z].color;
    int y = z;
    int x;

    if (m_pool[z].left == NIL) {
        x = m_pool[z].right;
        transplant(z, m_pool[z].right);
    } else if (m_pool[z].right == NIL) {
        x = m_pool[z].left;
        transplant(z, m_pool[z].left);
    } else {
        y = minimum(m_pool[z].right);
        origColor = m_pool[y].color;
        x = m_pool[y].right;
        if (m_pool[y].parent == z) {
            if (x != NIL) m_pool[x].parent = y;
        } else {
            transplant(y, m_pool[y].right);
            m_pool[y].right = m_pool[z].right;
            m_pool[m_pool[y].right].parent = y;
        }
        transplant(z, y);
        m_pool[y].left = m_pool[z].left;
        m_pool[m_pool[y].left].parent = y;
        m_pool[y].color = m_pool[z].color;
    }

    freeNode(z);
    m_size--;

    if (origColor == Black && x != NIL)
        deleteFixup(x);

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("remove"), key, elapsed);
}

/* ---- Search ---- */

bool RedBlackTree17::contains(int key) const
{
    int x = m_root;
    while (x != NIL) {
        if (key < m_pool[x].key) x = m_pool[x].left;
        else if (key > m_pool[x].key) x = m_pool[x].right;
        else return true;
    }
    return false;
}

double RedBlackTree17::value(int key) const
{
    int x = m_root;
    while (x != NIL) {
        if (key < m_pool[x].key) x = m_pool[x].left;
        else if (key > m_pool[x].key) x = m_pool[x].right;
        else return m_pool[x].value;
    }
    return 0.0;
}

/* ---- Traversal ---- */

void RedBlackTree17::inOrder(int node, QVector<int>& result) const
{
    if (node == NIL) return;
    inOrder(m_pool[node].left, result);
    result.append(m_pool[node].key);
    inOrder(m_pool[node].right, result);
}

QVector<int> RedBlackTree17::keys() const
{
    QVector<int> result;
    result.reserve(m_size);
    inOrder(m_root, result);
    return result;
}

/* ---- Height ---- */

int RedBlackTree17::computeHeight(int node) const
{
    if (node == NIL) return 0;
    return 1 + qMax(computeHeight(m_pool[node].left),
                    computeHeight(m_pool[node].right));
}

int RedBlackTree17::height() const { return computeHeight(m_root); }

/* ---- Reset ---- */

void RedBlackTree17::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_pool.clear();
    m_freeList.clear();
    m_root = NIL;
    m_size = 0;
    expandPool();
}
