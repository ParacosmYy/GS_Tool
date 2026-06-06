/**
 * @file Treap7.cpp
 * @brief Treap7 实现
 *
 * 实现Treap随机化平衡树：堆优先级旋转维护、split/merge操作、顺序统计。
 */

#include "utils/tree183/Treap7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

Treap7::Treap7(QObject *parent) : QObject(parent) {}
Treap7::~Treap7() { deleteTree(m_root); }

void Treap7::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

/* ---- Random priority ---- */

int Treap7::randomPriority() const
{
    return qrand();
}

/* ---- Size update ---- */

void Treap7::updateSize(Node* node)
{
    if (!node) return;
    node->size = 1;
    if (node->left) node->size += node->left->size;
    if (node->right) node->size += node->right->size;
}

/* ---- Rotations ---- */

Treap7::Node* Treap7::rotateRight(Node* node)
{
    if (!node || !node->left) return node;
    Node* left = node->left;
    node->left = left->right;
    left->right = node;
    updateSize(node);
    updateSize(left);
    return left;
}

Treap7::Node* Treap7::rotateLeft(Node* node)
{
    if (!node || !node->right) return node;
    Node* right = node->right;
    node->right = right->left;
    right->left = node;
    updateSize(node);
    updateSize(right);
    return right;
}

/* ---- Insert ---- */

Treap7::Node* Treap7::insertNode(Node* node, int key)
{
    if (!node) return new Node{key, randomPriority(), 1, nullptr, nullptr};

    if (key < node->key) {
        node->left = insertNode(node->left, key);
        // Heap property: parent priority should be >= children
        if (node->left->priority > node->priority)
            node = rotateRight(node);
    } else if (key > node->key) {
        node->right = insertNode(node->right, key);
        if (node->right->priority > node->priority)
            node = rotateLeft(node);
    }
    // Duplicate key: no-op

    updateSize(node);
    return node;
}

/* ---- Remove ---- */

Treap7::Node* Treap7::removeNode(Node* node, int key)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeNode(node->left, key);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key);
    } else {
        // Found: rotate down to leaf and delete
        if (!node->left && !node->right) {
            delete node;
            return nullptr;
        }
        // Rotate with higher-priority child
        if (!node->left) {
            node = rotateLeft(node);
            node->left = removeNode(node->left, key);
        } else if (!node->right) {
            node = rotateRight(node);
            node->right = removeNode(node->right, key);
        } else if (node->left->priority > node->right->priority) {
            node = rotateRight(node);
            node->right = removeNode(node->right, key);
        } else {
            node = rotateLeft(node);
            node->left = removeNode(node->left, key);
        }
    }

    updateSize(node);
    return node;
}

/* ---- Search ---- */

bool Treap7::searchNode(Node* node, int key) const
{
    while (node) {
        if (key < node->key) node = node->left;
        else if (key > node->key) node = node->right;
        else return true;
    }
    return false;
}

/* ---- Split ---- */

QPair<Treap7::Node*, Treap7::Node*> Treap7::splitNode(Node* node, int key)
{
    if (!node) return {nullptr, nullptr};

    if (key <= node->key) {
        auto [left, right] = splitNode(node->left, key);
        node->left = right;
        updateSize(node);
        return {left, node};
    } else {
        auto [left, right] = splitNode(node->right, key);
        node->right = left;
        updateSize(node);
        return {node, right};
    }
}

/* ---- Merge ---- */

Treap7::Node* Treap7::mergeNode(Node* a, Node* b)
{
    if (!a) return b;
    if (!b) return a;

    if (a->priority > b->priority) {
        a->right = mergeNode(a->right, b);
        updateSize(a);
        return a;
    } else {
        b->left = mergeNode(a, b->left);
        updateSize(b);
        return b;
    }
}

/* ---- Kth ---- */

int Treap7::kthNode(Node* node, int k) const
{
    if (!node) return -1;
    int leftSize = node->left ? node->left->size : 0;

    if (k <= leftSize) return kthNode(node->left, k);
    if (k == leftSize + 1) return node->key;
    return kthNode(node->right, k - leftSize - 1);
}

/* ---- Rank ---- */

int Treap7::rankNode(Node* node, int key) const
{
    if (!node) return 1;
    if (key <= node->key)
        return rankNode(node->left, key);
    int leftSize = node->left ? node->left->size : 0;
    return leftSize + 1 + rankNode(node->right, key);
}

/* ---- Inorder ---- */

void Treap7::inorderHelper(Node* node, QVector<int>& result) const
{
    if (!node) return;
    inorderHelper(node->left, result);
    result.append(node->key);
    inorderHelper(node->right, result);
}

/* ---- Public API ---- */

void Treap7::insert(int key)
{
    QElapsedTimer timer;
    timer.start();
    m_root = insertNode(m_root, key);

    m_stats.totalOperations++;
    m_stats.numNodes = size();
    m_stats.treeHeight = treeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("insert", key);
}

void Treap7::remove(int key)
{
    QElapsedTimer timer;
    timer.start();
    m_root = removeNode(m_root, key);

    m_stats.totalOperations++;
    m_stats.numNodes = size();
    m_stats.treeHeight = treeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("remove", key);
}

bool Treap7::contains(int key) const { return searchNode(m_root, key); }

int Treap7::kth(int k) const
{
    if (k < 1 || k > size()) return -1;
    return kthNode(m_root, k);
}

int Treap7::rank(int key) const { return rankNode(m_root, key); }

int Treap7::predecessor(int key) const
{
    int result = -1;
    Node* node = m_root;
    while (node) {
        if (node->key < key) { result = node->key; node = node->right; }
        else node = node->left;
    }
    return result;
}

int Treap7::successor(int key) const
{
    int result = -1;
    Node* node = m_root;
    while (node) {
        if (node->key > key) { result = node->key; node = node->left; }
        else node = node->right;
    }
    return result;
}

/* ---- Split (public) ---- */

QPair<Treap7*, Treap7*> Treap7::split(int key) const
{
    auto [left, right] = splitNode(m_root, key);
    Treap7* t1 = new Treap7();
    Treap7* t2 = new Treap7();
    t1->m_root = left;
    t2->m_root = right;
    return {t1, t2};
}

/* ---- Merge (public, static) ---- */

Treap7* Treap7::merge(Treap7* a, Treap7* b)
{
    Treap7* result = new Treap7();
    result->m_root = mergeNode(a ? a->m_root : nullptr, b ? b->m_root : nullptr);
    return result;
}

QVector<int> Treap7::inorder() const
{
    QVector<int> result;
    inorderHelper(m_root, result);
    return result;
}

int Treap7::size() const { return m_root ? m_root->size : 0; }

void Treap7::clear()
{
    deleteTree(m_root);
    m_root = nullptr;
}

int Treap7::treeHeight(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(treeHeight(node->left), treeHeight(node->right));
}

/* ---- Reset ---- */

void Treap7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
