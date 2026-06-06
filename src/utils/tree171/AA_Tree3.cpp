/**
 * @file AA_Tree3.cpp
 * @brief AA_Tree3 实现
 *
 * 实现AA树：skew/split重平衡、插入/删除、排名/选择、范围查询。
 */

#include "utils/tree171/AA_Tree3.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AA_Tree3::AA_Tree3(QObject *parent) : QObject(parent) {}
AA_Tree3::~AA_Tree3() { clearRec(m_root); }

/* ---- Helpers ---- */

int AA_Tree3::getSize(Node* t) { return t ? t->subtreeSize : 0; }

void AA_Tree3::updateSize(Node* t)
{
    if (t) t->subtreeSize = 1 + getSize(t->left) + getSize(t->right);
}

int AA_Tree3::computeHeight(Node* t)
{
    if (!t) return 0;
    return t->level;
}

/* ---- Skew (right rotation to fix left horizontal link) ---- */

AA_Tree3::Node* AA_Tree3::skew(Node* t)
{
    if (!t || !t->left) return t;
    if (t->left->level != t->level) return t;

    Node* l = t->left;
    t->left = l->right;
    l->right = t;

    updateSize(t);
    updateSize(l);

    m_skewCount++;
    m_stats.totalRebalances++;
    return l;
}

/* ---- Split (left rotation to fix consecutive right horizontal links) ---- */

AA_Tree3::Node* AA_Tree3::split(Node* t)
{
    if (!t || !t->right || !t->right->right) return t;
    if (t->right->right->level != t->level) return t;

    Node* r = t->right;
    t->right = r->left;
    r->left = t;
    r->level++;

    updateSize(t);
    updateSize(r);

    m_splitCount++;
    m_stats.totalRebalances++;
    return r;
}

/* ---- Insert ---- */

AA_Tree3::Node* AA_Tree3::insertRec(Node* t, double key, double value, bool& inserted)
{
    if (!t) {
        inserted = true;
        return new Node(key, value);
    }

    if (key < t->key)
        t->left = insertRec(t->left, key, value, inserted);
    else if (key > t->key)
        t->right = insertRec(t->right, key, value, inserted);
    else {
        inserted = false;
        t->value = value;  /* Update existing */
        return t;
    }

    updateSize(t);
    t = skew(t);
    t = split(t);
    return t;
}

bool AA_Tree3::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    m_skewCount = 0;
    m_splitCount = 0;

    bool inserted = false;
    m_root = insertRec(m_root, key, value, inserted);

    if (inserted) {
        m_stats.currentSize++;
        m_stats.totalInserts++;
        m_stats.treeHeight = computeHeight(m_root);
    }

    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeletes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit insertCompleted(key, m_stats.currentSize);
    if (m_skewCount > 0 || m_splitCount > 0)
        emit rebalancePerformed(m_skewCount, m_splitCount);

    return inserted;
}

/* ---- Successor / Predecessor ---- */

AA_Tree3::Node* AA_Tree3::successor(Node* t) const
{
    if (!t) return nullptr;
    t = t->right;
    while (t && t->left) t = t->left;
    return t;
}

AA_Tree3::Node* AA_Tree3::predecessor(Node* t) const
{
    if (!t) return nullptr;
    t = t->left;
    while (t && t->right) t = t->right;
    return t;
}

/* ---- Delete ---- */

AA_Tree3::Node* AA_Tree3::removeRec(Node* t, double key, bool& removed)
{
    if (!t) { removed = false; return nullptr; }

    if (key < t->key) {
        t->left = removeRec(t->left, key, removed);
    } else if (key > t->key) {
        t->right = removeRec(t->right, key, removed);
    } else {
        removed = true;
        if (!t->left && !t->right) {
            delete t;
            return nullptr;
        }
        if (!t->left) {
            Node* s = successor(t);
            t->key = s->key; t->value = s->value;
            t->right = removeRec(t->right, s->key, removed);
        } else {
            Node* p = predecessor(t);
            t->key = p->key; t->value = p->value;
            t->left = removeRec(t->left, p->key, removed);
        }
    }

    updateSize(t);

    /* Fix levels: decrease level if needed */
    int leftLevel = t->left ? t->left->level : 0;
    int rightLevel = t->right ? t->right->level : 0;
    int shouldBe = qMin(leftLevel, rightLevel) + 1;
    if (shouldBe < t->level) {
        t->level = shouldBe;
        if (t->right && shouldBe < t->right->level)
            t->right->level = shouldBe;
    }

    /* Rebalance */
    t = skew(t);
    if (t->right) {
        t->right = skew(t->right);
        if (t->right->right)
            t->right->right = skew(t->right->right);
    }
    t = split(t);
    if (t->right)
        t->right = split(t->right);

    return t;
}

bool AA_Tree3::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    m_skewCount = 0;
    m_splitCount = 0;

    bool removed = false;
    m_root = removeRec(m_root, key, removed);

    if (removed) {
        m_stats.currentSize--;
        m_stats.totalDeletes++;
        m_stats.treeHeight = computeHeight(m_root);
    }

    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeletes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit deleteCompleted(key);
    return removed;
}

/* ---- Find ---- */

bool AA_Tree3::find(double key, double& value) const
{
    Node* t = m_root;
    while (t) {
        if (key < t->key) t = t->left;
        else if (key > t->key) t = t->right;
        else { value = t->value; return true; }
    }
    return false;
}

/* ---- Rank (1-based) ---- */

int AA_Tree3::rank(double key) const
{
    int r = 1;
    Node* t = m_root;
    while (t) {
        if (key < t->key) {
            t = t->left;
        } else if (key > t->key) {
            r += getSize(t->left) + 1;
            t = t->right;
        } else {
            return r + getSize(t->left);
        }
    }
    return -1;
}

/* ---- Select k-th smallest (1-based) ---- */

bool AA_Tree3::select(int k, double& key, double& value) const
{
    if (k < 1 || k > m_stats.currentSize) return false;
    Node* t = m_root;
    while (t) {
        int leftSize = getSize(t->left);
        if (k <= leftSize) {
            t = t->left;
        } else if (k == leftSize + 1) {
            key = t->key;
            value = t->value;
            return true;
        } else {
            k -= leftSize + 1;
            t = t->right;
        }
    }
    return false;
}

/* ---- Range query ---- */

void AA_Tree3::rangeQueryRec(Node* t, double lo, double hi,
                               QVector<QPair<double, double>>& result) const
{
    if (!t) return;
    if (lo < t->key) rangeQueryRec(t->left, lo, hi, result);
    if (lo <= t->key && t->key <= hi)
        result.append({t->key, t->value});
    if (hi > t->key) rangeQueryRec(t->right, lo, hi, result);
}

QVector<QPair<double, double>> AA_Tree3::rangeQuery(double lo, double hi) const
{
    QVector<QPair<double, double>> result;
    rangeQueryRec(m_root, lo, hi, result);
    return result;
}

/* ---- In-order traversal ---- */

void AA_Tree3::inOrderRec(Node* t, QVector<QPair<double, double>>& result) const
{
    if (!t) return;
    inOrderRec(t->left, result);
    result.append({t->key, t->value});
    inOrderRec(t->right, result);
}

QVector<QPair<double, double>> AA_Tree3::inOrder() const
{
    QVector<QPair<double, double>> result;
    inOrderRec(m_root, result);
    return result;
}

/* ---- Utility ---- */

bool AA_Tree3::isEmpty() const { return m_root == nullptr; }

void AA_Tree3::clearRec(Node* t)
{
    if (!t) return;
    clearRec(t->left);
    clearRec(t->right);
    delete t;
}

void AA_Tree3::clear()
{
    clearRec(m_root);
    m_root = nullptr;
    m_stats.currentSize = 0;
    m_stats.treeHeight = 0;
}

void AA_Tree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
