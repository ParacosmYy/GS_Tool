/**
 * @file Treap8.cpp
 * @brief Treap8 实现
 *
 * 实现隐式Treap：split/merge序列操作、区间插入/删除/反转。
 */

#include "utils/tree199/Treap8.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Treap8::Treap8(QObject *parent) : QObject(parent) {}
Treap8::~Treap8() { delete m_root; }

/* ---- Helpers ---- */

int Treap8::randPriority()
{
    return QRandomGenerator::global()->generate();
}

int Treap8::getSize(Node* n) { return n ? n->size : 0; }
double Treap8::getSum(Node* n) { return n ? n->sum : 0.0; }

void Treap8::update(Node* n)
{
    if (!n) return;
    n->size = 1 + getSize(n->left) + getSize(n->right);
    n->sum = n->value + getSum(n->left) + getSum(n->right);
}

void Treap8::pushDown(Node* n)
{
    if (!n || !n->rev) return;
    n->rev = false;
    std::swap(n->left, n->right);
    if (n->left) n->left->rev ^= true;
    if (n->right) n->right->rev ^= true;
}

/* ---- Split ---- */

QPair<Treap8::Node*, Treap8::Node*> Treap8::split(Node* n, int k)
{
    if (!n) return {nullptr, nullptr};
    pushDown(n);

    int leftSize = getSize(n->left);
    if (leftSize >= k) {
        auto [l, r] = split(n->left, k);
        n->left = r;
        update(n);
        return {l, n};
    } else {
        auto [l, r] = split(n->right, k - leftSize - 1);
        n->right = l;
        update(n);
        return {n, r};
    }
}

/* ---- Merge ---- */

Treap8::Node* Treap8::merge(Node* left, Node* right)
{
    if (!left) return right;
    if (!right) return left;

    pushDown(left);
    pushDown(right);

    if (left->priority > right->priority) {
        left->right = merge(left->right, right);
        update(left);
        return left;
    } else {
        right->left = merge(left, right->left);
        update(right);
        return right;
    }
}

/* ---- Build from vector ---- */

Treap8::Node* Treap8::buildFromVec(const QVector<double>& values, int lo, int hi)
{
    if (lo > hi) return nullptr;
    int mid = (lo + hi) / 2;
    Node* n = new Node;
    n->value = values[mid];
    n->priority = randPriority();
    n->rev = false;
    n->left = buildFromVec(values, lo, mid - 1);
    n->right = buildFromVec(values, mid + 1, hi);
    update(n);
    return n;
}

void Treap8::build(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    delete m_root;
    if (values.isEmpty()) { m_root = nullptr; return; }
    m_root = buildFromVec(values, 0, values.size() - 1);

    m_stats.totalOperations++;
    m_stats.nodeCount = getSize(m_root);
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("build", getSize(m_root), computeHeight(m_root), timer.elapsed());
}

/* ---- Insert ---- */

void Treap8::insert(int pos, double value)
{
    QElapsedTimer timer;
    timer.start();

    Node* n = new Node;
    n->value = value;
    n->priority = randPriority();

    auto [left, right] = split(m_root, pos);
    m_root = merge(merge(left, n), right);

    m_stats.totalOperations++;
    m_stats.nodeCount = getSize(m_root);
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("insert", getSize(m_root), computeHeight(m_root), timer.elapsed());
}

void Treap8::insertRange(int pos, const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    auto [left, right] = split(m_root, pos);
    Node* mid = nullptr;
    for (double v : values) {
        Node* n = new Node;
        n->value = v;
        n->priority = randPriority();
        mid = merge(mid, n);
    }
    m_root = merge(merge(left, mid), right);

    m_stats.totalOperations++;
    m_stats.nodeCount = getSize(m_root);
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("insertRange", getSize(m_root), computeHeight(m_root), timer.elapsed());
}

/* ---- Remove ---- */

bool Treap8::removeAt(int pos)
{
    QElapsedTimer timer;
    timer.start();

    if (pos < 0 || pos >= getSize(m_root)) return false;

    auto [left, midRight] = split(m_root, pos);
    auto [mid, right] = split(midRight, 1);
    delete mid;
    m_root = merge(left, right);

    m_stats.totalOperations++;
    m_stats.nodeCount = getSize(m_root);
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("removeAt", getSize(m_root), computeHeight(m_root), timer.elapsed());
    return true;
}

int Treap8::removeRange(int l, int r)
{
    QElapsedTimer timer;
    timer.start();

    int sz = getSize(m_root);
    if (l < 0) l = 0;
    if (r > sz) r = sz;
    if (l >= r) return 0;
    int count = r - l;

    auto [left, midRight] = split(m_root, l);
    auto [mid, right] = split(midRight, r - l);
    delete mid;
    m_root = merge(left, right);

    m_stats.totalOperations++;
    m_stats.nodeCount = getSize(m_root);
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("removeRange", getSize(m_root), computeHeight(m_root), timer.elapsed());
    return count;
}

/* ---- Reverse ---- */

void Treap8::reverseRange(int l, int r)
{
    QElapsedTimer timer;
    timer.start();

    int sz = getSize(m_root);
    if (l < 0) l = 0;
    if (r > sz) r = sz;
    if (l >= r) return;

    auto [left, midRight] = split(m_root, l);
    auto [mid, right] = split(midRight, r - l);
    if (mid) mid->rev ^= true;
    m_root = merge(merge(left, mid), right);

    m_stats.totalOperations++;
    m_stats.nodeCount = getSize(m_root);
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("reverseRange", getSize(m_root), computeHeight(m_root), timer.elapsed());
}

/* ---- At / Set ---- */

double Treap8::at(int pos) const
{
    if (pos < 0 || pos >= getSize(m_root)) return 0.0;

    Node* n = m_root;
    while (n) {
        const_cast<Treap8*>(this)->pushDown(n);
        int leftSize = getSize(n->left);
        if (pos < leftSize) n = n->left;
        else if (pos == leftSize) return n->value;
        else { pos -= leftSize + 1; n = n->right; }
    }
    return 0.0;
}

void Treap8::setAt(int pos, double value)
{
    if (pos < 0 || pos >= getSize(m_root)) return;

    auto [left, midRight] = split(m_root, pos);
    auto [mid, right] = split(midRight, 1);
    if (mid) { mid->value = value; update(mid); }
    m_root = merge(merge(left, mid), right);
}

/* ---- Range sum ---- */

double Treap8::rangeSum(int l, int r)
{
    int sz = getSize(m_root);
    if (l < 0) l = 0;
    if (r > sz) r = sz;
    if (l >= r) return 0.0;

    auto [left, midRight] = split(m_root, l);
    auto [mid, right] = split(midRight, r - l);
    double s = getSum(mid);
    m_root = merge(merge(left, mid), right);
    return s;
}

/* ---- Traversal ---- */

void Treap8::inOrder(Node* n, QVector<double>& result) const
{
    if (!n) return;
    const_cast<Treap8*>(this)->pushDown(n);
    inOrder(n->left, result);
    result.append(n->value);
    inOrder(n->right, result);
}

QVector<double> Treap8::toVector() const
{
    QVector<double> result;
    result.reserve(getSize(m_root));
    inOrder(m_root, result);
    return result;
}

/* ---- Size ---- */

int Treap8::size() const { return getSize(m_root); }
bool Treap8::isEmpty() const { return m_root == nullptr; }

/* ---- Height ---- */

int Treap8::computeHeight(const Node* n)
{
    if (!n) return 0;
    return 1 + qMax(computeHeight(n->left), computeHeight(n->right));
}

/* ---- Reset ---- */

void Treap8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    delete m_root;
    m_root = nullptr;
}
