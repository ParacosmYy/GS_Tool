/**
 * @file ScapegoatTree8.cpp
 * @brief ScapegoatTree8 实现
 *
 * 实现替罪羊树：权重平衡重建触发与自顶向下搜索路径重平衡。
 */

#include "utils/tree236/ScapegoatTree8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ScapegoatTree8::ScapegoatTree8(QObject *parent) : QObject(parent) {}
ScapegoatTree8::~ScapegoatTree8() { deleteTree(m_root); }

/* ---- Configuration ---- */

void ScapegoatTree8::setAlpha(double alpha) { m_alpha = qBound(0.51, alpha, 0.99); }

/* ---- Helper: h_alpha(n) = floor(log_{1/alpha}(n)) ---- */

int ScapegoatTree8::hAlpha(int n) const
{
    if (n <= 1) return 0;
    double logBase = qLn(1.0 / m_alpha);
    if (logBase < 1e-10) return 30;
    return static_cast<int>(qLn(n) / logBase);
}

/* ---- Node size ---- */

int ScapegoatTree8::nodeSize(Node* node) const
{
    return node ? node->size : 0;
}

/* ---- Update size ---- */

void ScapegoatTree8::updateSize(Node* node)
{
    if (node) node->size = 1 + nodeSize(node->left) + nodeSize(node->right);
}

/* ---- Check unbalanced ---- */

bool ScapegoatTree8::isUnbalanced(Node* node, int depth) const
{
    int n = nodeSize(node);
    return depth > hAlpha(n);
}

/* ---- Flatten subtree into sorted array ---- */

void ScapegoatTree8::flatten(Node* node, QVector<Node*>& nodes) const
{
    if (!node) return;
    flatten(node->left, nodes);
    nodes.append(node);
    flatten(node->right, nodes);
}

/* ---- Rebuild balanced subtree from sorted array ---- */

ScapegoatTree8::Node* ScapegoatTree8::rebuildBalanced(const QVector<Node*>& nodes, int lo, int hi)
{
    if (lo > hi) return nullptr;
    int mid = (lo + hi) / 2;
    Node* node = nodes[mid];
    node->left = rebuildBalanced(nodes, lo, mid - 1);
    node->right = rebuildBalanced(nodes, mid + 1, hi);
    updateSize(node);
    return node;
}

/* ---- Find scapegoat on search path ---- */

ScapegoatTree8::Node** ScapegoatTree8::findScapegoat(Node** rootPtr, int key)
{
    // Walk down the search path, find the highest unbalanced node
    // A node is a scapegoat if one child has > alpha * node.size elements
    Node** ptr = rootPtr;
    Node** scapegoat = nullptr;

    while (*ptr) {
        Node* node = *ptr;
        int sz = node->size;
        int leftSz = nodeSize(node->left);
        int rightSz = nodeSize(node->right);

        // Check weight balance condition
        if (leftSz > m_alpha * sz || rightSz > m_alpha * sz) {
            scapegoat = ptr;
        }

        if (key < node->key) ptr = &node->left;
        else if (key > node->key) ptr = &node->right;
        else break;
    }
    return scapegoat;
}

/* ---- Insert helper (returns pointer to pointer for rebuild) ---- */

ScapegoatTree8::Node** ScapegoatTree8::insertHelper(Node** ptr, int key, double value, int depth)
{
    if (!*ptr) {
        *ptr = new Node{key, value, 1, nullptr, nullptr};
        return nullptr;  // No rebuild needed at leaf
    }

    Node* node = *ptr;
    if (key == node->key) {
        node->value = value;  // Update
        return nullptr;
    }

    Node** childPtr = (key < node->key) ? &node->left : &node->right;
    Node** rebuildPtr = insertHelper(childPtr, key, value, depth + 1);
    updateSize(node);

    // Check if this node is the scapegoat
    if (!rebuildPtr && isUnbalanced(node, depth)) {
        return ptr;
    }
    return rebuildPtr;
}

/* ---- Insert ---- */

void ScapegoatTree8::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    int oldSize = nodeSize(m_root);

    // Insert and find deepest unbalanced node
    Node** rebuildPtr = insertHelper(&m_root, key, value, 0);

    if (rebuildPtr && *rebuildPtr) {
        // Rebuild the scapegoat subtree
        QVector<Node*> nodes;
        flatten(*rebuildPtr, nodes);
        int sz = nodes.size();
        *rebuildPtr = rebuildBalanced(nodes, 0, sz - 1);

        m_stats.numRebuilds++;
        m_stats.maxNodesBeforeRebuild = qMax(m_stats.maxNodesBeforeRebuild, oldSize);
        emit treeRebuilt(sz, timer.elapsed());
    }

    int newSize = nodeSize(m_root);
    if (newSize > m_maxSize) m_maxSize = newSize;

    m_stats.numKeys = newSize;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit nodeInserted(key);
}

/* ---- Search helper ---- */

ScapegoatTree8::Node* ScapegoatTree8::searchHelper(Node* node, int key) const
{
    while (node) {
        if (key == node->key) return node;
        node = (key < node->key) ? node->left : node->right;
    }
    return nullptr;
}

/* ---- Search ---- */

double ScapegoatTree8::search(int key) const
{
    Node* node = searchHelper(m_root, key);
    return node ? node->value : std::numeric_limits<double>::quiet_NaN();
}

/* ---- Contains ---- */

bool ScapegoatTree8::contains(int key) const
{
    return searchHelper(m_root, key) != nullptr;
}

/* ---- Remove ---- */

void ScapegoatTree8::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Find node and parent
    Node** ptr = &m_root;
    while (*ptr) {
        Node* node = *ptr;
        if (key == node->key) {
            if (!node->left && !node->right) {
                // Leaf
                delete node;
                *ptr = nullptr;
            } else if (!node->left) {
                *ptr = node->right;
                delete node;
            } else if (!node->right) {
                *ptr = node->left;
                delete node;
            } else {
                // Two children: replace with in-order successor
                Node** succ = &node->right;
                while ((*succ)->left) succ = &(*succ)->left;
                Node* s = *succ;
                node->key = s->key;
                node->value = s->value;
                *succ = s->right;
                delete s;
            }
            break;
        }
        ptr = (key < node->key) ? &node->left : &node->right;
    }

    // Update sizes along path
    // Simplified: recompute from root down
    // (In production, track path during search)

    // Check if global rebuild needed (tree too small relative to max)
    if (m_root) {
        updateSize(m_root);
        if (m_root->size < m_alpha * m_maxSize) {
            QVector<Node*> nodes;
            flatten(m_root, nodes);
            m_root = rebuildBalanced(nodes, 0, nodes.size() - 1);
            m_maxSize = nodes.size();
            m_stats.numRebuilds++;
            emit treeRebuilt(nodes.size(), timer.elapsed());
        }
    }

    m_stats.numKeys = nodeSize(m_root);
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit nodeRemoved(key);
}

/* ---- In-order traversal ---- */

void ScapegoatTree8::inOrder(Node* node, QVector<KVPair>& result) const
{
    if (!node) return;
    inOrder(node->left, result);
    result.append({node->key, node->value});
    inOrder(node->right, result);
}

/* ---- All pairs ---- */

QVector<ScapegoatTree8::KVPair> ScapegoatTree8::allPairs() const
{
    QVector<KVPair> result;
    inOrder(m_root, result);
    return result;
}

/* ---- Height ---- */

int ScapegoatTree8::computeHeight(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(computeHeight(node->left), computeHeight(node->right));
}

int ScapegoatTree8::height() const { return computeHeight(m_root); }

/* ---- Delete tree ---- */

void ScapegoatTree8::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

/* ---- Reset ---- */

void ScapegoatTree8::resetStatistics()
{
    deleteTree(m_root);
    m_root = nullptr;
    m_maxSize = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
