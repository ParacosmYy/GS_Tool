/**
 * @file SplayTree11.cpp
 * @brief SplayTree11 实现
 *
 * 实现伸展树：手指搜索优化与半伸展降低摊还深度。
 */

#include "utils/tree253/SplayTree11.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplayTree11::SplayTree11(QObject *parent) : QObject(parent) {}
SplayTree11::~SplayTree11() { deleteTree(m_root); }

/* ---- Delete tree ---- */

void SplayTree11::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

/* ---- Rotations ---- */

void SplayTree11::rotateLeft(Node* x)
{
    Node* y = x->right;
    if (!y) return;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    m_stats.numRotations++;
}

void SplayTree11::rotateRight(Node* x)
{
    Node* y = x->left;
    if (!y) return;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) m_root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;
    m_stats.numRotations++;
}

/* ---- Zig: single rotation ---- */

void SplayTree11::zig(Node* x)
{
    if (x == x->parent->left) rotateRight(x->parent);
    else rotateLeft(x->parent);
}

/* ---- Zig-zig: same direction double rotation ---- */

void SplayTree11::zigZig(Node* x)
{
    Node* p = x->parent;
    Node* g = p->parent;
    if (x == p->left) {
        rotateRight(g);
        rotateRight(p);
    } else {
        rotateLeft(g);
        rotateLeft(p);
    }
}

/* ---- Zig-zag: opposite direction double rotation ---- */

void SplayTree11::zigZag(Node* x)
{
    Node* p = x->parent;
    if (x == p->left && p == p->parent->right) {
        rotateRight(p);
        rotateLeft(x->parent);
    } else if (x == p->right && p == p->parent->left) {
        rotateLeft(p);
        rotateRight(x->parent);
    }
}

/* ---- Full splay ---- */

void SplayTree11::splay(Node* x)
{
    if (!x) return;
    while (x->parent) {
        if (!x->parent->parent) {
            zig(x);
        } else if ((x == x->parent->left && x->parent == x->parent->parent->left)
                   || (x == x->parent->right && x->parent == x->parent->parent->right)) {
            zigZig(x);
        } else {
            zigZag(x);
        }
    }
    m_stats.numSplays++;
}

/* ---- Semi-splay: partial splay for amortized depth reduction ---- */

void SplayTree11::semiSplay(Node* x)
{
    if (!x) return;
    // Only splay halfway: stop after one zig-zig or zig-zag
    int halfSteps = 1;
    while (x->parent && halfSteps > 0) {
        if (!x->parent->parent) {
            zig(x);
        } else {
            zigZig(x);
        }
        halfSteps--;
    }
    m_stats.numSplays++;
}

/* ---- Find node ---- */

SplayTree11::Node* SplayTree11::findNode(int key) const
{
    Node* x = m_root;
    while (x) {
        if (key == x->key) return x;
        x = (key < x->key) ? x->left : x->right;
    }
    return nullptr;
}

/* ---- Minimum ---- */

SplayTree11::Node* SplayTree11::minimum(Node* x) const
{
    while (x && x->left) x = x->left;
    return x;
}

/* ---- Maximum ---- */

SplayTree11::Node* SplayTree11::maximum(Node* x) const
{
    while (x && x->right) x = x->right;
    return x;
}

/* ---- In-order traversal ---- */

void SplayTree11::inorder(Node* node, QVector<int>& result) const
{
    if (!node) return;
    inorder(node->left, result);
    result.append(node->key);
    inorder(node->right, result);
}

/* ---- Height ---- */

int SplayTree11::heightRec(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(heightRec(node->left), heightRec(node->right));
}

/* ---- Insert ---- */

void SplayTree11::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    Node* z = new Node();
    z->key = key;

    Node* y = nullptr;
    Node* x = m_root;
    while (x) {
        y = x;
        x = (z->key < x->key) ? x->left : x->right;
    }

    z->parent = y;
    if (!y) m_root = z;
    else if (z->key < y->key) y->left = z;
    else y->right = z;

    splay(z);
    m_finger = z;

    m_stats.numNodes++;
    m_stats.numInsertions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Remove ---- */

void SplayTree11::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    Node* z = findNode(key);
    if (!z) return;

    splay(z); // Bring to root

    Node* leftTree = z->left;
    Node* rightTree = z->right;

    if (!leftTree) {
        m_root = rightTree;
        if (rightTree) rightTree->parent = nullptr;
    } else if (!rightTree) {
        m_root = leftTree;
        leftTree->parent = nullptr;
    } else {
        leftTree->parent = nullptr;
        // Find max in left subtree
        Node* maxLeft = maximum(leftTree);
        splay(maxLeft);
        maxLeft->right = rightTree;
        rightTree->parent = maxLeft;
        m_root = maxLeft;
    }

    delete z;
    m_finger = m_root;

    m_stats.numNodes--;
    m_stats.numDeletions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Contains with splay ---- */

bool SplayTree11::contains(int key)
{
    Node* x = findNode(key);
    if (x) {
        splay(x);
        m_finger = x;
    }
    return x != nullptr;
}

/* ---- Finger search: search near last accessed key ---- */

bool SplayTree11::fingerSearch(int key)
{
    QElapsedTimer timer;
    timer.start();

    // If finger is valid, try searching from finger first
    if (m_finger) {
        // Check if key is near finger
        Node* current = m_finger;
        // Search locally around finger
        int steps = 0;
        const int maxSteps = 5; // Local search radius

        while (current && steps < maxSteps) {
            if (current->key == key) {
                semiSplay(current); // Use semi-splay for finger hits
                m_finger = current;
                m_stats.numFingerHits++;
                return true;
            }
            current = (key < current->key) ? current->left : current->right;
            steps++;
        }
    }

    // Fallback to standard search
    Node* x = findNode(key);
    if (x) {
        splay(x);
        m_finger = x;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return x != nullptr;
}

/* ---- Keys in sorted order ---- */

QVector<int> SplayTree11::keys() const
{
    QVector<int> result;
    inorder(m_root, result);
    return result;
}

/* ---- Height ---- */

int SplayTree11::height() const { return heightRec(m_root); }

/* ---- Clear ---- */

void SplayTree11::clear()
{
    deleteTree(m_root);
    m_root = nullptr;
    m_finger = nullptr;
    m_stats.numNodes = 0;
}

/* ---- Reset ---- */

void SplayTree11::resetStatistics()
{
    clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
