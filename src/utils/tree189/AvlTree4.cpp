/**
 * @file AvlTree4.cpp
 * @brief AvlTree4 实现
 *
 * 实现AVL平衡二叉树：插入删除旋转、穿线中序遍历、第k小查询、范围查询。
 */

#include "utils/tree189/AvlTree4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AvlTree4::AvlTree4(QObject *parent) : QObject(parent) {}
AvlTree4::~AvlTree4() { deleteTree(m_root); }

/* ---- Balance factor ---- */

int AvlTree4::balanceFactor(Node* n) const
{
    return n ? nodeHeight(n->left) - nodeHeight(n->right) : 0;
}

/* ---- Update height and subtree size ---- */

void AvlTree4::updateNode(Node* n)
{
    if (!n) return;
    n->height = 1 + qMax(nodeHeight(n->left), nodeHeight(n->right));
    n->subtreeSize = 1;
    if (n->left) n->subtreeSize += n->left->subtreeSize;
    if (n->right) n->subtreeSize += n->right->subtreeSize;
}

/* ---- Right rotation ---- */

AvlTree4::Node* AvlTree4::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    if (x->right) x->right->parent = y;
    x->right = y;
    x->parent = y->parent;
    y->parent = x;
    updateNode(y);
    updateNode(x);
    return x;
}

/* ---- Left rotation ---- */

AvlTree4::Node* AvlTree4::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->left = x;
    y->parent = x->parent;
    x->parent = y;
    updateNode(x);
    updateNode(y);
    return y;
}

/* ---- Rebalance ---- */

AvlTree4::Node* AvlTree4::rebalance(Node* n)
{
    if (!n) return nullptr;
    updateNode(n);
    int bf = balanceFactor(n);

    if (bf > 1) {
        if (balanceFactor(n->left) < 0)
            n->left = rotateLeft(n->left);
        return rotateRight(n);
    }
    if (bf < -1) {
        if (balanceFactor(n->right) > 0)
            n->right = rotateRight(n->right);
        return rotateLeft(n);
    }
    return n;
}

/* ---- Insert helper ---- */

AvlTree4::Node* AvlTree4::insertNode(Node* n, double key, Node* parent)
{
    if (!n) {
        Node* node = new Node{key, 1, 1, nullptr, nullptr, parent, false, false};
        m_size++;
        return node;
    }
    if (key < n->key)
        n->left = insertNode(n->left, key, n);
    else if (key > n->key)
        n->right = insertNode(n->right, key, n);
    else
        return n; // Duplicate, ignore
    return rebalance(n);
}

/* ---- Find minimum ---- */

AvlTree4::Node* AvlTree4::findMin(Node* n) const
{
    while (n && n->left) n = n->left;
    return n;
}

/* ---- Remove helper ---- */

AvlTree4::Node* AvlTree4::removeNode(Node* n, double key)
{
    if (!n) return nullptr;

    if (key < n->key) {
        n->left = removeNode(n->left, key);
    } else if (key > n->key) {
        n->right = removeNode(n->right, key);
    } else {
        // Found the node
        if (!n->left || !n->right) {
            Node* child = n->left ? n->left : n->right;
            if (child) child->parent = n->parent;
            delete n;
            m_size--;
            return child;
        }
        // Two children: replace with in-order successor
        Node* succ = findMin(n->right);
        n->key = succ->key;
        n->right = removeNode(n->right, succ->key);
    }
    return rebalance(n);
}

/* ---- Find node ---- */

AvlTree4::Node* AvlTree4::findNode(Node* n, double key) const
{
    while (n) {
        if (key < n->key) n = n->left;
        else if (key > n->key) n = n->right;
        else return n;
    }
    return nullptr;
}

/* ---- Public insert ---- */

void AvlTree4::insert(double key)
{
    QElapsedTimer timer;
    timer.start();
    m_root = insertNode(m_root, key, nullptr);
    updateThreading();

    m_stats.totalOperations++;
    m_stats.numNodes = m_size;
    m_stats.treeHeight = height();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("insert", m_size, timer.elapsed());
}

/* ---- Public remove ---- */

void AvlTree4::remove(double key)
{
    QElapsedTimer timer;
    timer.start();
    m_root = removeNode(m_root, key);
    updateThreading();

    m_stats.totalOperations++;
    m_stats.numNodes = m_size;
    m_stats.treeHeight = height();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("remove", m_size, timer.elapsed());
}

/* ---- Contains ---- */

bool AvlTree4::contains(double key) const
{
    return findNode(m_root, key) != nullptr;
}

/* ---- Update threading for all nodes ---- */

void AvlTree4::updateThreading()
{
    // Reset all thread flags
    QVector<Node*> stack;
    QVector<Node*> order;
    Node* cur = m_root;
    while (cur || !stack.isEmpty()) {
        while (cur) { stack.append(cur); cur = cur->left; }
        cur = stack.back(); stack.removeLast();
        order.append(cur);
        cur = cur->right;
    }

    for (int i = 0; i < order.size(); ++i) {
        order[i]->leftThread = false;
        order[i]->rightThread = false;
    }

    // Set threading for nodes with null children
    for (int i = 0; i < order.size(); ++i) {
        if (!order[i]->left && i > 0) {
            order[i]->left = order[i - 1];
            order[i]->leftThread = true;
        }
        if (!order[i]->right && i < order.size() - 1) {
            order[i]->right = order[i + 1];
            order[i]->rightThread = true;
        }
    }
}

/* ---- Threaded inorder traversal ---- */

QVector<double> AvlTree4::threadedInorder() const
{
    QVector<double> result;
    if (!m_root) return result;

    // Find leftmost node
    Node* cur = m_root;
    while (cur->left && !cur->leftThread) cur = cur->left;

    while (cur) {
        result.append(cur->key);
        if (cur->rightThread) {
            cur = cur->right;
        } else {
            cur = cur->right;
            if (cur) {
                while (cur->left && !cur->leftThread) cur = cur->left;
            }
        }
    }
    return result;
}

/* ---- k-th smallest ---- */

AvlTree4::Node* AvlTree4::kthNode(Node* n, int k) const
{
    if (!n) return nullptr;
    int leftSize = n->left ? n->left->subtreeSize : 0;
    if (k <= leftSize) return kthNode(n->left, k);
    if (k == leftSize + 1) return n;
    return kthNode(n->right, k - leftSize - 1);
}

double AvlTree4::kthSmallest(int k) const
{
    if (k < 1 || k > m_size) return qQNaN();
    Node* n = kthNode(m_root, k);
    return n ? n->key : qQNaN();
}

/* ---- Range query ---- */

QVector<double> AvlTree4::rangeQuery(double lo, double hi) const
{
    QVector<double> result;
    // Use threaded inorder to collect
    auto all = threadedInorder();
    for (double v : all) {
        if (v >= lo && v <= hi) result.append(v);
    }
    return result;
}

/* ---- Tree height ---- */

int AvlTree4::height() const { return nodeHeight(m_root); }

/* ---- Clear ---- */

void AvlTree4::clear()
{
    deleteTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/* ---- Delete tree ---- */

void AvlTree4::deleteTree(Node* n)
{
    if (!n) return;
    if (!n->leftThread) deleteTree(n->left);
    if (!n->rightThread) deleteTree(n->right);
    delete n;
}

/* ---- Reset statistics ---- */

void AvlTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
