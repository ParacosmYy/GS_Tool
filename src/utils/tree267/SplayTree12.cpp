/**
 * @file SplayTree12.cpp
 * @brief SplayTree12 实现
 *
 * 实现伸展树：自顶向下zig-zig/zig-zag重构与半伸展摊还再平衡自调整BST。
 */

#include "utils/tree267/SplayTree12.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>
#include <climits>

/* ---- Construction / Destruction ---- */

SplayTree12::SplayTree12(QObject *parent)
    : QObject(parent) {}

SplayTree12::~SplayTree12()
{
    destroyTree(m_root);
}

void SplayTree12::destroyTree(Node* node)
{
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

/* ---- Rotations ---- */

SplayTree12::Node* SplayTree12::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    m_stats.numRotations++;
    return x;
}

SplayTree12::Node* SplayTree12::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    m_stats.numRotations++;
    return y;
}

/* ---- Top-down splay ---- */

SplayTree12::Node* SplayTree12::splay(Node* root, int key)
{
    if (!root || root->key == key) return root;

    // Create dummy nodes for left and right subtrees
    Node header;
    header.left = header.right = nullptr;
    Node* leftTree = &header;
    Node* rightTree = &header;

    while (root && root->key != key) {
        if (key < root->key) {
            if (!root->left) break;

            // Zig-zig case: key < root->left->key
            if (key < root->left->key) {
                root = rotateRight(root);
                if (!root->left) break;
            }
            // Link to right tree
            rightTree->left = root;
            rightTree = root;
            root = root->left;
            rightTree->left = nullptr;
        } else {
            if (!root->right) break;

            // Zig-zag case: key > root->right->key
            if (key > root->right->key) {
                root = rotateLeft(root);
                if (!root->right) break;
            }
            // Link to left tree
            leftTree->right = root;
            leftTree = root;
            root = root->right;
            leftTree->right = nullptr;
        }
    }

    // Reassemble
    leftTree->right = root ? root->left : nullptr;
    rightTree->left = root ? root->right : nullptr;

    if (root) {
        root->left = header.right;
        root->right = header.left;
    }

    m_stats.numSplays++;
    return root;
}

/* ---- Semi-splay ---- */

SplayTree12::Node* SplayTree12::semiSplay(Node* root)
{
    // Semi-splay: only rotate once per level instead of double rotation
    // This provides lighter rebalancing for amortized O(log n)
    if (!root) return root;

    int leftH = computeHeight(root->left);
    int rightH = computeHeight(root->right);

    // If significantly unbalanced, perform single rotation
    if (leftH > rightH + 1 && root->left) {
        root = rotateRight(root);
    } else if (rightH > leftH + 1 && root->right) {
        root = rotateLeft(root);
    }

    return root;
}

/* ---- Insert ---- */

void SplayTree12::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new Node{key, nullptr, nullptr};
        m_size++;
    } else {
        m_root = splay(m_root, key);

        if (m_root->key == key) {
            // Duplicate: semi-splay only
            m_root = semiSplay(m_root);
        } else if (key < m_root->key) {
            Node* newNode = new Node{key, m_root->left, m_root};
            m_root->left = nullptr;
            m_root = newNode;
            m_size++;
        } else {
            Node* newNode = new Node{key, m_root, m_root->right};
            m_root->right = nullptr;
            m_root = newNode;
            m_size++;
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numElements = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_size, m_stats.treeHeight, m_stats.numSplays, elapsed);
}

/* ---- Remove ---- */

void SplayTree12::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return;

    m_root = splay(m_root, key);

    if (m_root->key != key) {
        // Key not found
        return;
    }

    Node* toDelete = m_root;
    if (!m_root->left) {
        m_root = m_root->right;
    } else if (!m_root->right) {
        m_root = m_root->left;
    } else {
        // Splay maximum of left subtree to root
        Node* leftSub = m_root->left;
        Node* rightSub = m_root->right;
        leftSub = splay(leftSub, INT_MAX);

        // leftSub now has no right child
        leftSub->right = rightSub;
        m_root = leftSub;
    }

    delete toDelete;
    m_size--;

    double elapsed = timer.elapsed();
    m_stats.numElements = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_size, m_stats.treeHeight, m_stats.numSplays, elapsed);
}

/* ---- Search (with splay) ---- */

bool SplayTree12::contains(int key)
{
    if (!m_root) return false;
    m_root = splay(m_root, key);
    return m_root && m_root->key == key;
}

/* ---- Min / Max ---- */

int SplayTree12::minimum()
{
    if (!m_root) return INT_MIN;
    Node* curr = m_root;
    while (curr->left) curr = curr->left;
    m_root = splay(m_root, curr->key);
    return curr->key;
}

int SplayTree12::maximum()
{
    if (!m_root) return INT_MAX;
    Node* curr = m_root;
    while (curr->right) curr = curr->right;
    m_root = splay(m_root, curr->key);
    return curr->key;
}

/* ---- In-order traversal ---- */

void SplayTree12::inOrderCollect(Node* node, QVector<int>& keys) const
{
    if (!node) return;
    inOrderCollect(node->left, keys);
    keys.append(node->key);
    inOrderCollect(node->right, keys);
}

QVector<int> SplayTree12::inOrderKeys() const
{
    QVector<int> keys;
    inOrderCollect(m_root, keys);
    return keys;
}

/* ---- Height ---- */

int SplayTree12::computeHeight(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(computeHeight(node->left), computeHeight(node->right));
}

int SplayTree12::size() const { return m_size; }

/* ---- Reset ---- */

void SplayTree12::resetStatistics()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
