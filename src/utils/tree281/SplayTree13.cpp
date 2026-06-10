/**
 * @file SplayTree13.cpp
 * @brief SplayTree13 实现
 *
 * 实现伸展树：手指搜索与动态手指定理保证的局部性感知访问模式。
 */

#include "utils/tree281/SplayTree13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplayTree13::SplayTree13(QObject *parent)
    : QObject(parent) {}

SplayTree13::~SplayTree13()
{
    destroyTree(m_root);
}

/* ---- Destroy tree recursively ---- */

void SplayTree13::destroyTree(Node* x)
{
    if (!x) return;
    destroyTree(x->left);
    destroyTree(x->right);
    delete x;
}

/* ---- Set child helper ---- */

void SplayTree13::setChild(Node* parent, Node* child, bool isLeft)
{
    if (parent) {
        if (isLeft) parent->left = child;
        else parent->right = child;
    }
    if (child) child->parent = parent;
}

/* ---- Rotate left ---- */

void SplayTree13::rotateLeft(Node* x)
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

/* ---- Rotate right ---- */

void SplayTree13::rotateRight(Node* y)
{
    Node* x = y->left;
    if (!x) return;
    y->left = x->right;
    if (x->right) x->right->parent = y;
    x->parent = y->parent;
    if (!y->parent) m_root = x;
    else if (y == y->parent->right) y->parent->right = x;
    else y->parent->left = x;
    x->right = y;
    y->parent = x;
    m_stats.numRotations++;
}

/* ---- Zig step: x is direct child of root ---- */

void SplayTree13::zig(Node* x)
{
    if (x == x->parent->left) rotateRight(x->parent);
    else rotateLeft(x->parent);
}

/* ---- Zig-zig step ---- */

void SplayTree13::zigZig(Node* x)
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

/* ---- Zig-zag step ---- */

void SplayTree13::zigZag(Node* x)
{
    Node* p = x->parent;
    Node* g = p->parent;
    if (x == p->left && p == g->left) {
        rotateRight(p);
        rotateRight(g);
    } else if (x == p->right && p == g->right) {
        rotateLeft(p);
        rotateLeft(g);
    } else if (x == p->right && p == g->left) {
        rotateLeft(p);
        rotateRight(g);
    } else {
        rotateRight(p);
        rotateLeft(g);
    }
}

/* ---- Splay node x to root ---- */

void SplayTree13::splay(Node* x)
{
    if (!x) return;
    while (x->parent) {
        Node* p = x->parent;
        if (!p->parent) {
            // Zig case
            zig(x);
        } else {
            // Check zig-zig vs zig-zag
            Node* g = p->parent;
            bool xIsLeft = (x == p->left);
            bool pIsLeft = (p == g->left);
            if (xIsLeft == pIsLeft) zigZig(x);
            else zigZag(x);
        }
    }
    m_root = x;
    m_stats.numSplays++;
}

/* ---- Find node by key ---- */

SplayTree13::Node* SplayTree13::findNode(double key) const
{
    Node* x = m_root;
    while (x) {
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else return x;
    }
    return nullptr;
}

/* ---- Subtree min/max ---- */

SplayTree13::Node* SplayTree13::subtreeMin(Node* x) const
{
    if (!x) return nullptr;
    while (x->left) x = x->left;
    return x;
}

SplayTree13::Node* SplayTree13::subtreeMax(Node* x) const
{
    if (!x) return nullptr;
    while (x->right) x = x->right;
    return x;
}

/* ---- Join two subtrees ---- */

SplayTree13::Node* SplayTree13::join(Node* left, Node* right)
{
    if (!left) return right;
    if (!right) return left;
    // Find max of left subtree, splay it to root of left
    Node* mx = subtreeMax(left);
    splay(mx);
    mx->right = right;
    if (right) right->parent = mx;
    return mx;
}

/* ---- Split at key ---- */

void SplayTree13::split(double key, Node*& left, Node*& right)
{
    if (!m_root) { left = right = nullptr; return; }

    // Find closest node to key and splay it
    Node* x = m_root;
    Node* last = x;
    while (x) {
        last = x;
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else break;
    }
    splay(last);
    m_root = last;

    if (last->key <= key) {
        left = last;
        right = last->right;
        if (right) right->parent = nullptr;
        last->right = nullptr;
    } else {
        right = last;
        left = last->left;
        if (left) left->parent = nullptr;
        last->left = nullptr;
    }
}

/* ---- Insert ---- */

void SplayTree13::insert(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* newNode = new Node{key, nullptr, nullptr, nullptr};

    if (!m_root) {
        m_root = newNode;
        m_finger = newNode;
    } else {
        // Split at key, then join with new node as root
        Node *left = nullptr, *right = nullptr;
        split(key, left, right);

        // Check for duplicate in left max
        if (left && left->key == key) {
            // Duplicate: rejoin and delete new node
            left->right = right;
            if (right) right->parent = left;
            m_root = left;
            delete newNode;
            return;
        }

        newNode->left = left;
        newNode->right = right;
        if (left) left->parent = newNode;
        if (right) right->parent = newNode;
        m_root = newNode;
        m_finger = newNode;  // Update dynamic finger
    }
    m_numNodes++;

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_numNodes;
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("insert"), m_numNodes, m_stats.treeHeight, elapsed);
}

/* ---- Remove ---- */

void SplayTree13::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* target = findNode(key);
    if (!target) return;

    splay(target);
    m_root = target;

    // Detach left and right subtrees
    Node* left = target->left;
    Node* right = target->right;
    if (left) left->parent = nullptr;
    if (right) right->parent = nullptr;

    delete target;
    m_numNodes--;

    m_root = join(left, right);

    // Update finger
    if (m_finger == target) m_finger = m_root;

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_numNodes;
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("remove"), m_numNodes, m_stats.treeHeight, elapsed);
}

/* ---- Contains (with splay) ---- */

bool SplayTree13::contains(double key)
{
    Node* n = findNode(key);
    if (n) {
        splay(n);
        m_finger = n;
        return true;
    }
    return false;
}

/* ---- Finger search: exploit locality from last access ---- */

double SplayTree13::fingerSearch(double key)
{
    QElapsedTimer timer;
    timer.start();

    // If we have a finger (last accessed node), start search from there
    if (m_finger) {
        Node* x = m_finger;
        // Walk up from finger to find ancestor, then down
        // Optimized: if key is near finger, traversal is O(log|key-finger|)
        while (x->parent && ((key < x->key && x->parent->key > key) ||
                              (key > x->key && x->parent->key < key))) {
            x = x->parent;
        }
        // Now search downward from x
        Node* best = x;
        while (x) {
            if (key < x->key) {
                // Check if this is closer
                if (qAbs(x->key - key) < qAbs(best->key - key)) best = x;
                x = x->left;
            } else if (key > x->key) {
                if (qAbs(x->key - key) < qAbs(best->key - key)) best = x;
                x = x->right;
            } else {
                best = x;
                break;
            }
        }
        splay(best);
        m_finger = best;
        return best->key;
    }

    // No finger: standard search + splay
    Node* n = findNode(key);
    if (n) { splay(n); m_finger = n; return n->key; }
    return key;
}

/* ---- Min/Max with splay ---- */

double SplayTree13::minimum()
{
    Node* n = subtreeMin(m_root);
    if (n) { splay(n); m_finger = n; return n->key; }
    return 0.0;
}

double SplayTree13::maximum()
{
    Node* n = subtreeMax(m_root);
    if (n) { splay(n); m_finger = n; return n->key; }
    return 0.0;
}

/* ---- Successor ---- */

double SplayTree13::successor(double key)
{
    Node* x = m_root;
    Node* succ = nullptr;
    while (x) {
        if (key < x->key) { succ = x; x = x->left; }
        else x = x->right;
    }
    if (succ) { splay(succ); m_finger = succ; return succ->key; }
    return key;
}

/* ---- Predecessor ---- */

double SplayTree13::predecessor(double key)
{
    Node* x = m_root;
    Node* pred = nullptr;
    while (x) {
        if (key > x->key) { pred = x; x = x->right; }
        else x = x->left;
    }
    if (pred) { splay(pred); m_finger = pred; return pred->key; }
    return key;
}

/* ---- In-order traversal ---- */

void SplayTree13::inOrderRec(Node* x, QVector<double>& result) const
{
    if (!x) return;
    inOrderRec(x->left, result);
    result.append(x->key);
    inOrderRec(x->right, result);
}

QVector<double> SplayTree13::inOrder() const
{
    QVector<double> result;
    inOrderRec(m_root, result);
    return result;
}

/* ---- Height ---- */

int SplayTree13::heightRec(Node* x) const
{
    if (!x) return 0;
    return 1 + qMax(heightRec(x->left), heightRec(x->right));
}

int SplayTree13::height() const { return heightRec(m_root); }

/* ---- Reset ---- */

void SplayTree13::resetStatistics()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_finger = nullptr;
    m_numNodes = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
