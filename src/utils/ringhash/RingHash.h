/**
 * @file RingHash.h
 * @brief 一致性哈希 — 虚拟节点环
 *
 * 功能: 一致性哈希环，支持虚拟节点、动态添加/删除节点、
 *       键查找，统计迁移数/查找次数/耗时。
 */
#ifndef RINGHASH_CONSISTENT_H
#define RINGHASH_CONSISTENT_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVector>

class RingHash : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalLookups = 0;
        quint64 totalAdds = 0;
        quint64 totalRemoves = 0;
        quint64 totalMigrations = 0;
        int     nodeCount = 0;
        int     virtualNodes = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    /**
     * @brief 构造
     * @param virtualNodesPerNode 每个物理节点的虚拟节点数
     * @param parent 父对象
     */
    explicit RingHash(int virtualNodesPerNode = 150,
                       QObject* parent = nullptr);

    /** @brief 添加节点 @param node 节点名 */
    void addNode(const QString& node);

    /** @brief 移除节点 @param node 节点名 */
    void removeNode(const QString& node);

    /** @brief 查找键对应的节点 @param key 键 @return 节点名 */
    QString lookup(const QString& key) const;

    /** @brief 查找键对应的多个节点(复制) @param key 键 @param count 数量 @return 节点列表 */
    QVector<QString> lookupN(const QString& key, int count) const;

    /** @brief 获取所有节点 @return 节点集合 */
    QSet<QString> nodes() const { return m_nodes; }

    /** @brief 节点数 */
    int nodeCount() const { return m_nodes.size(); }

    /** @brief 环大小(虚拟节点总数) */
    int ringSize() const { return m_ring.size(); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void nodeAdded(const QString& node);
    void nodeRemoved(const QString& node);
    void keyMigrated(const QString& key, const QString& from, const QString& to);

private:
    quint32 hashKey(const QString& key) const;
    quint32 hashVirtual(const QString& node, int replica) const;

    int m_vnodesPerNode;
    QMap<quint32, QString> m_ring;    ///< hash → node
    QSet<QString> m_nodes;
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // RINGHASH_CONSISTENT_H
