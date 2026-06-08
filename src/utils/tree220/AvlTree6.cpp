/**
 * @file AvlTree6.cpp
 * @brief AvlTree6 实现
 *
 * 实现AVL树：写时复制快照、世代内存回收、平衡旋转。
 */

#include "utils/tree220/AvlTree6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AvlTree6::AvlTree6(QObject *parent)
    : QObject(parent), m_currentEpoch(1)
{
}

AvlTree6::~AvlTree6() { clear(); }

/* ---- Height helpers ---- */

int AvlTree6::getHeight(Node* node) const
{
    return node ? node->height : 0;
}

int AvlTree6::balanceFactor(Node* node) const
{
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

void AvlTree6::updateHeight(Node* node)
{
    if (node)
        node->height = 1 + qMax(getHeight(node->left), getHeight(node->right));
}

/* ---- Rotations ---- */

AvlTree6::Node* AvlTree6::rotateRight(Node* y)
{
    Node* x = y->left;
    Node* T2 = x->right;

    x->right = y;
    y->left = T2;

    updateHeight(y);
    updateHeight(x);
    return x;
}

AvlTree6::Node* AvlTree6::rotateLeft(Node* x)
{
    Node* y = x->right;
    Node* T2 = y->left;

    y->left = x;
    x->right = T2;

    updateHeight(x);
    updateHeight(y);
    return y;
}

/* ---- Balance ---- */

AvlTree6::Node* AvlTree6::balance(Node* node)
{
    updateHeight(node);
    int bf = balanceFactor(node);

    // Left heavy
    if (bf > 1) {
        if (balanceFactor(node->left) < 0)
            node->left = rotateLeft(node->left);
        return rotateRight(node);
    }

    // Right heavy
    if (bf < -1) {
        if (balanceFactor(node->right) > 0)
            node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

/* ---- Copy-on-write clone ---- */

AvlTree6::Node* AvlTree6::cowClone(Node* node)
{
    if (!node) return nullptr;
    Node* cloned = new Node{node->key, node->value, node->height, 1,
                             node->left, node->right};
    // Increment ref count on children (shared until modified)
    if (node->left) node->left->refCount++;
    if (node->right) node->right->refCount++;
    return cloned;
}

/* ---- Recursive insert ---- */

AvlTree6::Node* AvlTree6::insertImpl(Node* node, int key, double value)
{
    if (!node) {
        return new Node{key, value, 1, 1, nullptr, nullptr};
    }

    // COW: clone on write path
    Node* n = cowClone(node);
    safeDelete(node);

    if (key < n->key) {
        n->left = insertImpl(n->left, key, value);
    } else if (key > n->key) {
        n->right = insertImpl(n->right, key, value);
    } else {
        n->value = value;  // Update existing
        return n;
    }

    return balance(n);
}

/* ---- Find minimum ---- */

AvlTree6::Node* AvlTree6::findMin(Node* node) const
{
    while (node && node->left) node = node->left;
    return node;
}

/* ---- Recursive remove ---- */

AvlTree6::Node* AvlTree6::removeImpl(Node* node, int key)
{
    if (!node) return nullptr;

    Node* n = cowClone(node);
    safeDelete(node);

    if (key < n->key) {
        n->left = removeImpl(n->left, key);
    } else if (key > n->key) {
        n->right = removeImpl(n->right, key);
    } else {
        if (!n->left || !n->right) {
            Node* child = n->left ? n->left : n->right;
            delete n;
            return child;
        }

        Node* succ = findMin(n->right);
        n->key = succ->key;
        n->value = succ->value;
        n->right = removeImpl(n->right, succ->key);
    }

    return balance(n);
}

/* ---- Recursive search ---- */

double AvlTree6::searchImpl(Node* node, int key) const
{
    if (!node) return qQNaN();
    if (key < node->key) return searchImpl(node->left, key);
    if (key > node->key) return searchImpl(node->right, key);
    return node->value;
}

/* ---- In-order traversal ---- */

void AvlTree6::inOrderImpl(Node* node, QVector<int>& result) const
{
    if (!node) return;
    inOrderImpl(node->left, result);
    result.append(node->key);
    inOrderImpl(node->right, result);
}

/* ---- Safe delete with ref counting ---- */

void AvlTree6::safeDelete(Node* node)
{
    if (!node) return;
    node->refCount--;
    if (node->refCount <= 0) {
        safeDelete(node->left);
        safeDelete(node->right);
        delete node;
    }
}

/* ---- Compute tree height ---- */

int AvlTree6::computeHeight(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(computeHeight(node->left), computeHeight(node->right));
}

/* ---- Insert ---- */

void AvlTree6::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertImpl(m_root, key, value);

    m_stats.nodeCount++;
    m_stats.numInserts++;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("insert", key, timer.elapsed());
}

/* ---- Remove ---- */

void AvlTree6::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = removeImpl(m_root, key);

    m_stats.nodeCount = qMax(0, m_stats.nodeCount - 1);
    m_stats.numDeletes++;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("remove", key, timer.elapsed());
}

/* ---- Search ---- */

double AvlTree6::search(int key) const
{
    return searchImpl(m_root, key);
}

/* ---- Contains ---- */

bool AvlTree6::contains(int key) const
{
    return !qIsNaN(search(key));
}

/* ---- Create snapshot ---- */

void AvlTree6::createSnapshot()
{
    if (m_root) {
        m_root->refCount++;  // Pin current root
        m_snapshots.append(m_root);
        m_snapshotEpochs.append(m_currentEpoch.load());
        m_stats.numSnapshots++;
        emit snapshotCreated(m_stats.numSnapshots, m_stats.nodeCount);
    }
}

/* ---- Reclaim epoch ---- */

void AvlTree6::reclaimEpoch()
{
    int currentEpoch = m_currentEpoch.load();
    int safeEpoch = currentEpoch - 2;  // Keep last 2 epochs

    // Reclaim snapshots from old epochs
    for (int i = m_snapshots.size() - 1; i >= 0; --i) {
        if (m_snapshotEpochs[i] <= safeEpoch) {
            safeDelete(m_snapshots[i]);
            m_snapshots.removeAt(i);
            m_snapshotEpochs.removeAt(i);
        }
    }
    m_currentEpoch.fetchAndAddRelaxed(1);
}

/* ---- In-order keys ---- */

QVector<int> AvlTree6::inOrderKeys() const
{
    QVector<int> result;
    inOrderImpl(m_root, result);
    return result;
}

/* ---- Clear ---- */

void AvlTree6::clear()
{
    safeDelete(m_root);
    m_root = nullptr;
    for (auto* snap : m_snapshots) safeDelete(snap);
    m_snapshots.clear();
    m_snapshotEpochs.clear();
}

/* ---- Reset ---- */

void AvlTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    clear();
}
