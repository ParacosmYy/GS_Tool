/**
 * @file AvlTree9.cpp
 * @brief AvlTree9 实现
 *
 * 实现AVL树：线程化节点O(1)后继前驱与父指针重平衡高效迭代。
 */

#include "utils/tree262/AvlTree9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

AvlTree9::AvlTree9(QObject *parent)
    : QObject(parent) {}
AvlTree9::~AvlTree9() { clearHelper(m_root); }

/* ---- Rotation helpers ---- */

AvlTree9::Node* AvlTree9::rotateRight(Node* node)
{
    if (!node || !node->left) return node;
    Node* l = node->left;
    bool leftWasThread = node->leftIsThread;

    node->left = l->right;
    node->leftIsThread = l->rightIsThread;
    l->right = node;
    l->rightIsThread = false;
    l->parent = node->parent;
    node->parent = l;

    // Fix parent's child pointer
    if (l->parent) {
        if (l->parent->left == node) l->parent->left = l;
        else l->parent->right = l;
    }

    // Update balance factors
    node->balanceFactor = qMax(-1, qMin(1, node->balanceFactor + 1 - qMin(0, l->balanceFactor)));
    l->balanceFactor = qMax(-1, qMin(1, l->balanceFactor + 1 + qMax(0, node->balanceFactor)));

    repairThreads(node);
    m_rotations++;
    return l;
}

AvlTree9::Node* AvlTree9::rotateLeft(Node* node)
{
    if (!node || !node->right) return node;
    Node* r = node->right;
    bool rightWasThread = node->rightIsThread;

    node->right = r->left;
    node->rightIsThread = r->leftIsThread;
    r->left = node;
    r->leftIsThread = false;
    r->parent = node->parent;
    node->parent = r;

    // Fix parent's child pointer
    if (r->parent) {
        if (r->parent->left == node) r->parent->left = r;
        else r->parent->right = r;
    }

    // Update balance factors
    node->balanceFactor = qMax(-1, qMin(1, node->balanceFactor - 1 - qMax(0, r->balanceFactor)));
    r->balanceFactor = qMax(-1, qMin(1, r->balanceFactor - 1 + qMin(0, node->balanceFactor)));

    repairThreads(node);
    m_rotations++;
    return r;
}

/* ---- Repair thread links after rotation ---- */

void AvlTree9::repairThreads(Node* node)
{
    // Rebuild thread links in the subtree rooted at node
    // Thread next/prev for the rotated node
    if (node) {
        // Successor: leftmost in right subtree or parent
        if (node->right && !node->rightIsThread) {
            Node* succ = findMin(node->right);
            node->threadNext = succ;
            if (succ) succ->threadPrev = node;
        }
        // Predecessor: rightmost in left subtree or parent
        if (node->left && !node->leftIsThread) {
            Node* pred = findMax(node->left);
            node->threadPrev = pred;
            if (pred) pred->threadNext = node;
        }
    }
}

/* ---- Update balance factor ---- */

int AvlTree9::updateBalance(Node* node) const
{
    if (!node) return 0;
    int lh = 0, rh = 0;
    if (node->left && !node->leftIsThread) lh = 1 + updateBalance(node->left);
    if (node->right && !node->rightIsThread) rh = 1 + updateBalance(node->right);
    return rh - lh;
}

/* ---- Update thread links after insertion ---- */

void AvlTree9::updateThreads(Node* newNode, Node* parent, bool isLeft)
{
    newNode->parent = parent;
    if (isLeft) {
        // New node is left child; its successor is parent
        newNode->threadNext = parent;
        parent->threadPrev = newNode;
        // New node's predecessor is parent's old predecessor
        newNode->threadPrev = parent->leftIsThread ? nullptr : nullptr;
        if (parent->left && parent->left != newNode && parent->left->threadNext == parent) {
            parent->left->threadNext = newNode;
            newNode->threadPrev = parent->left;
        }
    } else {
        // New node is right child; its predecessor is parent
        newNode->threadPrev = parent;
        parent->threadNext = newNode;
        // New node's successor is parent's old successor
        if (parent->right && parent->right != newNode && parent->right->threadPrev == parent) {
            parent->right->threadPrev = newNode;
            newNode->threadNext = parent->right;
        }
    }
}

/* ---- Rebalance using parent pointers ---- */

void AvlTree9::rebalance(Node* node)
{
    while (node) {
        node->balanceFactor = updateBalance(node);
        if (node->balanceFactor > 1) {
            // Right-heavy
            if (node->right && node->right->balanceFactor < 0) {
                rotateRight(node->right);
            }
            Node* newRoot = rotateLeft(node);
            if (node == m_root) m_root = newRoot;
            node = newRoot;
        } else if (node->balanceFactor < -1) {
            // Left-heavy
            if (node->left && node->left->balanceFactor > 0) {
                rotateLeft(node->left);
            }
            Node* newRoot = rotateRight(node);
            if (node == m_root) m_root = newRoot;
            node = newRoot;
        }
        node = node->parent;
    }
}

/* ---- Find node ---- */

AvlTree9::Node* AvlTree9::findNode(int key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            if (cur->leftIsThread) break;
            cur = cur->left;
        } else if (key > cur->key) {
            if (cur->rightIsThread) break;
            cur = cur->right;
        } else {
            return cur;
        }
    }
    return nullptr;
}

/* ---- Find min/max ---- */

AvlTree9::Node* AvlTree9::findMin(Node* node) const
{
    if (!node) return nullptr;
    while (node->left && !node->leftIsThread)
        node = node->left;
    return node;
}

AvlTree9::Node* AvlTree9::findMax(Node* node) const
{
    if (!node) return nullptr;
    while (node->right && !node->rightIsThread)
        node = node->right;
    return node;
}

/* ---- Insert ---- */

void AvlTree9::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    Node* newNode = new Node{key, 0, nullptr, nullptr, nullptr, nullptr, nullptr, false, false};

    if (!m_root) {
        m_root = newNode;
    } else {
        Node* cur = m_root;
        Node* parent = nullptr;
        bool isLeft = false;
        while (cur) {
            parent = cur;
            if (key < cur->key) {
                if (cur->leftIsThread || !cur->left) { isLeft = true; break; }
                cur = cur->left;
            } else if (key > cur->key) {
                if (cur->rightIsThread || !cur->right) { isLeft = false; break; }
                cur = cur->right;
            } else {
                delete newNode;  // Duplicate
                return;
            }
        }

        newNode->parent = parent;
        if (isLeft) {
            // Take over thread links
            newNode->leftIsThread = true;
            newNode->left = parent->leftIsThread ? nullptr : nullptr;
            newNode->rightIsThread = true;
            newNode->right = parent;
            newNode->threadPrev = parent->threadPrev;
            newNode->threadNext = parent;
            if (parent->threadPrev) parent->threadPrev->threadNext = newNode;
            parent->threadPrev = newNode;
            parent->left = newNode;
            parent->leftIsThread = false;
        } else {
            newNode->leftIsThread = true;
            newNode->left = parent;
            newNode->rightIsThread = true;
            newNode->right = parent->rightIsThread ? nullptr : nullptr;
            newNode->threadPrev = parent;
            newNode->threadNext = parent->threadNext;
            if (parent->threadNext) parent->threadNext->threadPrev = newNode;
            parent->threadNext = newNode;
            parent->right = newNode;
            parent->rightIsThread = false;
        }

        rebalance(parent);
    }

    double elapsed = timer.elapsed();
    m_stats.numNodes = size();
    m_stats.treeHeight = height();
    m_stats.numRotations = m_rotations;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeModified(m_stats.numNodes, m_stats.treeHeight, m_rotations, elapsed);
}

/* ---- Remove ---- */

void AvlTree9::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    Node* target = findNode(key);
    if (!target) return;

    // Repair thread links before removal
    if (target->threadPrev) target->threadPrev->threadNext = target->threadNext;
    if (target->threadNext) target->threadNext->threadPrev = target->threadPrev;

    Node* rebalanceStart = target->parent;

    if (!target->left || target->leftIsThread) {
        // No left child: replace with right
        Node* child = target->rightIsThread ? nullptr : target->right;
        if (target->parent) {
            if (target->parent->left == target) {
                target->parent->left = child;
                if (!child) {
                    target->parent->leftIsThread = true;
                    target->parent->left = nullptr;
                }
            } else {
                target->parent->right = child;
                if (!child) {
                    target->parent->rightIsThread = true;
                    target->parent->right = nullptr;
                }
            }
            if (child) child->parent = target->parent;
        } else {
            m_root = child;
            if (child) child->parent = nullptr;
        }
        delete target;
    } else if (!target->right || target->rightIsThread) {
        // No right child: replace with left
        Node* child = target->leftIsThread ? nullptr : target->left;
        if (target->parent) {
            if (target->parent->left == target)
                target->parent->left = child;
            else
                target->parent->right = child;
            if (child) child->parent = target->parent;
        } else {
            m_root = child;
            if (child) child->parent = nullptr;
        }
        delete target;
    } else {
        // Two children: replace with in-order successor
        Node* succ = findMin(target->right);
        target->key = succ->key;
        // Now remove the successor
        rebalanceStart = succ->parent;
        if (succ->threadPrev && succ->threadPrev != target)
            succ->threadPrev->threadNext = succ->threadNext;
        if (succ->threadNext)
            succ->threadNext->threadPrev = succ->threadPrev;
        if (succ->parent->left == succ) {
            succ->parent->left = succ->rightIsThread ? nullptr : succ->right;
            if (!succ->right || succ->rightIsThread)
                succ->parent->leftIsThread = true;
        } else {
            succ->parent->right = succ->rightIsThread ? nullptr : succ->right;
            if (!succ->right || succ->rightIsThread)
                succ->parent->rightIsThread = true;
        }
        if (succ->right && !succ->rightIsThread)
            succ->right->parent = succ->parent;
        delete succ;
    }

    if (rebalanceStart) rebalance(rebalanceStart);
    if (m_root) m_root->balanceFactor = updateBalance(m_root);

    double elapsed = timer.elapsed();
    m_stats.numNodes = size();
    m_stats.treeHeight = height();
    m_stats.numRotations = m_rotations;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeModified(m_stats.numNodes, m_stats.treeHeight, m_rotations, elapsed);
}

/* ---- Contains ---- */

bool AvlTree9::contains(int key) const { return findNode(key) != nullptr; }

/* ---- Successor (O(1) via threading) ---- */

int AvlTree9::successor(int key) const
{
    Node* node = findNode(key);
    if (!node || !node->threadNext) return std::numeric_limits<int>::max();
    return node->threadNext->key;
}

/* ---- Predecessor (O(1) via threading) ---- */

int AvlTree9::predecessor(int key) const
{
    Node* node = findNode(key);
    if (!node || !node->threadPrev) return std::numeric_limits<int>::min();
    return node->threadPrev->key;
}

/* ---- In-order traversal ---- */

void AvlTree9::inOrderHelper(Node* node, QVector<int>& result) const
{
    if (!node) return;
    if (!node->leftIsThread) inOrderHelper(node->left, result);
    result.append(node->key);
    if (!node->rightIsThread) inOrderHelper(node->right, result);
}

QVector<int> AvlTree9::inOrder() const
{
    QVector<int> result;
    inOrderHelper(m_root, result);
    return result;
}

/* ---- Height ---- */

int AvlTree9::heightHelper(Node* node) const
{
    if (!node) return 0;
    int lh = (node->left && !node->leftIsThread) ? heightHelper(node->left) : 0;
    int rh = (node->right && !node->rightIsThread) ? heightHelper(node->right) : 0;
    return 1 + qMax(lh, rh);
}

int AvlTree9::height() const { return heightHelper(m_root); }

/* ---- Size ---- */

int AvlTree9::size() const
{
    int count = 0;
    QVector<Node*> stack;
    if (m_root) stack.append(m_root);
    while (!stack.isEmpty()) {
        Node* n = stack.takeLast();
        count++;
        if (n->right && !n->rightIsThread) stack.append(n->right);
        if (n->left && !n->leftIsThread) stack.append(n->left);
    }
    return count;
}

/* ---- Clear ---- */

void AvlTree9::clearHelper(Node* node)
{
    if (!node) return;
    if (!node->leftIsThread) clearHelper(node->left);
    if (!node->rightIsThread) clearHelper(node->right);
    delete node;
}

/* ---- Reset ---- */

void AvlTree9::resetStatistics()
{
    clearHelper(m_root);
    m_root = nullptr;
    m_rotations = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
