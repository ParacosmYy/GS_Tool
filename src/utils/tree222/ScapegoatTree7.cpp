/**
 * @file ScapegoatTree7.cpp
 * @brief ScapegoatTree7 实现
 *
 * 实现替罪羊树：加权失衡检测、最优重建触发、批量插入支持。
 */

#include "utils/tree222/ScapegoatTree7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ScapegoatTree7::ScapegoatTree7(QObject *parent) : QObject(parent) {}

ScapegoatTree7::~ScapegoatTree7() { clearNode(m_root); }

/* ---- Configuration ---- */

void ScapegoatTree7::setAlpha(double alpha)
{
    m_alpha = qBound(0.51, alpha, 0.99);
    m_stats.alpha = m_alpha;
}

/* ---- Weight helpers ---- */

int ScapegoatTree7::weight(Node* node) const
{
    return node ? node->weight : 0;
}

void ScapegoatTree7::updateWeight(Node* node)
{
    if (!node) return;
    node->weight = 1 + weight(node->left) + weight(node->right);
}

/* ---- Alpha balance check ---- */

bool ScapegoatTree7::isAlphaBalanced(Node* node) const
{
    if (!node) return true;
    int w = node->weight;
    return weight(node->left) <= m_alpha * w
        && weight(node->right) <= m_alpha * w;
}

/* ---- Find scapegoat on insertion path ---- */

ScapegoatTree7::Node* ScapegoatTree7::findScapegoat(Node* path[], int pathLen)
{
    // Walk from leaf to root, find first unbalanced node
    for (int i = pathLen - 1; i >= 0; --i) {
        if (!isAlphaBalanced(path[i])) return path[i];
    }
    return nullptr;
}

/* ---- Flatten subtree ---- */

void ScapegoatTree7::flatten(Node* node, QVector<int>& keys) const
{
    if (!node) return;
    flatten(node->left, keys);
    keys.append(node->key);
    flatten(node->right, keys);
}

/* ---- Rebuild balanced BST from sorted array ---- */

ScapegoatTree7::Node* ScapegoatTree7::rebuild(const QVector<int>& keys,
                                                 int lo, int hi)
{
    if (lo > hi) return nullptr;
    int mid = (lo + hi) / 2;
    Node* node = new Node(keys[mid]);
    node->left = rebuild(keys, lo, mid - 1);
    node->right = rebuild(keys, mid + 1, hi);
    updateWeight(node);
    return node;
}

/* ---- Rebuild at scapegoat ---- */

void ScapegoatTree7::rebuildAt(Node*& root)
{
    QVector<int> keys;
    flatten(root, keys);
    clearNode(root);
    root = rebuild(keys, 0, keys.size() - 1);
    m_stats.numRebuilds++;
}

/* ---- Insert ---- */

void ScapegoatTree7::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Track insertion path for scapegoat detection
    QVector<Node*> path;
    path.reserve(64);

    if (!m_root) {
        m_root = new Node(key);
        m_maxSize = 1;
    } else {
        Node* cur = m_root;
        while (cur) {
            path.append(cur);
            if (key < cur->key) {
                if (!cur->left) { cur->left = new Node(key); break; }
                cur = cur->left;
            } else if (key > cur->key) {
                if (!cur->right) { cur->right = new Node(key); break; }
                cur = cur->right;
            } else {
                // Duplicate, ignore
                return;
            }
        }

        // Update weights along path
        for (auto* n : path) updateWeight(n);

        m_stats.nodeCount++;
        m_maxSize = qMax(m_maxSize, m_stats.nodeCount);

        // Check if root height exceeds log_(1/alpha)(maxSize)
        int h = height(m_root);
        int maxH = (m_maxSize > 0)
            ? qCeil(qLn(m_maxSize) / qLn(1.0 / m_alpha)) : 0;

        if (h > maxH) {
            // Find scapegoat
            Node** scapegoatPtr = &m_root;
            // Find scapegoat in path
            Node* scapegoat = nullptr;
            for (int i = 0; i < path.size(); ++i) {
                if (!isAlphaBalanced(path[i])) {
                    scapegoat = path[i];
                    m_stats.maxRebuildDepth = qMax(m_stats.maxRebuildDepth, i);
                    break;
                }
            }

            if (scapegoat) {
                // Find pointer to scapegoat
                if (scapegoat == m_root) {
                    rebuildAt(m_root);
                } else {
                    // Find parent of scapegoat
                    for (int i = 0; i < path.size() - 1; ++i) {
                        if (path[i]->left == scapegoat) {
                            rebuildAt(path[i]->left);
                            break;
                        }
                        if (path[i]->right == scapegoat) {
                            rebuildAt(path[i]->right);
                            break;
                        }
                    }
                }
                // Update weights up the tree
                updateWeight(m_root);
                emit rebuildTriggered(scapegoat ? scapegoat->weight : 0,
                                       path.size(), timer.elapsed());
            }
        }
    }

    m_stats.treeHeight = height(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("insert", key, timer.elapsed());
}

/* ---- Bulk insert ---- */

void ScapegoatTree7::bulkInsert(const QVector<int>& keys)
{
    QElapsedTimer timer;
    timer.start();

    // Collect existing keys
    QVector<int> allKeys;
    flatten(m_root, allKeys);

    // Merge with new keys
    allKeys.append(keys);
    std::sort(allKeys.begin(), allKeys.end());
    // Remove duplicates
    int j = 0;
    for (int i = 0; i < allKeys.size(); ++i) {
        if (j == 0 || allKeys[i] != allKeys[j - 1])
            allKeys[j++] = allKeys[i];
    }
    allKeys.resize(j);

    // Rebuild entire tree
    clearNode(m_root);
    m_root = rebuild(allKeys, 0, allKeys.size() - 1);

    m_stats.nodeCount = allKeys.size();
    m_maxSize = qMax(m_maxSize, m_stats.nodeCount);
    m_stats.numRebuilds++;
    m_stats.treeHeight = height(m_root);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit rebuildTriggered(allKeys.size(), 0, timer.elapsed());
}

/* ---- Remove ---- */

bool ScapegoatTree7::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Find node and parent
    Node* parent = nullptr;
    Node* cur = m_root;
    bool isLeft = false;

    while (cur && cur->key != key) {
        parent = cur;
        if (key < cur->key) { cur = cur->left; isLeft = true; }
        else { cur = cur->right; isLeft = false; }
    }

    if (!cur) return false;

    // Three cases: no child, one child, two children
    Node* replacement = nullptr;
    if (!cur->left && !cur->right) {
        replacement = nullptr;
    } else if (!cur->left) {
        replacement = cur->right;
    } else if (!cur->right) {
        replacement = cur->left;
    } else {
        // In-order successor
        Node* succParent = cur;
        Node* succ = cur->right;
        while (succ->left) { succParent = succ; succ = succ->left; }
        cur->key = succ->key;
        // Remove successor
        if (succParent == cur) succParent->right = succ->right;
        else succParent->left = succ->right;
        delete succ;
        updateWeight(m_root);
        m_stats.nodeCount--;
        m_stats.treeHeight = height(m_root);

        // Check if rebuild needed: nodeCount < alpha * maxSize
        if (m_stats.nodeCount < m_alpha * m_maxSize) {
            rebuildAt(m_root);
            m_maxSize = m_stats.nodeCount;
        }

        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit operationCompleted("remove", key, timer.elapsed());
        return true;
    }

    if (!parent) m_root = replacement;
    else if (isLeft) parent->left = replacement;
    else parent->right = replacement;

    delete cur;
    updateWeight(m_root);
    m_stats.nodeCount--;
    m_stats.treeHeight = height(m_root);

    if (m_stats.nodeCount < m_alpha * m_maxSize && m_root) {
        rebuildAt(m_root);
        m_maxSize = m_stats.nodeCount;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("remove", key, timer.elapsed());
    return true;
}

/* ---- Search ---- */

bool ScapegoatTree7::searchHelper(Node* node, int key) const
{
    while (node) {
        if (key == node->key) return true;
        node = (key < node->key) ? node->left : node->right;
    }
    return false;
}

bool ScapegoatTree7::search(int key) const
{
    return searchHelper(m_root, key);
}

/* ---- In-order ---- */

void ScapegoatTree7::inOrderHelper(Node* node, QVector<int>& result) const
{
    if (!node) return;
    inOrderHelper(node->left, result);
    result.append(node->key);
    inOrderHelper(node->right, result);
}

QVector<int> ScapegoatTree7::inOrder() const
{
    QVector<int> result;
    inOrderHelper(m_root, result);
    return result;
}

/* ---- Balance check ---- */

bool ScapegoatTree7::isBalancedHelper(Node* node) const
{
    if (!node) return true;
    return isAlphaBalanced(node)
        && isBalancedHelper(node->left)
        && isBalancedHelper(node->right);
}

bool ScapegoatTree7::isBalanced() const
{
    return isBalancedHelper(m_root);
}

/* ---- Height ---- */

int ScapegoatTree7::height(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(height(node->left), height(node->right));
}

/* ---- Cleanup ---- */

void ScapegoatTree7::clearNode(Node* node)
{
    if (!node) return;
    clearNode(node->left);
    clearNode(node->right);
    delete node;
}

/* ---- Reset ---- */

void ScapegoatTree7::resetStatistics()
{
    m_stats = Stats{};
    m_stats.alpha = m_alpha;
    m_timeSum = 0.0;
    clearNode(m_root);
    m_root = nullptr;
    m_maxSize = 0;
}
