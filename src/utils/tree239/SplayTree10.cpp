/**
 * @file SplayTree10.cpp
 * @brief SplayTree10 实现
 *
 * 实现伸展树：自顶向下zig-zig/zig-zag重构与工作集定理摊还O(log n)。
 */

#include "utils/tree239/SplayTree10.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplayTree10::SplayTree10(QObject *parent) : QObject(parent) {}
SplayTree10::~SplayTree10() { destroyTree(m_root); }

/* ---- Rotation helpers ---- */

SplayTree10::Node* SplayTree10::rotateRight(Node* x)
{
    Node* y = x->left;
    x->left = y->right;
    y->right = x;
    return y;
}

SplayTree10::Node* SplayTree10::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    return y;
}

/* ---- Top-down splay ---- */

SplayTree10::Node* SplayTree10::splay(Node* root, double key)
{
    if (!root) return nullptr;

    // Auxiliary trees for top-down splay
    Node stub;
    Node* leftTree = &stub;
    Node* rightTree = &stub;
    Node* leftTail = leftTree;
    Node* rightTail = rightTree;

    while (root) {
        if (key < root->key) {
            if (!root->left) break;

            // Zig-zig: key is in left-left subtree
            if (key < root->left->key) {
                root = rotateRight(root);
                m_stats.numZigZig++;
                if (!root->left) break;
            }

            // Link root to right tree
            rightTail->left = root;
            rightTail = root;
            root = root->left;
            rightTail->left = nullptr;

        } else if (key > root->key) {
            if (!root->right) break;

            // Zig-zag (or zig-zig in right direction)
            if (key > root->right->key) {
                root = rotateLeft(root);
                m_stats.numZigZag++;
                if (!root->right) break;
            }

            // Link root to left tree
            leftTail->right = root;
            leftTail = root;
            root = root->right;
            leftTail->right = nullptr;

        } else {
            // Exact match found
            break;
        }
    }

    // Reassemble: attach left tree and right tree
    leftTail->right = root->left;
    rightTail->left = root->right;
    root->left = stub.right;
    root->right = stub.left;

    m_stats.numSplayOps++;
    m_stats.numZig++;  // count as zig operation
    return root;
}

/* ---- Insert ---- */

void SplayTree10::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new Node{key, value, nullptr, nullptr};
        m_stats.numNodes++;
        m_stats.treeHeight = 1;
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit nodeInserted(key);
        return;
    }

    m_root = splay(m_root, key);

    if (key == m_root->key) {
        // Update existing
        m_root->value = value;
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit splayCompleted(key, 0, timer.elapsed());
        return;
    }

    Node* newNode = new Node{key, value, nullptr, nullptr};

    if (key < m_root->key) {
        newNode->right = m_root;
        newNode->left = m_root->left;
        m_root->left = nullptr;
    } else {
        newNode->left = m_root;
        newNode->right = m_root->right;
        m_root->right = nullptr;
    }

    m_root = newNode;
    m_stats.numNodes++;
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit nodeInserted(key);
}

/* ---- Remove ---- */

bool SplayTree10::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return false;

    m_root = splay(m_root, key);

    if (m_root->key != key) {
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit nodeRemoved(key, false);
        return false;
    }

    Node* toDelete = m_root;
    if (!m_root->left) {
        m_root = m_root->right;
    } else {
        Node* newRoot = m_root->left;
        newRoot = splay(newRoot, key);
        newRoot->right = m_root->right;
        m_root = newRoot;
    }

    delete toDelete;
    m_stats.numNodes--;
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit nodeRemoved(key, true);
    return true;
}

/* ---- Find ---- */

bool SplayTree10::find(double key, double& value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return false;

    m_root = splay(m_root, key);

    bool found = (m_root->key == key);
    if (found) value = m_root->value;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit splayCompleted(key, 0, timer.elapsed());
    return found;
}

/* ---- Minimum ---- */

SplayTree10::Entry SplayTree10::minimum()
{
    if (!m_root) return {};
    Node* n = m_root;
    while (n->left) n = n->left;
    m_root = splay(m_root, n->key);
    return {m_root->key, m_root->value};
}

/* ---- Maximum ---- */

SplayTree10::Entry SplayTree10::maximum()
{
    if (!m_root) return {};
    Node* n = m_root;
    while (n->right) n = n->right;
    m_root = splay(m_root, n->key);
    return {m_root->key, m_root->value};
}

/* ---- Split ---- */

void SplayTree10::split(double key, SplayTree10& leftTree, SplayTree10& rightTree)
{
    if (!m_root) {
        leftTree.m_root = nullptr;
        rightTree.m_root = nullptr;
        return;
    }

    m_root = splay(m_root, key);
    leftTree.destroyTree(leftTree.m_root);
    rightTree.destroyTree(rightTree.m_root);

    if (m_root->key <= key) {
        leftTree.m_root = cloneTree(m_root);
        rightTree.m_root = cloneTree(m_root->right);
    } else {
        leftTree.m_root = cloneTree(m_root->left);
        rightTree.m_root = cloneTree(m_root);
    }
}

/* ---- Merge ---- */

void SplayTree10::merge(SplayTree10& other)
{
    if (!other.m_root) return;
    if (!m_root) {
        m_root = other.m_root;
        other.m_root = nullptr;
        return;
    }

    // Get maximum of this tree, splay to root, attach other
    Entry maxE = maximum();
    m_root->right = other.m_root;
    other.m_root = nullptr;
    m_stats.numNodes += other.m_stats.numNodes;
}

/* ---- In-order traversal ---- */

void SplayTree10::inOrderRec(Node* n, QVector<Entry>& result) const
{
    if (!n) return;
    inOrderRec(n->left, result);
    result.append({n->key, n->value});
    inOrderRec(n->right, result);
}

QVector<SplayTree10::Entry> SplayTree10::inOrder() const
{
    QVector<Entry> result;
    inOrderRec(m_root, result);
    return result;
}

/* ---- Height ---- */

int SplayTree10::heightRec(Node* n) const
{
    if (!n) return 0;
    return 1 + qMax(heightRec(n->left), heightRec(n->right));
}

int SplayTree10::height() const { return heightRec(m_root); }
bool SplayTree10::isEmpty() const { return m_root == nullptr; }

/* ---- Destroy ---- */

void SplayTree10::destroyTree(Node* n)
{
    if (!n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    delete n;
}

/* ---- Clone ---- */

SplayTree10::Node* SplayTree10::cloneTree(Node* n) const
{
    if (!n) return nullptr;
    Node* c = new Node{n->key, n->value, nullptr, nullptr};
    c->left = cloneTree(n->left);
    c->right = cloneTree(n->right);
    return c;
}

/* ---- Reset ---- */

void SplayTree10::resetStatistics()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_stats = Stats{}; m_timeSum = 0.0;
}
