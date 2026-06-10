/**
 * @file WeightBalancedTree11.cpp
 * @brief WeightBalancedTree11 实现
 *
 * 实现权重平衡树：三路重平衡与平衡因子传播的BB[alpha]树维护。
 */

#include "utils/tree284/WeightBalancedTree11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WeightBalancedTree11::WeightBalancedTree11(QObject *parent)
    : QObject(parent) {}

WeightBalancedTree11::~WeightBalancedTree11() = default;

/* ---- Configuration ---- */

void WeightBalancedTree11::setAlpha(double alpha)
{
    m_alpha = qBound(0.2, alpha, 0.4);  // Valid range for BB[alpha]
}

/* ---- Node allocation ---- */

int WeightBalancedTree11::allocNode(int key, double value)
{
    int idx;
    if (!m_freeList.isEmpty()) {
        idx = m_freeList.takeLast();
        m_nodes[idx] = WBNode{key, value, -1, -1, -1, 1, 0.5};
    } else {
        idx = m_nodes.size();
        m_nodes.append(WBNode{key, value, -1, -1, -1, 1, 0.5});
    }
    return idx;
}

void WeightBalancedTree11::freeNode(int idx)
{
    m_freeList.append(idx);
}

/* ---- Update weight and balance factor ---- */

void WeightBalancedTree11::updateWeight(int idx)
{
    if (idx < 0) return;
    WBNode& n = m_nodes[idx];
    int lw = (n.left >= 0) ? m_nodes[n.left].weight : 0;
    int rw = (n.right >= 0) ? m_nodes[n.right].weight : 0;
    n.weight = lw + rw + 1;
    n.balance = (n.weight > 0) ? static_cast<double>(lw) / n.weight : 0.5;
}

/* ---- Check BB[alpha] balance violation ---- */

bool WeightBalancedTree11::isUnbalanced(int idx) const
{
    if (idx < 0) return false;
    const WBNode& n = m_nodes[idx];
    return n.balance < m_alpha || n.balance > (1.0 - m_alpha);
}

/* ---- Rotate left ---- */

int WeightBalancedTree11::rotateLeft(int idx)
{
    WBNode& n = m_nodes[idx];
    int r = n.right;
    if (r < 0) return idx;

    WBNode& nr = m_nodes[r];
    n.right = nr.left;
    if (nr.left >= 0) m_nodes[nr.left].parent = idx;

    nr.left = idx;
    nr.parent = n.parent;
    n.parent = r;

    // Update weights bottom-up
    updateWeight(idx);
    updateWeight(r);

    return r;
}

/* ---- Rotate right ---- */

int WeightBalancedTree11::rotateRight(int idx)
{
    WBNode& n = m_nodes[idx];
    int l = n.left;
    if (l < 0) return idx;

    WBNode& nl = m_nodes[l];
    n.left = nl.right;
    if (nl.right >= 0) m_nodes[nl.right].parent = idx;

    nl.right = idx;
    nl.parent = n.parent;
    n.parent = l;

    updateWeight(idx);
    updateWeight(l);

    return l;
}

/* ---- 3-way rebalance: pick best among rotation options ---- */

void WeightBalancedTree11::rebalance(int idx)
{
    if (idx < 0) return;

    // Walk up to root, checking and fixing imbalances
    int current = idx;
    while (current >= 0) {
        updateWeight(current);

        if (isUnbalanced(current)) {
            WBNode& n = m_nodes[current];
            int oldWeight = n.weight;
            int parent = n.parent;
            int newRoot = current;

            if (n.balance < m_alpha) {
                // Left subtree too heavy → rotate right
                // But check if 3-way (LR) is better
                if (n.left >= 0 && m_nodes[n.left].balance > (1.0 - m_alpha)) {
                    // Left child is right-heavy: double rotation (LR)
                    n.left = rotateLeft(n.left);
                    if (n.left >= 0) m_nodes[n.left].parent = current;
                    newRoot = rotateRight(current);
                } else {
                    newRoot = rotateRight(current);
                }
            } else {
                // Right subtree too heavy → rotate left
                if (n.right >= 0 && m_nodes[n.right].balance < m_alpha) {
                    // Right child is left-heavy: double rotation (RL)
                    n.right = rotateRight(n.right);
                    if (n.right >= 0) m_nodes[n.right].parent = current;
                    newRoot = rotateLeft(current);
                } else {
                    newRoot = rotateLeft(current);
                }
            }

            // Reconnect with parent
            if (parent < 0) {
                m_root = newRoot;
            } else {
                if (m_nodes[parent].left == current)
                    m_nodes[parent].left = newRoot;
                else
                    m_nodes[parent].right = newRoot;
            }
            m_nodes[newRoot].parent = parent;

            emit rebalanceDone(newRoot, oldWeight, m_nodes[newRoot].weight);
            current = parent;
        } else {
            current = n.parent;
        }
    }
}

/* ---- Propagate balance factors ---- */

void WeightBalancedTree11::propagateBalance(int idx)
{
    while (idx >= 0) {
        updateWeight(idx);
        idx = m_nodes[idx].parent;
    }
}

/* ---- Insert ---- */

bool WeightBalancedTree11::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0) {
        m_root = allocNode(key, value);
        m_size = 1;
        m_stats.treeSize = 1;
        m_stats.treeHeight = 1;
        return true;
    }

    // BST search for insertion point
    int current = m_root;
    int parent = -1;

    while (current >= 0) {
        parent = current;
        if (key < m_nodes[current].key) {
            current = m_nodes[current].left;
        } else if (key > m_nodes[current].key) {
            current = m_nodes[current].right;
        } else {
            // Key exists: update value
            m_nodes[current].value = value;
            return false;
        }
    }

    int newNode = allocNode(key, value);
    m_nodes[newNode].parent = parent;

    if (key < m_nodes[parent].key)
        m_nodes[parent].left = newNode;
    else
        m_nodes[parent].right = newNode;

    m_size++;

    // Rebalance from the new node upward
    rebalance(parent);
    propagateBalance(newNode);

    m_stats.treeSize = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit insertDone(key, m_stats.treeHeight, timer.elapsed());

    return true;
}

/* ---- Remove ---- */

bool WeightBalancedTree11::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Find node
    int current = m_root;
    while (current >= 0) {
        if (key < m_nodes[current].key)
            current = m_nodes[current].left;
        else if (key > m_nodes[current].key)
            current = m_nodes[current].right;
        else
            break;
    }

    if (current < 0) return false;  // Not found

    int rebalanceStart = m_nodes[current].parent;

    // Case 1: Leaf or single child
    int left = m_nodes[current].left;
    int right = m_nodes[current].right;

    if (left < 0 && right < 0) {
        // Leaf node
        int p = m_nodes[current].parent;
        if (p < 0) m_root = -1;
        else if (m_nodes[p].left == current) m_nodes[p].left = -1;
        else m_nodes[p].right = -1;
        rebalanceStart = p;
    } else if (left < 0 || right < 0) {
        // Single child
        int child = (left >= 0) ? left : right;
        int p = m_nodes[current].parent;
        m_nodes[child].parent = p;
        if (p < 0) m_root = child;
        else if (m_nodes[p].left == current) m_nodes[p].left = child;
        else m_nodes[p].right = child;
        rebalanceStart = p;
    } else {
        // Two children: find in-order successor
        int succ = right;
        while (m_nodes[succ].left >= 0)
            succ = m_nodes[succ].left;

        // Copy successor data
        m_nodes[current].key = m_nodes[succ].key;
        m_nodes[current].value = m_nodes[succ].value;

        // Remove successor (it has at most one right child)
        int sp = m_nodes[succ].parent;
        int sr = m_nodes[succ].right;
        if (sp >= 0) {
            if (m_nodes[sp].left == succ) m_nodes[sp].left = sr;
            else m_nodes[sp].right = sr;
        }
        if (sr >= 0) m_nodes[sr].parent = sp;
        rebalanceStart = sp;
        current = succ;  // Free this node
    }

    freeNode(current);
    m_size--;

    if (rebalanceStart >= 0)
        rebalance(rebalanceStart);
    else if (m_root >= 0)
        propagateBalance(m_root);

    m_stats.treeSize = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit removeDone(key, m_stats.treeHeight, timer.elapsed());

    return true;
}

/* ---- Search ---- */

WeightBalancedTree11::SearchResult WeightBalancedTree11::search(int key) const
{
    SearchResult result;
    int current = m_root;
    int depth = 0;

    while (current >= 0) {
        if (key < m_nodes[current].key) {
            current = m_nodes[current].left;
            depth++;
        } else if (key > m_nodes[current].key) {
            current = m_nodes[current].right;
            depth++;
        } else {
            result.nodeIdx = current;
            result.value = m_nodes[current].value;
            result.found = true;
            result.depth = depth;
            return result;
        }
    }
    result.depth = depth;
    return result;
}

/* ---- Inorder traversal ---- */

void WeightBalancedTree11::inOrderHelper(int idx, QVector<int>& result) const
{
    if (idx < 0) return;
    inOrderHelper(m_nodes[idx].left, result);
    result.append(m_nodes[idx].key);
    inOrderHelper(m_nodes[idx].right, result);
}

QVector<int> WeightBalancedTree11::inOrderKeys() const
{
    QVector<int> result;
    result.reserve(m_size);
    inOrderHelper(m_root, result);
    return result;
}

/* ---- Get node ---- */

const WeightBalancedTree11::WBNode& WeightBalancedTree11::node(int idx) const
{
    return m_nodes[idx];
}

/* ---- Compute height ---- */

int WeightBalancedTree11::computeHeight(int idx) const
{
    if (idx < 0) return 0;
    int lh = computeHeight(m_nodes[idx].left);
    int rh = computeHeight(m_nodes[idx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Reset ---- */

void WeightBalancedTree11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_root = -1;
    m_size = 0;
    m_nodes.clear();
    m_freeList.clear();
}
