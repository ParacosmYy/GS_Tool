/**
 * @file RedBlackTree13.cpp
 * @brief RedBlackTree13 实现
 *
 * 实现红黑树：区间增强节点重叠查询与批量插入色翻优化。
 */

#include "utils/tree238/RedBlackTree13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RedBlackTree13::RedBlackTree13(QObject *parent) : QObject(parent)
{
    initNil();
    m_root = m_nil;
}

RedBlackTree13::~RedBlackTree13()
{
    destroyTree(m_root);
    delete m_nil;
}

/* ---- Sentinel initialization ---- */

void RedBlackTree13::initNil()
{
    m_nil = new Node();
    m_nil->color = Black;
    m_nil->iv = {0.0, 0.0, 0.0};
    m_nil->maxHigh = 0.0;
    m_nil->left = m_nil->right = m_nil->parent = m_nil;
}

/* ---- Update max-high ---- */

void RedBlackTree13::updateMaxHigh(Node* n)
{
    if (n == m_nil) return;
    n->maxHigh = n->iv.high;
    if (n->left != m_nil) n->maxHigh = qMax(n->maxHigh, n->left->maxHigh);
    if (n->right != m_nil) n->maxHigh = qMax(n->maxHigh, n->right->maxHigh);
}

void RedBlackTree13::fixMaxHigh(Node* n)
{
    while (n != m_nil) {
        updateMaxHigh(n);
        n = n->parent;
    }
}

/* ---- Rotations ---- */

void RedBlackTree13::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    if (y->left != m_nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == m_nil) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    updateMaxHigh(x);
    updateMaxHigh(y);
    m_stats.numRotations++;
}

void RedBlackTree13::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    if (x->right != m_nil) x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == m_nil) m_root = x;
    else if (y == y->parent->right) y->parent->right = x;
    else y->parent->left = x;
    x->right = y;
    y->parent = x;
    updateMaxHigh(y);
    updateMaxHigh(x);
    m_stats.numRotations++;
}

/* ---- Insert fixup ---- */

void RedBlackTree13::insertFixup(Node* z)
{
    while (z->parent->color == Red) {
        if (z->parent == z->parent->parent->left) {
            Node* y = z->parent->parent->right;
            if (y->color == Red) {
                // Color flip: parent and uncle to black, grandparent to red
                z->parent->color = Black;
                y->color = Black;
                z->parent->parent->color = Red;
                m_stats.numColorFlips += 3;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rotateLeft(z);
                }
                z->parent->color = Black;
                z->parent->parent->color = Red;
                m_stats.numColorFlips += 2;
                rotateRight(z->parent->parent);
            }
        } else {
            Node* y = z->parent->parent->left;
            if (y->color == Red) {
                z->parent->color = Black;
                y->color = Black;
                z->parent->parent->color = Red;
                m_stats.numColorFlips += 3;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rotateRight(z);
                }
                z->parent->color = Black;
                z->parent->parent->color = Red;
                m_stats.numColorFlips += 2;
                rotateLeft(z->parent->parent);
            }
        }
    }
    m_root->color = Black;
}

/* ---- Single insert ---- */

void RedBlackTree13::insert(const Interval& iv)
{
    QElapsedTimer timer;
    timer.start();

    Node* z = new Node();
    z->iv = iv;
    z->maxHigh = iv.high;
    z->color = Red;
    z->left = m_nil;
    z->right = m_nil;
    z->parent = m_nil;

    Node* y = m_nil;
    Node* x = m_root;
    while (x != m_nil) {
        y = x;
        if (iv.low < x->iv.low) x = x->left;
        else x = x->right;
    }

    z->parent = y;
    if (y == m_nil) m_root = z;
    else if (iv.low < y->iv.low) y->left = z;
    else y->right = z;

    fixMaxHigh(z);
    insertFixup(z);

    m_stats.numNodes++;
    m_stats.treeHeight = height(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit nodeInserted(iv.low, iv.high);
}

/* ---- Build sorted (bulk insert) ---- */

RedBlackTree13::Node* RedBlackTree13::buildSorted(const QVector<Interval>& sorted,
                                                    int lo, int hi, Color color)
{
    if (lo > hi) return m_nil;
    int mid = (lo + hi) / 2;
    Node* n = new Node();
    n->iv = sorted[mid];
    n->color = color;
    n->left = m_nil;
    n->right = m_nil;
    n->parent = m_nil;

    n->left = buildSorted(sorted, lo, mid - 1, Red);
    if (n->left != m_nil) n->left->parent = n;
    n->right = buildSorted(sorted, mid + 1, hi, Red);
    if (n->right != m_nil) n->right->parent = n;

    updateMaxHigh(n);
    m_stats.numColorFlips++;
    return n;
}

/* ---- Bulk insert ---- */

void RedBlackTree13::bulkInsert(const QVector<Interval>& intervals)
{
    if (intervals.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    // Collect existing intervals
    QVector<Interval> all = allIntervals();
    for (const auto& iv : intervals) all.append(iv);

    // Sort by low bound
    std::sort(all.begin(), all.end(),
              [](const Interval& a, const Interval& b) { return a.low < b.low; });

    // Rebuild balanced tree with color-flip optimization
    destroyTree(m_root);
    m_stats.numColorFlips = 0;

    m_root = buildSorted(all, 0, all.size() - 1, Black);
    if (m_root != m_nil) m_root->parent = m_nil;

    m_stats.numNodes = all.size();
    m_stats.treeHeight = height(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit bulkInsertCompleted(intervals.size(), m_stats.numColorFlips, timer.elapsed());
}

/* ---- Transplant ---- */

void RedBlackTree13::transplant(Node* u, Node* v)
{
    if (u->parent == m_nil) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

/* ---- Tree minimum ---- */

RedBlackTree13::Node* RedBlackTree13::treeMinimum(Node* x) const
{
    while (x->left != m_nil) x = x->left;
    return x;
}

/* ---- Remove fixup ---- */

void RedBlackTree13::removeFixup(Node* x)
{
    while (x != m_root && x->color == Black) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;
            if (w->color == Red) {
                w->color = Black;
                x->parent->color = Red;
                rotateLeft(x->parent);
                w = x->parent->right;
            }
            if (w->left->color == Black && w->right->color == Black) {
                w->color = Red;
                x = x->parent;
            } else {
                if (w->right->color == Black) {
                    w->left->color = Black;
                    w->color = Red;
                    rotateRight(w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = Black;
                w->right->color = Black;
                rotateLeft(x->parent);
                x = m_root;
            }
        } else {
            Node* w = x->parent->left;
            if (w->color == Red) {
                w->color = Black;
                x->parent->color = Red;
                rotateRight(x->parent);
                w = x->parent->left;
            }
            if (w->right->color == Black && w->left->color == Black) {
                w->color = Red;
                x = x->parent;
            } else {
                if (w->left->color == Black) {
                    w->right->color = Black;
                    w->color = Red;
                    rotateLeft(w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = Black;
                w->left->color = Black;
                rotateRight(x->parent);
                x = m_root;
            }
        }
    }
    x->color = Black;
}

/* ---- Remove ---- */

bool RedBlackTree13::remove(const Interval& iv)
{
    Node* z = m_root;
    while (z != m_nil) {
        if (iv.low < z->iv.low) z = z->left;
        else if (iv.low > z->iv.low) z = z->right;
        else if (qFuzzyCompare(iv.high, z->iv.high)) break;
        else z = z->right;
    }
    if (z == m_nil) return false;

    Node* y = z;
    Color yOrigColor = y->color;
    Node* x = m_nil;

    if (z->left == m_nil) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == m_nil) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = treeMinimum(z->right);
        yOrigColor = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    delete z;
    fixMaxHigh(x->parent);
    if (yOrigColor == Black) removeFixup(x);

    m_stats.numNodes--;
    m_stats.treeHeight = height(m_root);
    emit nodeRemoved(iv.low, iv.high);
    return true;
}

/* ---- Overlap query ---- */

void RedBlackTree13::queryOverlapRec(Node* n, double low, double high,
                                      QVector<OverlapResult>& results) const
{
    if (n == m_nil) return;
    // Prune: if maxHigh in subtree < low, no overlap possible
    if (n->maxHigh < low) return;
    // Check left subtree
    queryOverlapRec(n->left, low, high, results);
    // Check current node
    if (n->iv.low <= high && n->iv.high >= low) {
        OverlapResult r;
        r.interval = n->iv;
        r.exact = (n->iv.low == low && n->iv.high == high);
        results.append(r);
    }
    // If current node's low > high, no need to check right
    if (n->iv.low <= high)
        queryOverlapRec(n->right, low, high, results);
}

QVector<RedBlackTree13::OverlapResult> RedBlackTree13::queryOverlap(double low, double high) const
{
    QVector<OverlapResult> results;
    queryOverlapRec(m_root, low, high, results);
    return results;
}

/* ---- Contains point ---- */

bool RedBlackTree13::containsPoint(double point) const
{
    Node* n = m_root;
    while (n != m_nil) {
        if (point >= n->iv.low && point <= n->iv.high) return true;
        if (point < n->iv.low) n = n->left;
        else n = n->right;
    }
    return false;
}

/* ---- In-order traversal ---- */

void RedBlackTree13::inOrderCollect(Node* n, QVector<Interval>& out) const
{
    if (n == m_nil) return;
    inOrderCollect(n->left, out);
    out.append(n->iv);
    inOrderCollect(n->right, out);
}

QVector<RedBlackTree13::Interval> RedBlackTree13::allIntervals() const
{
    QVector<Interval> result;
    inOrderCollect(m_root, result);
    return result;
}

/* ---- Height ---- */

int RedBlackTree13::height(Node* n) const
{
    if (n == m_nil) return 0;
    return 1 + qMax(height(n->left), height(n->right));
}

/* ---- Destroy ---- */

void RedBlackTree13::destroyTree(Node* n)
{
    if (n == m_nil || !n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    if (n != m_nil) delete n;
}

/* ---- Reset ---- */

void RedBlackTree13::resetStatistics()
{
    destroyTree(m_root);
    m_root = m_nil;
    m_stats = Stats{}; m_timeSum = 0.0;
}
