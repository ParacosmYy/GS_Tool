/**
 * @file ScapegoatTree10.cpp
 * @brief ScapegoatTree10 实现
 *
 * 实现替罪羊树：权重平衡替罪羊查找子树重建摊还O(log n)重平衡。
 */

#include "utils/tree264/ScapegoatTree10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ScapegoatTree10::ScapegoatTree10(double alpha, QObject *parent)
    : QObject(parent), m_alpha(qBound(0.5, alpha, 1.0)) {}

ScapegoatTree10::~ScapegoatTree10() { clearTree(m_root); }

/* ---- Node size helpers ---- */

void ScapegoatTree10::updateSize(Node* node)
{
    if (node) node->subtreeSize = 1 + nodeSize(node->left) + nodeSize(node->right);
}

bool ScapegoatTree10::isBalanced(Node* node) const
{
    if (!node) return true;
    int ls = nodeSize(node->left), rs = nodeSize(node->right);
    int total = ls + rs + 1;
    return ls <= m_alpha * total && rs <= m_alpha * total;
}

/* ---- Find scapegoat on path ---- */

ScapegoatTree10::Node* ScapegoatTree10::findScapegoat(Node* node, int key)
{
    // Walk down from root tracking path, find deepest unbalanced ancestor
    Node* scapegoat = nullptr;
    Node* cur = node;
    while (cur) {
        if (!isBalanced(cur)) scapegoat = cur;
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else break;
    }
    return scapegoat;
}

/* ---- Flatten subtree ---- */

void ScapegoatTree10::flatten(Node* node, QVector<Node*>& nodes)
{
    if (!node) return;
    flatten(node->left, nodes);
    nodes.append(node);
    flatten(node->right, nodes);
}

/* ---- Rebuild balanced from sorted array ---- */

ScapegoatTree10::Node* ScapegoatTree10::rebuildBalanced(QVector<Node*>& nodes,
                                                          int start, int end)
{
    if (start > end) return nullptr;
    int mid = (start + end) / 2;
    Node* node = nodes[mid];
    node->left = rebuildBalanced(nodes, start, mid - 1);
    node->right = rebuildBalanced(nodes, mid + 1, end);
    updateSize(node);
    return node;
}

/* ---- Find path to key (for rebuild pointer fixup) ---- */

QVector<ScapegoatTree10::PathEntry> ScapegoatTree10::findPath(int key)
{
    QVector<PathEntry> path;
    Node** link = &m_root;
    while (*link) {
        path.append({link, *link});
        if (key < (*link)->key) link = &((*link)->left);
        else if (key > (*link)->key) link = &((*link)->right);
        else break;
    }
    return path;
}

/* ---- Rebuild subtree ---- */

void ScapegoatTree10::rebuildSubtree(Node*& root, Node* scapegoat)
{
    // Find the pointer to scapegoat in the tree
    auto path = findPath(scapegoat->key);
    QVector<Node*> nodes;
    flatten(scapegoat, nodes);

    // Disconnect children before rebuild
    Node* rebuilt = rebuildBalanced(nodes, 0, nodes.size() - 1);

    // Fix pointer to rebuilt subtree
    for (auto& entry : path) {
        if (entry.node == scapegoat) {
            *entry.link = rebuilt;
            break;
        }
    }

    m_stats.numRebuilds++;
}

/* ---- Insert recursively ---- */

QPair<ScapegoatTree10::Node*, int> ScapegoatTree10::insertRec(Node* node, int key,
                                                                double value, int depth)
{
    if (!node) {
        Node* newNode = new Node{key, value, nullptr, nullptr, 1};
        m_size++;
        return {newNode, depth};
    }
    if (key < node->key) {
        auto [child, d] = insertRec(node->left, key, value, depth + 1);
        node->left = child;
    } else if (key > node->key) {
        auto [child, d] = insertRec(node->right, key, value, depth + 1);
        node->right = child;
    } else {
        node->value = value;  // Update existing
        return {node, depth};
    }
    updateSize(node);
    return {node, depth + 1};
}

/* ---- Insert ---- */

void ScapegoatTree10::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    int oldSize = m_size;
    auto [newRoot, depth] = insertRec(m_root, key, value, 0);
    m_root = newRoot;

    if (m_size > oldSize) {
        m_maxSize = qMax(m_maxSize, m_size);

        // Check if tree height exceeds alpha-weight-balanced height bound
        double hAlpha = qLog(m_size) / qLog(1.0 / m_alpha);
        if (depth > hAlpha) {
            Node* goat = findScapegoat(m_root, key);
            if (goat) rebuildSubtree(m_root, goat);
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numKeys = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.alpha = m_alpha;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_size, m_stats.treeHeight, m_stats.numRebuilds, elapsed);
}

/* ---- Remove recursively ---- */

ScapegoatTree10::Node* ScapegoatTree10::removeRec(Node* node, int key)
{
    if (!node) return nullptr;
    if (key < node->key) {
        node->left = removeRec(node->left, key);
    } else if (key > node->key) {
        node->right = removeRec(node->right, key);
    } else {
        if (!node->left || !node->right) {
            Node* child = node->left ? node->left : node->right;
            delete node;
            m_size--;
            return child;
        }
        Node* minRight = findMin(node->right);
        node->key = minRight->key;
        node->value = minRight->value;
        node->right = removeRec(node->right, minRight->key);
    }
    updateSize(node);
    return node;
}

/* ---- Remove ---- */

void ScapegoatTree10::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = removeRec(m_root, key);

    // Global rebuild condition: if too many deletions without rebuild
    if (m_size < m_alpha * m_maxSize && m_size > 0) {
        QVector<Node*> nodes;
        flatten(m_root, nodes);
        m_root = rebuildBalanced(nodes, 0, nodes.size() - 1);
        m_maxSize = m_size;
        m_stats.numRebuilds++;
    }

    double elapsed = timer.elapsed();
    m_stats.numKeys = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.alpha = m_alpha;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_size, m_stats.treeHeight, m_stats.numRebuilds, elapsed);
}

/* ---- Find min ---- */

ScapegoatTree10::Node* ScapegoatTree10::findMin(Node* node) const
{
    while (node && node->left) node = node->left;
    return node;
}

/* ---- Search ---- */

double ScapegoatTree10::search(int key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur->value;
    }
    return qQNaN();
}

bool ScapegoatTree10::contains(int key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return true;
    }
    return false;
}

/* ---- Keys in order ---- */

void ScapegoatTree10::inOrderKeys(Node* node, QVector<int>& result) const
{
    if (!node) return;
    inOrderKeys(node->left, result);
    result.append(node->key);
    inOrderKeys(node->right, result);
}

QVector<int> ScapegoatTree10::keys() const
{
    QVector<int> result;
    inOrderKeys(m_root, result);
    return result;
}

/* ---- Height ---- */

int ScapegoatTree10::computeHeight(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(computeHeight(node->left), computeHeight(node->right));
}

int ScapegoatTree10::height() const { return computeHeight(m_root); }
int ScapegoatTree10::size() const { return m_size; }

/* ---- Clear ---- */

void ScapegoatTree10::clearTree(Node* node)
{
    if (!node) return;
    clearTree(node->left);
    clearTree(node->right);
    delete node;
}

/* ---- Reset ---- */

void ScapegoatTree10::resetStatistics()
{
    clearTree(m_root);
    m_root = nullptr;
    m_size = 0;
    m_maxSize = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
