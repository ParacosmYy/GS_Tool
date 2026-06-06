/**
 * @file AvlTree3.cpp
 * @brief AvlTree3 实现
 *
 * 实现AVL平衡二叉搜索树：旋转调整、select/rank序统计。
 */

#include "utils/tree175/AvlTree3.h"

#include <QElapsedTimer>
#include <QString>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AvlTree3::AvlTree3(QObject *parent)
    : QObject(parent)
{
}

AvlTree3::~AvlTree3() { clearRec(m_root); }

/* ---- Node metadata ---- */

int AvlTree3::getHeight(Node* t) { return t ? t->height : 0; }
int AvlTree3::getSize(Node* t) { return t ? t->subtreeSize : 0; }

int AvlTree3::balanceFactor(Node* t)
{
    return t ? getHeight(t->left) - getHeight(t->right) : 0;
}

void AvlTree3::updateMeta(Node* t)
{
    if (!t) return;
    t->height = 1 + qMax(getHeight(t->left), getHeight(t->right));
    t->subtreeSize = 1 + getSize(t->left) + getSize(t->right);
}

/* ---- Rotations ---- */

AvlTree3::Node* AvlTree3::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    updateMeta(y);
    updateMeta(x);
    emit rotationPerformed(QStringLiteral("LL"));
    return x;
}

AvlTree3::Node* AvlTree3::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    updateMeta(x);
    updateMeta(y);
    emit rotationPerformed(QStringLiteral("RR"));
    return y;
}

/* ---- Balance ---- */

AvlTree3::Node* AvlTree3::balance(Node* t)
{
    if (!t) return nullptr;
    updateMeta(t);
    int bf = balanceFactor(t);

    if (bf > 1) {
        if (balanceFactor(t->left) < 0) {
            /* LR case */
            t->left = rotateLeft(t->left);
            emit rotationPerformed(QStringLiteral("LR"));
            return rotateRight(t);
        }
        /* LL case */
        return rotateRight(t);
    }

    if (bf < -1) {
        if (balanceFactor(t->right) > 0) {
            /* RL case */
            t->right = rotateRight(t->right);
            emit rotationPerformed(QStringLiteral("RL"));
            return rotateLeft(t);
        }
        /* RR case */
        return rotateLeft(t);
    }

    return t;
}

/* ---- Insert ---- */

AvlTree3::Node* AvlTree3::insertRec(Node* t, double key, double value, bool& inserted)
{
    if (!t) {
        inserted = true;
        return new Node(key, value);
    }

    if (key < t->key) {
        t->left = insertRec(t->left, key, value, inserted);
    } else if (key > t->key) {
        t->right = insertRec(t->right, key, value, inserted);
    } else {
        /* Key exists, update value */
        t->value = value;
        inserted = false;
        return t;
    }

    return balance(t);
}

bool AvlTree3::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    bool inserted = false;
    m_root = insertRec(m_root, key, value, inserted);

    if (inserted) m_stats.currentSize++;
    m_stats.totalInserts++;
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries);

    emit insertCompleted(key, m_stats.currentSize);
    return inserted;
}

/* ---- Subtree minimum ---- */

AvlTree3::Node* AvlTree3::subtreeMin(Node* t)
{
    if (!t) return nullptr;
    while (t->left) t = t->left;
    return t;
}

/* ---- Remove ---- */

AvlTree3::Node* AvlTree3::removeRec(Node* t, double key, bool& removed)
{
    if (!t) { removed = false; return nullptr; }

    if (key < t->key) {
        t->left = removeRec(t->left, key, removed);
    } else if (key > t->key) {
        t->right = removeRec(t->right, key, removed);
    } else {
        removed = true;
        if (!t->left || !t->right) {
            Node* child = t->left ? t->left : t->right;
            delete t;
            return child;
        }
        /* Two children: replace with inorder successor */
        Node* succ = subtreeMin(t->right);
        t->key = succ->key;
        t->value = succ->value;
        bool dummy = false;
        t->right = removeRec(t->right, succ->key, dummy);
    }

    return balance(t);
}

bool AvlTree3::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    bool removed = false;
    m_root = removeRec(m_root, key, removed);

    if (removed) m_stats.currentSize--;
    m_stats.totalDeletes++;
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries);

    emit deleteCompleted(key);
    return removed;
}

/* ---- Find ---- */

bool AvlTree3::find(double key, double& value) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else { value = cur->value; return true; }
    }
    return false;
}

/* ---- Select k-th smallest (1-based) ---- */

AvlTree3::Node* AvlTree3::selectRec(Node* t, int k)
{
    if (!t) return nullptr;
    int leftSize = getSize(t->left);
    if (k <= leftSize) return selectRec(t->left, k);
    if (k == leftSize + 1) return t;
    return selectRec(t->right, k - leftSize - 1);
}

bool AvlTree3::select(int k, double& key, double& value) const
{
    if (k < 1 || k > m_stats.currentSize) return false;
    Node* n = selectRec(m_root, k);
    if (!n) return false;
    key = n->key;
    value = n->value;
    return true;
}

/* ---- Rank of a key (1-based) ---- */

int AvlTree3::rankRec(Node* t, double key)
{
    if (!t) return 0;
    if (key < t->key) return rankRec(t->left, key);
    if (key == t->key) return getSize(t->left) + 1;
    return getSize(t->left) + 1 + rankRec(t->right, key);
}

int AvlTree3::rank(double key) const
{
    return rankRec(m_root, key);
}

/* ---- In-order traversal ---- */

void AvlTree3::inOrderRec(Node* t, QVector<QPair<double, double>>& result)
{
    if (!t) return;
    inOrderRec(t->left, result);
    result.append({t->key, t->value});
    inOrderRec(t->right, result);
}

QVector<QPair<double, double>> AvlTree3::inOrder() const
{
    QVector<QPair<double, double>> result;
    inOrderRec(m_root, result);
    return result;
}

/* ---- Range query ---- */

void AvlTree3::rangeRec(Node* t, double lo, double hi,
                          QVector<QPair<double, double>>& result)
{
    if (!t) return;
    if (lo < t->key) rangeRec(t->left, lo, hi, result);
    if (lo <= t->key && t->key <= hi)
        result.append({t->key, t->value});
    if (hi > t->key) rangeRec(t->right, lo, hi, result);
}

QVector<QPair<double, double>> AvlTree3::rangeQuery(double keyLo, double keyHi) const
{
    QVector<QPair<double, double>> result;
    rangeRec(m_root, keyLo, keyHi, result);
    return result;
}

/* ---- Utility ---- */

void AvlTree3::clearRec(Node* t)
{
    if (!t) return;
    clearRec(t->left);
    clearRec(t->right);
    delete t;
}

int AvlTree3::computeHeight(Node* t)
{
    if (!t) return 0;
    return 1 + qMax(computeHeight(t->left), computeHeight(t->right));
}

bool AvlTree3::isEmpty() const { return m_root == nullptr; }

void AvlTree3::clear()
{
    clearRec(m_root);
    m_root = nullptr;
    m_stats.currentSize = 0;
    m_stats.treeHeight = 0;
}

int AvlTree3::size() const { return m_stats.currentSize; }

/* ---- Statistics ---- */

void AvlTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
