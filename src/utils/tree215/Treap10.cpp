/**
 * @file Treap10.cpp
 * @brief Treap10 实现
 *
 * 实现隐式键Treap：分裂-合并、区间更新、懒传播。
 */

#include "utils/tree215/Treap10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

Treap10::Treap10(QObject *parent) : QObject(parent) {}
Treap10::~Treap10() = default;

/* ---- Allocate node ---- */

int Treap10::allocateNode(double value)
{
    Node n;
    n.value = value;
    n.sum = value;
    n.lazy = 0.0;
    n.priority = std::rand();
    n.left = -1;
    n.right = -1;
    n.size = 1;
    n.hasLazy = false;
    m_nodes.append(n);
    return m_nodes.size() - 1;
}

/* ---- Update aggregates ---- */

void Treap10::update(int node)
{
    if (node < 0 || node >= m_nodes.size()) return;
    m_nodes[node].sum = m_nodes[node].value;
    m_nodes[node].size = 1;

    if (m_nodes[node].left >= 0) {
        m_nodes[node].sum += m_nodes[m_nodes[node].left].sum;
        m_nodes[node].size += m_nodes[m_nodes[node].left].size;
    }
    if (m_nodes[node].right >= 0) {
        m_nodes[node].sum += m_nodes[m_nodes[node].right].sum;
        m_nodes[node].size += m_nodes[m_nodes[node].right].size;
    }
}

/* ---- Push lazy propagation ---- */

void Treap10::pushDown(int node)
{
    if (node < 0 || node >= m_nodes.size()) return;
    if (!m_nodes[node].hasLazy) return;

    double delta = m_nodes[node].lazy;

    if (m_nodes[node].left >= 0) {
        int l = m_nodes[node].left;
        m_nodes[l].value += delta;
        m_nodes[l].sum += delta * m_nodes[l].size;
        m_nodes[l].lazy += delta;
        m_nodes[l].hasLazy = true;
    }
    if (m_nodes[node].right >= 0) {
        int r = m_nodes[node].right;
        m_nodes[r].value += delta;
        m_nodes[r].sum += delta * m_nodes[r].size;
        m_nodes[r].lazy += delta;
        m_nodes[r].hasLazy = true;
    }

    m_nodes[node].lazy = 0.0;
    m_nodes[node].hasLazy = false;
}

/* ---- Split at position ---- */

QPair<int, int> Treap10::split(int root, int pos) const
{
    if (root < 0 || root >= m_nodes.size()) return {-1, -1};

    const_cast<Treap10*>(this)->pushDown(root);

    int leftSize = (m_nodes[root].left >= 0)
        ? m_nodes[m_nodes[root].left].size : 0;

    if (leftSize >= pos) {
        // Entire split is in left subtree
        auto [ll, lr] = const_cast<Treap10*>(this)->split(m_nodes[root].left, pos);
        m_nodes[root].left = lr;
        const_cast<Treap10*>(this)->update(root);
        return {ll, root};
    } else {
        auto [rl, rr] = const_cast<Treap10*>(this)->split(
            m_nodes[root].right, pos - leftSize - 1);
        m_nodes[root].right = rl;
        const_cast<Treap10*>(this)->update(root);
        return {root, rr};
    }
}

/* ---- Merge two treaps ---- */

int Treap10::merge(int left, int right)
{
    if (left < 0) return right;
    if (right < 0) return left;
    if (left >= m_nodes.size() || right >= m_nodes.size()) return -1;

    pushDown(left);
    pushDown(right);

    if (m_nodes[left].priority > m_nodes[right].priority) {
        m_nodes[left].right = merge(m_nodes[left].right, right);
        update(left);
        return left;
    } else {
        m_nodes[right].left = merge(left, m_nodes[right].left);
        update(right);
        return right;
    }
}

/* ---- Build from values ---- */

void Treap10::build(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes.clear();
    m_root = -1;

    for (int i = 0; i < values.size(); ++i) {
        int node = allocateNode(values[i]);
        m_root = merge(m_root, node);
    }

    m_stats.treeSize = values.size();
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Insert at position ---- */

void Treap10::insert(int pos, double value)
{
    QElapsedTimer timer;
    timer.start();

    int node = allocateNode(value);
    auto [left, right] = split(m_root, pos);
    m_root = merge(merge(left, node), right);

    m_stats.treeSize++;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Remove at position ---- */

void Treap10::remove(int pos)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0 || pos < 0 || pos >= size()) return;

    auto [left, midRight] = split(m_root, pos);
    auto [mid, right] = split(midRight, 1);
    // Discard 'mid' (single node)
    m_root = merge(left, right);

    m_stats.treeSize--;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Get value at position ---- */

double Treap10::get(int pos) const
{
    if (m_root < 0) return 0.0;
    int node = m_root;
    while (node >= 0 && node < m_nodes.size()) {
        const_cast<Treap10*>(this)->pushDown(node);
        int leftSize = (m_nodes[node].left >= 0)
            ? m_nodes[m_nodes[node].left].size : 0;
        if (pos < leftSize) {
            node = m_nodes[node].left;
        } else if (pos == leftSize) {
            return m_nodes[node].value;
        } else {
            pos -= leftSize + 1;
            node = m_nodes[node].right;
        }
    }
    return 0.0;
}

/* ---- Set value at position ---- */

void Treap10::set(int pos, double value)
{
    if (m_root < 0 || pos < 0) return;
    auto [left, midRight] = split(m_root, pos);
    auto [mid, right] = split(midRight, 1);
    if (mid >= 0 && mid < m_nodes.size())
        m_nodes[mid].value = value;
    update(mid);
    m_root = merge(merge(left, mid), right);
}

/* ---- Range update [l, r] ---- */

void Treap10::rangeUpdate(int l, int r, double delta)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0 || l > r) return;
    auto [left, midRight] = split(m_root, l);
    auto [mid, right] = split(midRight, r - l + 1);

    if (mid >= 0 && mid < m_nodes.size()) {
        m_nodes[mid].value += delta;
        m_nodes[mid].sum += delta * m_nodes[mid].size;
        m_nodes[mid].lazy += delta;
        m_nodes[mid].hasLazy = true;
    }

    m_root = merge(merge(left, mid), right);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Range query [l, r] ---- */

double Treap10::rangeQuery(int l, int r) const
{
    if (m_root < 0 || l > r) return 0.0;

    auto [left, midRight] = const_cast<Treap10*>(this)->split(m_root, l);
    auto [mid, right] = const_cast<Treap10*>(this)->split(midRight, r - l + 1);

    double result = (mid >= 0 && mid < m_nodes.size()) ? m_nodes[mid].sum : 0.0;

    m_root = const_cast<Treap10*>(this)->merge(
        const_cast<Treap10*>(this)->merge(left, mid), right);

    return result;
}

/* ---- In-order traversal ---- */

void Treap10::inOrder(int node, QVector<double>& result) const
{
    if (node < 0 || node >= m_nodes.size()) return;
    const_cast<Treap10*>(this)->pushDown(node);
    inOrder(m_nodes[node].left, result);
    result.append(m_nodes[node].value);
    inOrder(m_nodes[node].right, result);
}

QVector<double> Treap10::toVector() const
{
    QVector<double> result;
    result.reserve(size());
    inOrder(m_root, result);
    return result;
}

/* ---- Tree size ---- */

int Treap10::size() const
{
    return (m_root >= 0 && m_root < m_nodes.size())
        ? m_nodes[m_root].size : 0;
}

/* ---- Compute height ---- */

int Treap10::computeHeight(int node) const
{
    if (node < 0 || node >= m_nodes.size()) return 0;
    int lh = computeHeight(m_nodes[node].left);
    int rh = computeHeight(m_nodes[node].right);
    return 1 + qMax(lh, rh);
}

/* ---- Reset ---- */

void Treap10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_root = -1;
}
