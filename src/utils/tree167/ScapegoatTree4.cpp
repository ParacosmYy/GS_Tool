/**
 * @file ScapegoatTree4.cpp
 * @brief ScapegoatTree4 实现
 *
 * 实现替罪羊树：插入时检测不平衡并重建子树，删除后延迟重建。
 */

#include "utils/tree167/ScapegoatTree4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

ScapegoatTree4::ScapegoatTree4(QObject* parent)
    : QObject(parent)
{
}

ScapegoatTree4::~ScapegoatTree4()
{
    clearRec(m_root);
}

void ScapegoatTree4::setAlpha(double alpha)
{
    m_alpha = qBound(0.5, alpha, 1.0);
}

int ScapegoatTree4::treeSize(Node* node)
{
    return node ? node->size : 0;
}

void ScapegoatTree4::updateSize(Node* node)
{
    if (node) node->size = 1 + treeSize(node->left) + treeSize(node->right);
}

int ScapegoatTree4::height(Node* node)
{
    if (!node) return 0;
    return 1 + qMax(height(node->left), height(node->right));
}

bool ScapegoatTree4::containsRec(Node* node, double key) const
{
    if (!node) return false;
    if (key < node->key) return containsRec(node->left, key);
    if (key > node->key) return containsRec(node->right, key);
    return true;
}

void ScapegoatTree4::flatten(Node* node, QVector<Node*>& list) const
{
    if (!node) return;
    flatten(node->left, list);
    list.append(node);
    flatten(node->right, list);
}

ScapegoatTree4::Node* ScapegoatTree4::buildBalanced(const QVector<Node*>& list, int lo, int hi)
{
    if (lo > hi) return nullptr;
    int mid = (lo + hi) / 2;
    Node* node = list[mid];
    node->left = buildBalanced(list, lo, mid - 1);
    node->right = buildBalanced(list, mid + 1, hi);
    updateSize(node);
    return node;
}

ScapegoatTree4::Node* ScapegoatTree4::rebuildSubtree(Node* node)
{
    if (!node) return nullptr;

    QVector<Node*> nodes;
    flatten(node, nodes);

    /* Disconnect children before rebuilding */
    int count = nodes.size();
    m_stats.totalRebuilds++;

    emit rebuildTriggered(count);

    return buildBalanced(nodes, 0, count - 1);
}

ScapegoatTree4::Node* ScapegoatTree4::insertRec(Node* node, double key,
                                                  int depth, bool& rebuilt)
{
    if (!node) {
        m_stats.treeHeight = qMax(m_stats.treeHeight, depth);
        return new Node(key);
    }

    if (key < node->key) {
        node->left = insertRec(node->left, key, depth + 1, rebuilt);
    } else if (key > node->key) {
        node->right = insertRec(node->right, key, depth + 1, rebuilt);
    } else {
        /* Duplicate key: ignore */
        return node;
    }

    updateSize(node);

    /* Check if this node is unbalanced */
    if (!rebuilt) {
        int ls = treeSize(node->left);
        int rs = treeSize(node->right);
        int s = node->size;

        if (ls > m_alpha * s || rs > m_alpha * s) {
            rebuilt = true;
            return rebuildSubtree(node);
        }
    }

    return node;
}

bool ScapegoatTree4::insert(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (contains(key)) return false;

    bool rebuilt = false;
    m_stats.treeHeight = 0;
    m_root = insertRec(m_root, key, 0, rebuilt);
    m_stats.currentSize++;
    m_maxSize = qMax(m_maxSize, m_stats.currentSize);

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeletes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit insertCompleted(key, m_stats.currentSize);
    return true;
}

ScapegoatTree4::Node* ScapegoatTree4::removeRec(Node* node, double key, bool& found)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeRec(node->left, key, found);
    } else if (key > node->key) {
        node->right = removeRec(node->right, key, found);
    } else {
        found = true;
        if (!node->left && !node->right) {
            delete node;
            return nullptr;
        }
        if (!node->left) {
            Node* r = node->right;
            node->left = node->right = nullptr;
            delete node;
            return r;
        }
        if (!node->right) {
            Node* l = node->left;
            node->left = node->right = nullptr;
            delete node;
            return l;
        }
        /* Two children: find in-order successor */
        Node* succ = node->right;
        while (succ->left) succ = succ->left;
        node->key = succ->key;
        node->right = removeRec(node->right, succ->key, found);
    }

    updateSize(node);
    return node;
}

bool ScapegoatTree4::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    bool found = false;
    m_root = removeRec(m_root, key, found);

    if (!found) return false;

    m_stats.currentSize--;
    m_stats.totalDeletes++;

    /* Delete-triggered rebuild: if tree is too sparse */
    if (m_stats.currentSize < m_alpha * m_maxSize) {
        if (m_root) {
            m_root = rebuildSubtree(m_root);
        }
        m_maxSize = m_stats.currentSize;
    }

    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeletes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit deleteCompleted(key);
    return true;
}

bool ScapegoatTree4::contains(double key) const
{
    return containsRec(m_root, key);
}

int ScapegoatTree4::rankRec(Node* node, double key) const
{
    if (!node) return 0;
    if (key <= node->key) return rankRec(node->left, key);
    return 1 + treeSize(node->left) + rankRec(node->right, key);
}

int ScapegoatTree4::rank(double key) const
{
    return rankRec(m_root, key);
}

double ScapegoatTree4::kthRec(Node* node, int k) const
{
    if (!node) return 0.0;
    int ls = treeSize(node->left);
    if (k <= ls) return kthRec(node->left, k);
    if (k == ls + 1) return node->key;
    return kthRec(node->right, k - ls - 1);
}

double ScapegoatTree4::kth(int k) const
{
    if (k < 1 || k > m_stats.currentSize) return 0.0;
    return kthRec(m_root, k);
}

bool ScapegoatTree4::isEmpty() const
{
    return m_root == nullptr;
}

void ScapegoatTree4::clearRec(Node* node)
{
    if (!node) return;
    clearRec(node->left);
    clearRec(node->right);
    delete node;
}

void ScapegoatTree4::clear()
{
    clearRec(m_root);
    m_root = nullptr;
    m_maxSize = 0;
    m_stats.currentSize = 0;
    m_stats.treeHeight = 0;
}

void ScapegoatTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
