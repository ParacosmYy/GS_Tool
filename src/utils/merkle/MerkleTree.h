/**
 * @file MerkleTree.h
 * @brief 默克尔树 — 哈希树数据完整性验证
 *
 * 功能: 构建默克尔哈希树，验证数据完整性，生成/验证
 *       默克尔证明(Merkle proof)，统计树构建/验证次数。
 */
#ifndef MERKLETREE_H
#define MERKLETREE_H

#include <QObject>
#include <QByteArray>
#include <QVector>

class MerkleTree : public QObject {
    Q_OBJECT
public:
    /** 默克尔证明 */
    struct Proof {
        QByteArray rootHash;           ///< 根哈希
        QVector<QByteArray> siblings;  ///< 兄弟节点哈希
        QVector<bool> directions;      ///< true=右兄弟, false=左兄弟
        int leafIndex;                 ///< 叶节点索引
    };

    /** 统计 */
    struct Stats {
        quint64 totalBuilds = 0;
        quint64 totalProofsGenerated = 0;
        quint64 totalProofsVerified = 0;
        int     leafCount = 0;
        int     treeDepth = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit MerkleTree(QObject* parent = nullptr);

    /** @brief 从叶节点数据构建树 @param leaves 叶数据列表 */
    void build(const QVector<QByteArray>& leaves);

    /** @brief 获取根哈希 @return 根哈希 */
    QByteArray rootHash() const { return m_rootHash; }

    /** @brief 生成默克尔证明 @param leafIndex 叶索引 @return 证明 */
    Proof generateProof(int leafIndex);

    /** @brief 验证默克尔证明 @param proof 证明 @param leafData 叶数据 @return 是否有效 */
    bool verifyProof(const Proof& proof, const QByteArray& leafData);

    /** @brief 更新单个叶节点 @param leafIndex 叶索引 @param newData 新数据 */
    void updateLeaf(int leafIndex, const QByteArray& newData);

    /** @brief 验证整棵树完整性 @return 是否有效 */
    bool verifyTree() const;

    /** @brief 叶节点数 */
    int leafCount() const { return m_leaves.size(); }

    /** @brief 树深度 */
    int depth() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeBuilt(int leafCount, int depth);
    void proofGenerated(int leafIndex);
    void proofVerified(int leafIndex, bool valid);
    void leafUpdated(int leafIndex);

private:
    QByteArray computeHash(const QByteArray& data) const;
    void rebuildTree();

    QVector<QByteArray> m_leaves;       ///< 叶节点哈希
    QVector<QVector<QByteArray>> m_levels; ///< 每层哈希(level[0]=叶层)
    QByteArray m_rootHash;
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // MERKLETREE_H
