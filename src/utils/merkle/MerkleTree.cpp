/**
 * @file MerkleTree.cpp
 * @brief 默克尔树实现
 */

#include "utils/merkle/MerkleTree.h"

#include <QElapsedTimer>
#include <QCryptographicHash>

MerkleTree::MerkleTree(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

void MerkleTree::build(const QVector<QByteArray>& leaves)
{
    QElapsedTimer timer;
    timer.start();

    if (leaves.isEmpty()) {
        m_rootHash.clear();
        m_leaves.clear();
        m_levels.clear();
        return;
    }

    /* 计算叶节点哈希 */
    m_leaves.clear();
    for (const auto& leaf : leaves) {
        m_leaves.append(computeHash(leaf));
    }

    rebuildTree();

    m_stats.totalBuilds++;
    m_stats.leafCount = m_leaves.size();
    m_stats.treeDepth = depth();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalBuilds + m_stats.totalProofsGenerated +
             m_stats.totalProofsVerified, 1ULL);

    emit treeBuilt(m_leaves.size(), depth());
}

MerkleTree::Proof MerkleTree::generateProof(int leafIndex)
{
    Proof proof;
    proof.leafIndex = leafIndex;
    proof.rootHash = m_rootHash;

    if (leafIndex < 0 || leafIndex >= m_leaves.size() || m_levels.size() < 2) {
        return proof;
    }

    /* 从叶层向上收集兄弟节点 */
    int idx = leafIndex;
    for (int level = 0; level < static_cast<int>(m_levels.size()) - 1; ++level) {
        const auto& layer = m_levels[level];
        int siblingIdx = (idx % 2 == 0) ? idx + 1 : idx - 1;
        if (siblingIdx < layer.size()) {
            proof.siblings.append(layer[siblingIdx]);
            proof.directions.append(idx % 2 != 0);
        }
        idx /= 2;
    }

    m_stats.totalProofsGenerated++;
    emit proofGenerated(leafIndex);
    return proof;
}

bool MerkleTree::verifyProof(const Proof& proof, const QByteArray& leafData)
{
    QByteArray currentHash = computeHash(leafData);

    /* 从叶向根逐步计算 */
    for (int i = 0; i < proof.siblings.size(); ++i) {
        if (proof.directions[i]) {
            /* 兄弟在左边 */
            currentHash = computeHash(proof.siblings[i] + currentHash);
        } else {
            /* 兄弟在右边 */
            currentHash = computeHash(currentHash + proof.siblings[i]);
        }
    }

    bool valid = (currentHash == proof.rootHash);
    m_stats.totalProofsVerified++;
    emit proofVerified(proof.leafIndex, valid);
    return valid;
}

void MerkleTree::updateLeaf(int leafIndex, const QByteArray& newData)
{
    QElapsedTimer timer;
    timer.start();

    if (leafIndex < 0 || leafIndex >= m_leaves.size()) return;

    m_leaves[leafIndex] = computeHash(newData);
    rebuildTree();

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalBuilds + m_stats.totalProofsGenerated +
             m_stats.totalProofsVerified, 1ULL);

    emit leafUpdated(leafIndex);
}

bool MerkleTree::verifyTree() const
{
    if (m_levels.size() < 2) return true;

    /* 从叶层逐层验证 */
    for (int level = 0; level < static_cast<int>(m_levels.size()) - 1; ++level) {
        const auto& current = m_levels[level];
        const auto& parent = m_levels[level + 1];

        for (int i = 0; i < parent.size(); ++i) {
            QByteArray left = (2 * i < current.size())
                ? current[2 * i] : QByteArray();
            QByteArray right = (2 * i + 1 < current.size())
                ? current[2 * i + 1] : left;
            QByteArray expected = computeHash(left + right);
            if (expected != parent[i]) return false;
        }
    }

    return m_levels.last().first() == m_rootHash;
}

int MerkleTree::depth() const
{
    if (m_leaves.isEmpty()) return 0;
    int d = 0;
    int n = m_leaves.size();
    while (n > 1) {
        n = (n + 1) / 2;
        ++d;
    }
    return d;
}

QByteArray MerkleTree::computeHash(const QByteArray& data) const
{
    return QCryptographicHash::hash(data, QCryptographicHash::Sha256);
}

void MerkleTree::rebuildTree()
{
    m_levels.clear();

    /* 叶层 */
    QVector<QByteArray> current = m_leaves;
    m_levels.append(current);

    /* 逐层向上 */
    while (current.size() > 1) {
        QVector<QByteArray> next;
        for (int i = 0; i < current.size(); i += 2) {
            QByteArray left = current[i];
            QByteArray right = (i + 1 < current.size())
                ? current[i + 1] : left;
            next.append(computeHash(left + right));
        }
        current = next;
        m_levels.append(current);
    }

    m_rootHash = current.isEmpty() ? QByteArray() : current.first();
}

void MerkleTree::resetStatistics()
{
    m_stats = Stats{};
    m_stats.leafCount = m_leaves.size();
    m_stats.treeDepth = depth();
    m_timeSum = 0.0;
}
