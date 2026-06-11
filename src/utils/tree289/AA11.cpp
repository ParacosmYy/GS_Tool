/**
 * @file AA11.cpp
 * @brief AA11 实现
 *
 * 实现AA树：自底向上再平衡与基于层级的倾斜-分裂不变量简化平衡BST维护。
 */

#include "utils/tree289/AA11.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AA11::AA11(QObject *parent)
    : QObject(parent) {}

AA11::~AA11() = default;

/* ---- Node allocation ---- */

int AA11::allocNode(int key, int value)
{
    int idx;
    if (!m_freeList.isEmpty()) {
        idx = m_freeList.takeLast();
        m_nodes[idx] = AANode{};
    } else {
        idx = m_nodes.size();
        m_nodes.append(AANode{});
    }
    m_nodes[idx].key = key;
    m_nodes[idx].value = value;
    m_nodes[idx].level = 1;
    m_nodes[idx].left = NULL_NODE;
    m_nodes[idx].right = NULL_NODE;
    return idx;
}

void AA11::freeNode(int idx) { m_freeList.append(idx); }

/* ---- Skew: right rotation to fix left-horizontal link ----
 *
 *   T          L
 *  / \   =>   / \
 * L   c      a   T
 *            / \
 *           a   b
 */
int AA11::skew(int node)
{
    if (node == NULL_NODE) return NULL_NODE;
    int L = m_nodes[node].left;
    if (L == NULL_NODE) return node;
    if (m_nodes[L].level != m_nodes[node].level) return node;

    // Perform right rotation
    m_nodes[node].left = m_nodes[L].right;
    m_nodes[L].right = node;
    return L;
}

/* ---- Split: left rotation to fix consecutive right-horizontal links ----
 *
 *   T              R
 *  / \    =>     /   \
 * a   R         T     X
 *    / \       / \   / \
 *   b   X     a   b c   d
 *      / \
 *     c   d
 */
int AA11::split(int node)
{
    if (node == NULL_NODE) return NULL_NODE;
    int R = m_nodes[node].right;
    if (R == NULL_NODE) return node;
    int RR = m_nodes[R].right;
    if (RR == NULL_NODE) return node;
    if (m_nodes[RR].level != m_nodes[node].level) return node;

    // Perform left rotation and increase level of R
    m_nodes[node].right = m_nodes[R].left;
    m_nodes[R].left = node;
    m_nodes[R].level++;
    return R;
}

/* ---- Recursive insert ---- */

int AA11::insertRec(int node, int key, int value, bool& inserted)
{
    if (node == NULL_NODE) {
        inserted = true;
        return allocNode(key, value);
    }

    if (key < m_nodes[node].key) {
        m_nodes[node].left = insertRec(m_nodes[node].left, key, value, inserted);
    } else if (key > m_nodes[node].key) {
        m_nodes[node].right = insertRec(m_nodes[node].right, key, value, inserted);
    } else {
        // Key exists: update value
        m_nodes[node].value = value;
        inserted = false;
        return node;
    }

    // Bottom-up rebalance: skew then split
    node = skew(node);
    node = split(node);
    return node;
}

/* ---- Find minimum ---- */

int AA11::findMin(int node) const
{
    while (node != NULL_NODE && m_nodes[node].left != NULL_NODE)
        node = m_nodes[node].left;
    return node;
}

/* ---- Recursive remove ---- */

int AA11::removeRec(int node, int key, bool& removed)
{
    if (node == NULL_NODE) { removed = false; return NULL_NODE; }

    if (key < m_nodes[node].key) {
        m_nodes[node].left = removeRec(m_nodes[node].left, key, removed);
    } else if (key > m_nodes[node].key) {
        m_nodes[node].right = removeRec(m_nodes[node].right, key, removed);
    } else {
        removed = true;
        if (m_nodes[node].left == NULL_NODE && m_nodes[node].right == NULL_NODE) {
            freeNode(node);
            return NULL_NODE;
        }
        if (m_nodes[node].left == NULL_NODE) {
            int succ = findMin(m_nodes[node].right);
            m_nodes[node].key = m_nodes[succ].key;
            m_nodes[node].value = m_nodes[succ].value;
            m_nodes[node].right = removeRec(m_nodes[node].right, m_nodes[succ].key, removed);
            removed = true;
        } else {
            int pred = findMin(m_nodes[node].left);
            // Find rightmost in left subtree (in-order predecessor)
            int cur = m_nodes[node].left;
            while (m_nodes[cur].right != NULL_NODE) cur = m_nodes[cur].right;
            m_nodes[node].key = m_nodes[cur].key;
            m_nodes[node].value = m_nodes[cur].value;
            m_nodes[node].left = removeRec(m_nodes[node].left, m_nodes[cur].key, removed);
            removed = true;
        }
    }

    // Decrease level if needed
    int leftLevel = (m_nodes[node].left != NULL_NODE) ? m_nodes[m_nodes[node].left].level : 0;
    int rightLevel = (m_nodes[node].right != NULL_NODE) ? m_nodes[m_nodes[node].right].level : 0;
    int shouldBe = qMin(leftLevel, rightLevel) + 1;
    if (shouldBe < m_nodes[node].level) {
        m_nodes[node].level = shouldBe;
        if (m_nodes[node].right != NULL_NODE &&
            m_nodes[m_nodes[node].right].level > shouldBe)
            m_nodes[m_nodes[node].right].level = shouldBe;
    }

    // Bottom-up rebalance: skew and split three times to fix all invariants
    node = skew(node);
    m_nodes[node].right = skew(m_nodes[node].right);
    if (m_nodes[node].right != NULL_NODE)
        m_nodes[m_nodes[node].right].right = skew(m_nodes[m_nodes[node].right].right);
    node = split(node);
    m_nodes[node].right = split(m_nodes[node].right);

    return node;
}

/* ---- Insert ---- */

void AA11::insert(int key, int value)
{
    QElapsedTimer timer;
    timer.start();

    bool inserted = false;
    m_root = insertRec(m_root, key, value, inserted);
    if (inserted) m_size++;

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("insert"), key, elapsed);
}

/* ---- Remove ---- */

void AA11::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    bool removed = false;
    m_root = removeRec(m_root, key, removed);
    if (removed) m_size--;

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("remove"), key, elapsed);
}

/* ---- Search ---- */

int AA11::search(int key) const
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

bool AA11::contains(int key) const
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

void AA11::inOrder(int idx, QVector<QPair<int,int>>& result) const
{
    if (idx == NULL_NODE) return;
    inOrder(m_nodes[idx].left, result);
    result.append({m_nodes[idx].key, m_nodes[idx].value});
    inOrder(m_nodes[idx].right, result);
}

QVector<QPair<int,int>> AA11::toVector() const
{
    QVector<QPair<int,int>> result;
    result.reserve(m_size);
    inOrder(m_root, result);
    return result;
}

QVector<int> AA11::keys() const
{
    QVector<QPair<int,int>> pairs = toVector();
    QVector<int> k;
    k.reserve(pairs.size());
    for (const auto& p : pairs) k.append(p.first);
    return k;
}

/* ---- Compute height ---- */

int AA11::computeHeight(int idx) const
{
    if (idx == NULL_NODE) return 0;
    int lh = computeHeight(m_nodes[idx].left);
    int rh = computeHeight(m_nodes[idx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Reset ---- */

void AA11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_freeList.clear();
    m_root = NULL_NODE;
    m_size = 0;
}
