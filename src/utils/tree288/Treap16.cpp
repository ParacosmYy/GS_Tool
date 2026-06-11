/**
 * @file Treap16.cpp
 * @brief Treap16 实现
 *
 * 实现隐式键Treap：惰性传播与反转操作的高效范围查询随机优先级BST。
 */

#include "utils/tree288/Treap16.h"

#include <QElapsedTimer>
#include <algorithm>
#include <random>

/* ---- Construction / Destruction ---- */

Treap16::Treap16(QObject *parent)
    : QObject(parent) {}

Treap16::~Treap16() = default;

/* ---- Node allocation ---- */

int Treap16::allocNode(int value)
{
    std::mt19937 rng(static_cast<unsigned>(value + m_nodes.size() * 997 + 12345));
    std::uniform_int_distribution<int> dist(1, 2000000000);

    int idx;
    if (!m_freeList.isEmpty()) {
        idx = m_freeList.takeLast();
        m_nodes[idx] = TreapNode{};
    } else {
        idx = m_nodes.size();
        m_nodes.append(TreapNode{});
    }
    m_nodes[idx].value = value;
    m_nodes[idx].priority = dist(rng);
    m_nodes[idx].size = 1;
    m_nodes[idx].sum = value;
    m_nodes[idx].minVal = value;
    m_nodes[idx].maxVal = value;
    m_nodes[idx].left = NULL_NODE;
    m_nodes[idx].right = NULL_NODE;
    m_nodes[idx].parent = NULL_NODE;
    m_nodes[idx].revLazy = false;
    return idx;
}

void Treap16::freeNode(int idx) { m_freeList.append(idx); }

/* ---- Push lazy reversal ---- */

void Treap16::pushDown(int idx)
{
    if (idx == NULL_NODE || !m_nodes[idx].revLazy) return;
    m_nodes[idx].revLazy = false;

    int l = m_nodes[idx].left;
    int r = m_nodes[idx].right;

    // Swap children
    m_nodes[idx].left = r;
    m_nodes[idx].right = l;

    if (l != NULL_NODE) m_nodes[l].revLazy = !m_nodes[l].revLazy;
    if (r != NULL_NODE) m_nodes[r].revLazy = !m_nodes[r].revLazy;
}

/* ---- Pull aggregate info ---- */

void Treap16::pullUp(int idx)
{
    if (idx == NULL_NODE) return;
    TreapNode& node = m_nodes[idx];
    int l = node.left, r = node.right;

    node.size = 1;
    node.sum = node.value;
    node.minVal = node.value;
    node.maxVal = node.value;

    if (l != NULL_NODE) {
        pushDown(l);
        node.size += m_nodes[l].size;
        node.sum += m_nodes[l].sum;
        node.minVal = qMin(node.minVal, m_nodes[l].minVal);
        node.maxVal = qMax(node.maxVal, m_nodes[l].maxVal);
    }
    if (r != NULL_NODE) {
        pushDown(r);
        node.size += m_nodes[r].size;
        node.sum += m_nodes[r].sum;
        node.minVal = qMin(node.minVal, m_nodes[r].minVal);
        node.maxVal = qMax(node.maxVal, m_nodes[r].maxVal);
    }
}

/* ---- Split by position ---- */

QPair<int, int> Treap16::split(int root, int k)
{
    if (root == NULL_NODE) return {NULL_NODE, NULL_NODE};
    pushDown(root);

    int leftSize = (m_nodes[root].left != NULL_NODE) ? m_nodes[m_nodes[root].left].size : 0;

    if (k <= leftSize) {
        auto [l, r] = split(m_nodes[root].left, k);
        m_nodes[root].left = r;
        if (r != NULL_NODE) m_nodes[r].parent = root;
        if (l != NULL_NODE) m_nodes[l].parent = NULL_NODE;
        pullUp(root);
        return {l, root};
    } else {
        auto [l, r] = split(m_nodes[root].right, k - leftSize - 1);
        m_nodes[root].right = l;
        if (l != NULL_NODE) m_nodes[l].parent = root;
        if (r != NULL_NODE) m_nodes[r].parent = NULL_NODE;
        pullUp(root);
        return {root, r};
    }
}

/* ---- Merge ---- */

int Treap16::merge(int left, int right)
{
    if (left == NULL_NODE) return right;
    if (right == NULL_NODE) return left;

    pushDown(left);
    pushDown(right);

    if (m_nodes[left].priority > m_nodes[right].priority) {
        m_nodes[left].right = merge(m_nodes[left].right, right);
        if (m_nodes[left].right != NULL_NODE) m_nodes[m_nodes[left].right].parent = left;
        pullUp(left);
        return left;
    } else {
        m_nodes[right].left = merge(left, m_nodes[right].left);
        if (m_nodes[right].left != NULL_NODE) m_nodes[m_nodes[right].left].parent = right;
        pullUp(right);
        return right;
    }
}

/* ---- Build from sequence ---- */

void Treap16::build(const QVector<int>& values)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes.clear();
    m_freeList.clear();
    m_root = NULL_NODE;
    m_size = values.size();

    for (int v : values) {
        int node = allocNode(v);
        m_root = merge(m_root, node);
    }

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Insert ---- */

void Treap16::insert(int pos, int value)
{
    QElapsedTimer timer;
    timer.start();

    int node = allocNode(value);
    auto [left, right] = split(m_root, pos);
    m_root = merge(merge(left, node), right);
    m_size++;

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("insert"), pos, elapsed);
}

/* ---- Remove ---- */

void Treap16::remove(int pos)
{
    QElapsedTimer timer;
    timer.start();

    auto [left, midRight] = split(m_root, pos);
    auto [mid, right] = split(midRight, 1);
    if (mid != NULL_NODE) freeNode(mid);
    m_root = merge(left, right);
    m_size--;

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("remove"), pos, elapsed);
}

/* ---- At ---- */

int Treap16::at(int pos) const
{
    if (pos < 0 || pos >= m_size) return 0;
    int cur = m_root;
    while (cur != NULL_NODE) {
        const_cast<Treap16*>(this)->pushDown(cur);
        int leftSize = (m_nodes[cur].left != NULL_NODE) ? m_nodes[m_nodes[cur].left].size : 0;
        if (pos < leftSize) {
            cur = m_nodes[cur].left;
        } else if (pos == leftSize) {
            return m_nodes[cur].value;
        } else {
            pos -= leftSize + 1;
            cur = m_nodes[cur].right;
        }
    }
    return 0;
}

/* ---- Reverse ---- */

void Treap16::reverse(int l, int r)
{
    QElapsedTimer timer;
    timer.start();

    if (l < 0) l = 0;
    if (r >= m_size) r = m_size - 1;
    if (l >= r) return;

    auto [left, midRight] = split(m_root, l);
    auto [mid, right] = split(midRight, r - l + 1);
    if (mid != NULL_NODE) m_nodes[mid].revLazy = !m_nodes[mid].revLazy;
    m_root = merge(merge(left, mid), right);

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("reverse"), l, elapsed);
}

/* ---- Range sum ---- */

int Treap16::rangeSum(int l, int r)
{
    if (l < 0) l = 0;
    if (r >= m_size) r = m_size - 1;
    if (l > r) return 0;

    auto [left, midRight] = split(m_root, l);
    auto [mid, right] = split(midRight, r - l + 1);
    int sum = (mid != NULL_NODE) ? m_nodes[mid].sum : 0;
    m_root = merge(merge(left, mid), right);
    return sum;
}

/* ---- Range min ---- */

int Treap16::rangeMin(int l, int r)
{
    if (l < 0) l = 0;
    if (r >= m_size) r = m_size - 1;
    if (l > r) return 0;

    auto [left, midRight] = split(m_root, l);
    auto [mid, right] = split(midRight, r - l + 1);
    int mn = (mid != NULL_NODE) ? m_nodes[mid].minVal : 0;
    m_root = merge(merge(left, mid), right);
    return mn;
}

/* ---- Range max ---- */

int Treap16::rangeMax(int l, int r)
{
    if (l < 0) l = 0;
    if (r >= m_size) r = m_size - 1;
    if (l > r) return 0;

    auto [left, midRight] = split(m_root, l);
    auto [mid, right] = split(midRight, r - l + 1);
    int mx = (mid != NULL_NODE) ? m_nodes[mid].maxVal : 0;
    m_root = merge(merge(left, mid), right);
    return mx;
}

/* ---- To vector ---- */

void Treap16::inOrder(int idx, QVector<int>& result) const
{
    if (idx == NULL_NODE) return;
    const_cast<Treap16*>(this)->pushDown(idx);
    inOrder(m_nodes[idx].left, result);
    result.append(m_nodes[idx].value);
    inOrder(m_nodes[idx].right, result);
}

QVector<int> Treap16::toVector() const
{
    QVector<int> result;
    result.reserve(m_size);
    inOrder(m_root, result);
    return result;
}

/* ---- Compute height ---- */

int Treap16::computeHeight(int idx) const
{
    if (idx == NULL_NODE) return 0;
    int lh = computeHeight(m_nodes[idx].left);
    int rh = computeHeight(m_nodes[idx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Reset ---- */

void Treap16::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_freeList.clear();
    m_root = NULL_NODE;
    m_size = 0;
}
