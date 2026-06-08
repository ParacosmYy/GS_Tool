/**
 * @file SplayTree9.cpp
 * @brief SplayTree9 实现
 *
 * 实现伸展树：半伸展优化、访问频率驱动偏斜负载重构。
 */

#include "utils/tree225/SplayTree9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplayTree9::SplayTree9(QObject *parent) : QObject(parent) {}
SplayTree9::~SplayTree9() { deleteTree(m_root); }

void SplayTree9::deleteTree(SplayNode* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

/* ---- Rotations ---- */

SplayTree9::SplayNode* SplayTree9::rotateLeft(SplayNode* x)
{
    SplayNode* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    return y;
}

SplayTree9::SplayNode* SplayTree9::rotateRight(SplayNode* x)
{
    SplayNode* y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) m_root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;
    return y;
}

/* ---- Full splay ---- */

void SplayTree9::splay(SplayNode* node)
{
    if (!node) return;
    while (node->parent) {
        SplayNode* p = node->parent;
        SplayNode* gp = p->parent;

        if (!gp) {
            // Zig: single rotation
            if (node == p->left) rotateRight(p);
            else rotateLeft(p);
        } else if (node == p->left && p == gp->left) {
            // Zig-zig
            rotateRight(gp);
            rotateRight(p);
        } else if (node == p->right && p == gp->right) {
            // Zig-zig
            rotateLeft(gp);
            rotateLeft(p);
        } else if (node == p->right && p == gp->left) {
            // Zig-zag
            rotateLeft(p);
            rotateRight(gp);
        } else {
            // Zig-zag
            rotateRight(p);
            rotateLeft(gp);
        }
    }
    m_stats.totalSplays++;
}

/* ---- Semi-splay ---- */

void SplayTree9::semiSplay(SplayNode* node)
{
    if (!node) return;
    // Semi-splay: only splay halfway (single rotation if zig, skip zig-zag)
    // This reduces restructuring overhead for frequently accessed nodes
    int depth = 0;
    SplayNode* cur = node;
    while (cur->parent) { depth++; cur = cur->parent; }

    // If node is already shallow, skip splay
    if (depth <= 2) {
        if (node->parent) {
            if (node == node->parent->left) rotateRight(node->parent);
            else rotateLeft(node->parent);
        }
        m_stats.semiSplays++;
        return;
    }

    // Full splay for deep nodes, semi-splay for medium depth
    if (depth > 4) {
        splay(node);
    } else {
        // Semi-splay: bring parent to grandparent level only
        SplayNode* p = node->parent;
        if (p && p->parent) {
            SplayNode* gp = p->parent;
            if (node == p->left && p == gp->left) {
                rotateRight(gp);
            } else if (node == p->right && p == gp->right) {
                rotateLeft(gp);
            }
            // Zig-zag case: single rotation at parent level
            else if (node == p->right) {
                rotateLeft(p);
            } else {
                rotateRight(p);
            }
        } else if (p) {
            if (node == p->left) rotateRight(p);
            else rotateLeft(p);
        }
        m_stats.semiSplays++;
    }
}

/* ---- Find node ---- */

SplayTree9::SplayNode* SplayTree9::findNode(double key) const
{
    SplayNode* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur;
    }
    return nullptr;
}

/* ---- Substitute ---- */

void SplayTree9::substitute(SplayNode* u, SplayNode* v)
{
    if (!u->parent) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

/* ---- Subtree max ---- */

SplayTree9::SplayNode* SplayTree9::subtreeMax(SplayNode* node) const
{
    while (node && node->right) node = node->right;
    return node;
}

/* ---- Insert ---- */

void SplayTree9::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    SplayNode* node = new SplayNode();
    node->key = key;
    node->value = value;

    if (!m_root) {
        m_root = node;
    } else {
        SplayNode* cur = m_root;
        SplayNode* parent = nullptr;
        while (cur) {
            parent = cur;
            if (key < cur->key) cur = cur->left;
            else if (key > cur->key) cur = cur->right;
            else {
                // Key exists: update value and splay
                cur->value = value;
                cur->accessCount++;
                semiSplay(cur);
                delete node;
                m_stats.treeHeight = computeHeight(m_root);
                m_stats.totalOps++;
                m_timeSum += timer.elapsed();
                m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
                emit operationCompleted("insert", m_stats.numNodes, timer.elapsed());
                return;
            }
        }
        node->parent = parent;
        if (key < parent->key) parent->left = node;
        else parent->right = node;
    }

    m_stats.numNodes++;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("insert", m_stats.numNodes, timer.elapsed());
}

/* ---- Remove ---- */

bool SplayTree9::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    SplayNode* node = findNode(key);
    if (!node) return false;

    // Splay the node to root first
    splay(node);

    if (!node->left) {
        m_root = node->right;
        if (m_root) m_root->parent = nullptr;
    } else if (!node->right) {
        m_root = node->left;
        if (m_root) m_root->parent = nullptr;
    } else {
        // Find max of left subtree
        SplayNode* maxLeft = subtreeMax(node->left);
        splay(maxLeft);
        // Now maxLeft is root, node->right is its right child
        maxLeft->right = node->right;
        if (node->right) node->right->parent = maxLeft;
        m_root = maxLeft;
        maxLeft->parent = nullptr;
    }

    delete node;
    m_stats.numNodes--;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("remove", m_stats.numNodes, timer.elapsed());
    return true;
}

/* ---- Contains (with semi-splay) ---- */

bool SplayTree9::contains(double key)
{
    SplayNode* node = findNode(key);
    if (node) {
        node->accessCount++;
        m_stats.totalAccesses++;
        semiSplay(node);
        return true;
    }
    return false;
}

/* ---- Value ---- */

double SplayTree9::value(double key, double defaultValue) const
{
    SplayNode* node = findNode(key);
    return node ? node->value : defaultValue;
}

/* ---- Collect keys ---- */

void SplayTree9::collectKeys(SplayNode* node, QVector<double>& result) const
{
    if (!node) return;
    collectKeys(node->left, result);
    result.append(node->key);
    collectKeys(node->right, result);
}

QVector<double> SplayTree9::keys() const
{
    QVector<double> result;
    collectKeys(m_root, result);
    return result;
}

/* ---- Size ---- */

int SplayTree9::size() const { return m_stats.numNodes; }

/* ---- Compute height ---- */

int SplayTree9::computeHeight(SplayNode* node) const
{
    if (!node) return 0;
    return 1 + qMax(computeHeight(node->left), computeHeight(node->right));
}

/* ---- Build balanced from sorted ---- */

SplayTree9::SplayNode* SplayTree9::buildBalanced(
    const QVector<QPair<double, double>>& sorted, int lo, int hi)
{
    if (lo > hi) return nullptr;
    int mid = (lo + hi) / 2;
    SplayNode* node = new SplayNode();
    node->key = sorted[mid].first;
    node->value = sorted[mid].second;
    node->left = buildBalanced(sorted, lo, mid - 1);
    node->right = buildBalanced(sorted, mid + 1, hi);
    if (node->left) node->left->parent = node;
    if (node->right) node->right->parent = node;
    return node;
}

/* ---- Frequency restructure ---- */

void SplayTree9::frequencyRestructure()
{
    QElapsedTimer timer;
    timer.start();

    if (m_stats.numNodes < 2) return;

    // Collect all nodes with access counts
    QVector<QPair<double, double>> sorted;
    QVector<SplayNode*> nodes;
    // In-order traversal
    QVector<SplayNode*> stack;
    SplayNode* cur = m_root;
    while (cur || !stack.isEmpty()) {
        while (cur) { stack.append(cur); cur = cur->left; }
        cur = stack.takeLast();
        sorted.append(qMakePair(cur->key, cur->value));
        nodes.append(cur);
        cur = cur->right;
    }

    // Compute skew ratio: Gini coefficient of access counts
    quint64 totalAccess = 0;
    for (auto* n : nodes) totalAccess += n->accessCount;
    m_stats.skewRatio = (totalAccess > 0) ? (double)m_stats.totalAccesses / totalAccess : 0.0;

    // Rebuild balanced tree preserving key order
    // Delete old tree structure (not values) and rebuild
    for (auto* n : nodes) { n->left = nullptr; n->right = nullptr; n->parent = nullptr; }

    // Sort by key (already sorted from in-order)
    m_root = buildBalanced(sorted, 0, sorted.size() - 1);
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("restructure", m_stats.numNodes, timer.elapsed());
}

/* ---- Reset ---- */

void SplayTree9::resetStatistics()
{
    deleteTree(m_root);
    m_root = nullptr;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
