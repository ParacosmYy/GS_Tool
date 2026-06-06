/**
 * @file Treap6.cpp
 * @brief Treap6 实现
 *
 * 实现Treap：随机优先级堆序插入、split/merge删除、范围查询。
 */

#include "utils/tree173/Treap6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Treap6::Treap6(QObject *parent)
    : QObject(parent)
{
}

Treap6::~Treap6() { clearRec(m_root); }

/* ---- Random priority (LCG) ---- */

double Treap6::randomPriority() const
{
    m_rngState = m_rngState * 6364136223846793005ULL + 1442695040888963407ULL;
    return static_cast<double>(m_rngState & 0x7FFFFFFFFFFFFFFFULL)
           / static_cast<double>(0x7FFFFFFFFFFFFFFFULL);
}

/* ---- Node helpers ---- */

void Treap6::updateSize(Node* t)
{
    if (!t) return;
    t->size = 1;
    if (t->left) t->size += t->left->size;
    if (t->right) t->size += t->right->size;
}

int Treap6::computeHeight(Node* t)
{
    if (!t) return 0;
    return 1 + qMax(computeHeight(t->left), computeHeight(t->right));
}

/* ---- Rotations ---- */

Treap6::Node* Treap6::rotateRight(Node* t)
{
    Node* l = t->left;
    t->left = l->right;
    l->right = t;
    updateSize(t);
    updateSize(l);
    return l;
}

Treap6::Node* Treap6::rotateLeft(Node* t)
{
    Node* r = t->right;
    t->right = r->left;
    r->left = t;
    updateSize(t);
    updateSize(r);
    return r;
}

/* ---- Internal insert ---- */

Treap6::Node* Treap6::insertRec(Node* t, double key, double value, double priority)
{
    if (!t) return new Node(key, value, priority);

    if (key < t->key) {
        t->left = insertRec(t->left, key, value, priority);
        if (t->left->priority < t->priority)
            t = rotateRight(t);
    } else if (key > t->key) {
        t->right = insertRec(t->right, key, value, priority);
        if (t->right->priority < t->priority)
            t = rotateLeft(t);
    } else {
        t->value = value; /* Update existing */
    }

    updateSize(t);
    return t;
}

/* ---- Internal split ---- */

void Treap6::splitRec(Node* t, double key, Node*& left, Node*& right)
{
    if (!t) { left = right = nullptr; return; }

    if (key < t->key) {
        splitRec(t->left, key, left, t->left);
        right = t;
    } else {
        splitRec(t->right, key, t->right, right);
        left = t;
    }
    updateSize(t);
}

/* ---- Internal merge ---- */

Treap6::Node* Treap6::mergeRec(Node* left, Node* right)
{
    if (!left) return right;
    if (!right) return left;

    if (left->priority < right->priority) {
        left->right = mergeRec(left->right, right);
        updateSize(left);
        return left;
    } else {
        right->left = mergeRec(left, right->left);
        updateSize(right);
        return right;
    }
}

/* ---- Internal find ---- */

Treap6::Node* Treap6::findRec(Node* t, double key)
{
    while (t) {
        if (key < t->key) t = t->left;
        else if (key > t->key) t = t->right;
        else return t;
    }
    return nullptr;
}

/* ---- Range query ---- */

void Treap6::rangeRec(Node* t, double lo, double hi,
                        QVector<QPair<double, double>>& result)
{
    if (!t) return;
    if (lo < t->key) rangeRec(t->left, lo, hi, result);
    if (lo <= t->key && t->key <= hi)
        result.append({t->key, t->value});
    if (hi > t->key) rangeRec(t->right, lo, hi, result);
}

/* ---- In-order ---- */

void Treap6::inOrderRec(Node* t, QVector<QPair<double, double>>& result)
{
    if (!t) return;
    inOrderRec(t->left, result);
    result.append({t->key, t->value});
    inOrderRec(t->right, result);
}

/* ---- Clear ---- */

void Treap6::clearRec(Node* t)
{
    if (!t) return;
    clearRec(t->left);
    clearRec(t->right);
    delete t;
}

/* ---- Public: Insert ---- */

bool Treap6::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (findRec(m_root, key)) {
        /* Update value via re-insert */
        m_root = insertRec(m_root, key, value, randomPriority());
    } else {
        m_root = insertRec(m_root, key, value, randomPriority());
        m_stats.currentSize++;
    }

    m_stats.totalInserts++;
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInserts + m_stats.totalDeletes > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalDeletes) : 0.0;

    emit insertCompleted(key, m_stats.currentSize);
    return true;
}

/* ---- Public: Remove via split/merge ---- */

bool Treap6::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* left = nullptr;
    Node* mid = nullptr;
    Node* right = nullptr;

    /* Split into (< key) and (>= key) */
    splitRec(m_root, key - 1e-15, left, right);
    /* Split right into (== key) and (> key) */
    splitRec(right, key, mid, right);

    if (!mid || mid->key != key) {
        /* Not found, merge back */
        m_root = mergeRec(left, mergeRec(mid, right));
        return false;
    }

    /* Delete mid subtree */
    clearRec(mid);

    m_root = mergeRec(left, right);

    m_stats.currentSize--;
    m_stats.totalDeletes++;
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInserts + m_stats.totalDeletes > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalDeletes) : 0.0;

    emit deleteCompleted(key);
    return true;
}

/* ---- Public: Find ---- */

bool Treap6::find(double key, double& value) const
{
    Node* n = findRec(m_root, key);
    if (n) { value = n->value; return true; }
    return false;
}

/* ---- Public: Split ---- */

void Treap6::split(double key, Treap6& left, Treap6& right)
{
    left.clear();
    right.clear();

    Node* lRoot = nullptr;
    Node* rRoot = nullptr;
    splitRec(m_root, key, lRoot, rRoot);

    left.m_root = lRoot;
    right.m_root = rRoot;
    m_root = nullptr;

    left.m_stats.currentSize = lRoot ? lRoot->size : 0;
    right.m_stats.currentSize = rRoot ? rRoot->size : 0;
    m_stats.currentSize = 0;

    emit splitCompleted(left.m_stats.currentSize, right.m_stats.currentSize);
}

/* ---- Public: Merge ---- */

void Treap6::merge(Treap6& other)
{
    m_root = mergeRec(m_root, other.m_root);
    other.m_root = nullptr;
    m_stats.currentSize = m_root ? m_root->size : 0;
    other.m_stats.currentSize = 0;
}

/* ---- Public: Range query ---- */

QVector<QPair<double, double>> Treap6::rangeQuery(double keyLo, double keyHi) const
{
    QVector<QPair<double, double>> result;
    rangeRec(m_root, keyLo, keyHi, result);
    return result;
}

/* ---- Public: In-order ---- */

QVector<QPair<double, double>> Treap6::inOrder() const
{
    QVector<QPair<double, double>> result;
    inOrderRec(m_root, result);
    return result;
}

/* ---- Utility ---- */

bool Treap6::isEmpty() const { return m_root == nullptr; }

void Treap6::clear()
{
    clearRec(m_root);
    m_root = nullptr;
    m_stats.currentSize = 0;
    m_stats.treeHeight = 0;
}

/* ---- Statistics ---- */

void Treap6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
