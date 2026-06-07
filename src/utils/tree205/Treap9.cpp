/**
 * @file Treap9.cpp
 * @brief Treap9 实现
 *
 * 实现持久化Treap：路径复制版本持久化、区间求和查询、历史快照访问。
 */

#include "utils/tree205/Treap9.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Treap9::Treap9(QObject *parent) : QObject(parent)
{
    // Create initial empty version
    m_roots.append(-1);
    m_currentVersion = 0;
}

Treap9::~Treap9() = default;

/* ---- Clone node for path copying ---- */

int Treap9::cloneNode(int nodeIdx)
{
    if (nodeIdx < 0) return -1;
    Node clone = m_nodes[nodeIdx];
    int newIdx = m_nodes.size();
    m_nodes.append(clone);
    return newIdx;
}

/* ---- Update subtree sum ---- */

void Treap9::updateSum(int nodeIdx)
{
    if (nodeIdx < 0) return;
    double sum = m_nodes[nodeIdx].value;
    if (m_nodes[nodeIdx].left >= 0) sum += m_nodes[m_nodes[nodeIdx].left].subtreeSum;
    if (m_nodes[nodeIdx].right >= 0) sum += m_nodes[m_nodes[nodeIdx].right].subtreeSum;
    m_nodes[nodeIdx].subtreeSum = sum;
}

/* ---- Rotate right ---- */

int Treap9::rotateRight(int root)
{
    int left = m_nodes[root].left;
    if (left < 0) return root;
    m_nodes[root].left = m_nodes[left].right;
    m_nodes[left].right = root;
    updateSum(root);
    updateSum(left);
    return left;
}

/* ---- Rotate left ---- */

int Treap9::rotateLeft(int root)
{
    int right = m_nodes[root].right;
    if (right < 0) return root;
    m_nodes[root].right = m_nodes[right].left;
    m_nodes[right].left = root;
    updateSum(root);
    updateSum(right);
    return right;
}

/* ---- Insert recursive ---- */

int Treap9::insertRec(int root, int key, double value)
{
    if (root < 0) {
        Node n;
        n.key = key;
        n.value = value;
        n.priority = QRandomGenerator::global()->generate();
        n.subtreeSum = value;
        n.left = -1;
        n.right = -1;
        int idx = m_nodes.size();
        m_nodes.append(n);
        return idx;
    }

    // Path copying: clone current node
    int newRoot = cloneNode(root);

    if (key < m_nodes[newRoot].key) {
        m_nodes[newRoot].left = insertRec(m_nodes[newRoot].left, key, value);
        if (m_nodes[m_nodes[newRoot].left].priority < m_nodes[newRoot].priority)
            newRoot = rotateRight(newRoot);
    } else if (key > m_nodes[newRoot].key) {
        m_nodes[newRoot].right = insertRec(m_nodes[newRoot].right, key, value);
        if (m_nodes[m_nodes[newRoot].right].priority < m_nodes[newRoot].priority)
            newRoot = rotateLeft(newRoot);
    } else {
        // Key exists: update value
        m_nodes[newRoot].value = value;
    }

    updateSum(newRoot);
    return newRoot;
}

/* ---- Remove recursive ---- */

int Treap9::removeRec(int root, int key)
{
    if (root < 0) return -1;

    int newRoot = cloneNode(root);

    if (key < m_nodes[newRoot].key) {
        m_nodes[newRoot].left = removeRec(m_nodes[newRoot].left, key);
    } else if (key > m_nodes[newRoot].key) {
        m_nodes[newRoot].right = removeRec(m_nodes[newRoot].right, key);
    } else {
        // Found key to remove
        if (m_nodes[newRoot].left < 0 && m_nodes[newRoot].right < 0) {
            return -1; // Leaf node
        } else if (m_nodes[newRoot].left < 0) {
            return m_nodes[newRoot].right;
        } else if (m_nodes[newRoot].right < 0) {
            return m_nodes[newRoot].left;
        } else {
            // Two children: rotate and remove
            if (m_nodes[m_nodes[newRoot].left].priority < m_nodes[m_nodes[newRoot].right].priority) {
                newRoot = rotateRight(newRoot);
                m_nodes[newRoot].right = removeRec(m_nodes[newRoot].right, key);
            } else {
                newRoot = rotateLeft(newRoot);
                m_nodes[newRoot].left = removeRec(m_nodes[newRoot].left, key);
            }
        }
    }

    updateSum(newRoot);
    return newRoot;
}

/* ---- Lookup recursive ---- */

QPair<bool, double> Treap9::lookupRec(int root, int key) const
{
    if (root < 0) return {false, 0.0};
    if (key < m_nodes[root].key) return lookupRec(m_nodes[root].left, key);
    if (key > m_nodes[root].key) return lookupRec(m_nodes[root].right, key);
    return {true, m_nodes[root].value};
}

/* ---- Range sum recursive ---- */

double Treap9::rangeSumRec(int root, int lo, int hi) const
{
    if (root < 0) return 0.0;

    if (m_nodes[root].key < lo) return rangeSumRec(m_nodes[root].right, lo, hi);
    if (m_nodes[root].key > hi) return rangeSumRec(m_nodes[root].left, lo, hi);

    // This node is in range [lo, hi]
    double sum = m_nodes[root].value;

    // Add left subtree contributions in range
    if (m_nodes[root].left >= 0) {
        if (m_nodes[m_nodes[root].left].key >= lo) {
            // Entire right subtree of left child may be in range
            sum += rangeSumRec(m_nodes[root].left, lo, hi);
        }
    }

    // Add right subtree contributions in range
    if (m_nodes[root].right >= 0) {
        if (m_nodes[m_nodes[root].right].key <= hi) {
            sum += rangeSumRec(m_nodes[root].right, lo, hi);
        }
    }

    return sum;
}

/* ---- Insert ---- */

void Treap9::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    int root = (m_currentVersion >= 0 && m_currentVersion < m_roots.size())
                    ? m_roots[m_currentVersion] : -1;
    int newRoot = insertRec(root, key, value);

    m_roots.append(newRoot);
    m_currentVersion = m_roots.size() - 1;

    m_stats.totalOps++;
    m_stats.currentSize++;
    m_stats.numVersions = m_roots.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("insert", m_currentVersion, timer.elapsed());
}

/* ---- Remove ---- */

void Treap9::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    int root = (m_currentVersion >= 0 && m_currentVersion < m_roots.size())
                    ? m_roots[m_currentVersion] : -1;
    auto [found, _] = lookupRec(root, key);
    int newRoot = removeRec(root, key);

    m_roots.append(newRoot);
    m_currentVersion = m_roots.size() - 1;

    if (found) m_stats.currentSize--;
    m_stats.totalOps++;
    m_stats.numVersions = m_roots.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("remove", m_currentVersion, timer.elapsed());
}

/* ---- Lookup ---- */

QPair<bool, double> Treap9::lookup(int key) const
{
    if (m_currentVersion < 0 || m_currentVersion >= m_roots.size())
        return {false, 0.0};
    return lookupRec(m_roots[m_currentVersion], key);
}

/* ---- Range sum ---- */

double Treap9::rangeSum(int lo, int hi) const
{
    if (m_currentVersion < 0 || m_currentVersion >= m_roots.size()) return 0.0;
    return rangeSumRec(m_roots[m_currentVersion], lo, hi);
}

/* ---- Create snapshot ---- */

int Treap9::createSnapshot()
{
    // Current version is already stored; just return its ID
    return m_currentVersion;
}

/* ---- Restore snapshot ---- */

void Treap9::restoreSnapshot(int versionId)
{
    if (versionId >= 0 && versionId < m_roots.size())
        m_currentVersion = versionId;
}

/* ---- Lookup at version ---- */

QPair<bool, double> Treap9::lookupVersion(int key, int versionId) const
{
    if (versionId < 0 || versionId >= m_roots.size()) return {false, 0.0};
    return lookupRec(m_roots[versionId], key);
}

/* ---- Range sum at version ---- */

double Treap9::rangeSumVersion(int lo, int hi, int versionId) const
{
    if (versionId < 0 || versionId >= m_roots.size()) return 0.0;
    return rangeSumRec(m_roots[versionId], lo, hi);
}

/* ---- Version count ---- */

int Treap9::versionCount() const { return m_roots.size(); }

/* ---- Reset ---- */

void Treap9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_roots.clear();
    m_currentVersion = -1;
    m_roots.append(-1);
    m_currentVersion = 0;
}
