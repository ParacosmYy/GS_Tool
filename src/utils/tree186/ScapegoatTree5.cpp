/**
 * @file ScapegoatTree5.cpp
 * @brief ScapegoatTree5 实现
 *
 * 实现替罪羊树：α权重平衡检测、插入路径重建、懒删除、摊还O(log n)操作。
 */

#include "utils/tree186/ScapegoatTree5.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ScapegoatTree5::ScapegoatTree5(QObject *parent) : QObject(parent) {}
ScapegoatTree5::~ScapegoatTree5() { deleteTree(m_root); }

void ScapegoatTree5::deleteTree(Node* n)
{
    if (!n) return;
    deleteTree(n->left);
    deleteTree(n->right);
    delete n;
}

/* ---- Configuration ---- */

void ScapegoatTree5::setAlpha(double alpha) { m_alpha = qBound(0.5, alpha, 1.0); }

/* ---- Alpha balance check ---- */

bool ScapegoatTree5::isAlphaBalanced(Node* n) const
{
    if (!n) return true;
    int ls = n->left ? n->left->subtreeSize : 0;
    int rs = n->right ? n->right->subtreeSize : 0;
    int total = ls + rs + 1;
    return (ls <= m_alpha * total) && (rs <= m_alpha * total);
}

/* ---- Tree height ---- */

int ScapegoatTree5::treeHeight(Node* n) const
{
    if (!n) return 0;
    return 1 + qMax(treeHeight(n->left), treeHeight(n->right));
}

/* ---- Update sizes ---- */

void ScapegoatTree5::updateSizes(Node* n)
{
    while (n) {
        int ls = n->left ? n->left->subtreeSize : 0;
        int rs = n->right ? n->right->subtreeSize : 0;
        n->subtreeSize = ls + rs + 1;
        // Can't walk up without parent pointer — used during flatten/rebuild
    }
}

/* ---- Flatten subtree ---- */

void ScapegoatTree5::flatten(Node* n, QVector<Node*>& nodes) const
{
    if (!n) return;
    flatten(n->left, nodes);
    if (!n->deleted) nodes.append(n);
    flatten(n->right, nodes);
}

/* ---- Rebuild balanced subtree ---- */

ScapegoatTree5::Node* ScapegoatTree5::rebuildBalanced(QVector<Node*>& nodes,
                                                       int start, int end)
{
    if (start > end) return nullptr;
    int mid = (start + end) / 2;
    Node* n = nodes[mid];
    n->left = rebuildBalanced(nodes, start, mid - 1);
    n->right = rebuildBalanced(nodes, mid + 1, end);
    int ls = n->left ? n->left->subtreeSize : 0;
    int rs = n->right ? n->right->subtreeSize : 0;
    n->subtreeSize = ls + rs + 1;
    return n;
}

/* ---- In-order traversal ---- */

void ScapegoatTree5::inorderHelper(Node* n, QVector<int>& result) const
{
    if (!n) return;
    inorderHelper(n->left, result);
    if (!n->deleted) result.append(n->key);
    inorderHelper(n->right, result);
}

/* ---- Insert ---- */

void ScapegoatTree5::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Track insertion path for scapegoat detection
    QVector<Node*> path;
    Node* cur = m_root;
    Node* parent = nullptr;

    while (cur) {
        parent = cur;
        path.append(cur);
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else {
            // Duplicate: undelete if lazy-deleted
            if (cur->deleted) cur->deleted = false;
            return;
        }
    }

    // Create new node
    Node* newNode = new Node{key, 1, false, nullptr, nullptr};
    if (!m_root) {
        m_root = newNode;
    } else if (key < parent->key) {
        parent->left = newNode;
    } else {
        parent->right = newNode;
    }

    // Update sizes along path
    for (auto* n : path)
        n->subtreeSize++;

    m_maxSize = qMax(m_maxSize, (m_root ? m_root->subtreeSize : 0));

    // Check if tree is too tall: h > log_{1/alpha}(n)
    int h = path.size() + 1;
    int n = m_root ? m_root->subtreeSize : 0;
    double logBound = qLn(n + 1) / qLn(1.0 / m_alpha);

    if (h > logBound) {
        // Find scapegoat: first unbalanced node on path from leaf to root
        Node* scapegoat = nullptr;
        int sgIdx = -1;
        for (int i = path.size() - 1; i >= 0; --i) {
            if (!isAlphaBalanced(path[i])) {
                scapegoat = path[i];
                sgIdx = i;
                break;
            }
        }

        if (scapegoat) {
            // Rebuild scapegoat subtree
            QVector<Node*> nodes;
            flatten(scapegoat, nodes);

            Node* rebuilt = rebuildBalanced(nodes, 0, nodes.size() - 1);

            // Reattach rebuilt subtree
            if (sgIdx == 0) {
                m_root = rebuilt;
            } else {
                Node* p = (sgIdx > 0) ? path[sgIdx - 1] : nullptr;
                if (p && p->left == scapegoat) p->left = rebuilt;
                else if (p) p->right = rebuilt;
            }

            m_stats.numRebuilds++;
        }
    }

    m_stats.totalOperations++;
    m_stats.numNodes = 0;
    inorderHelper(m_root, *(new QVector<int>)); // count
    m_stats.numNodes = m_root ? m_root->subtreeSize : 0;
    m_stats.treeHeight = treeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("insert", key);
}

/* ---- Remove (lazy deletion) ---- */

void ScapegoatTree5::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else {
            cur->deleted = true;
            break;
        }
    }

    // Trigger rebuild if too many deleted nodes
    int totalSize = m_root ? m_root->subtreeSize : 0;
    int threshold = static_cast<int>(m_maxSize * m_alpha * m_alpha);
    if (totalSize < threshold && m_root) {
        QVector<Node*> nodes;
        flatten(m_root, nodes);
        m_root = rebuildBalanced(nodes, 0, nodes.size() - 1);
        m_maxSize = m_root ? m_root->subtreeSize : 0;
        m_stats.numRebuilds++;
    }

    m_stats.totalOperations++;
    m_stats.numNodes = m_root ? m_root->subtreeSize : 0;
    m_stats.treeHeight = treeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("remove", key);
}

/* ---- Contains ---- */

bool ScapegoatTree5::contains(int key) const
{
    Node* n = m_root;
    while (n) {
        if (key < n->key) n = n->left;
        else if (key > n->key) n = n->right;
        else return !n->deleted;
    }
    return false;
}

/* ---- Utilities ---- */

QVector<int> ScapegoatTree5::inorder() const
{
    QVector<int> result;
    inorderHelper(m_root, result);
    return result;
}

int ScapegoatTree5::size() const { return m_root ? m_root->subtreeSize : 0; }

void ScapegoatTree5::clear()
{
    deleteTree(m_root);
    m_root = nullptr;
    m_maxSize = 0;
}

/* ---- Reset ---- */

void ScapegoatTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
