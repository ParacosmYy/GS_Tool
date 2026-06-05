/**
 * @file ApproxNearestNeighbor.h
 * @brief 近似最近邻搜索 — 基于LSH(局部敏感哈希)
 *
 * 功能: 使用LSH(Locality-Sensitive Hashing)实现高维数据的近似最近邻搜索。
 *       通过随机投影哈希函数将相似点映射到相同桶中，实现次线性查询复杂度。
 *       适用于嵌入式数据可视化中的高维特征匹配和相似性搜索。
 *
 * 协作: KdTreeBalancer(精确最近邻) / KDTree(通用k维) / MinHash(集合相似)
 */
#ifndef APPROXNEARESTNEIGHBOR_H
#define APPROXNEARESTNEIGHBOR_H

#include <QObject>
#include <QVector>

/**
 * @brief 基于LSH的近似最近邻搜索器
 *
 * 使用多个哈希表和随机投影实现LSH。
 * 支持k近邻查询，返回最相似的k个点索引。
 */
class ApproxNearestNeighbor : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalBuilds = 0;       ///< 累计建树次数
        quint64 totalQueries = 0;      ///< 累计查询次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit ApproxNearestNeighbor(int numTables = 6, int numHashes = 4,
                                   QObject* parent = nullptr);
    ~ApproxNearestNeighbor();

    /**
     * @brief 从点集构建LSH索引
     * @param points 高维点集，每个点为特征向量
     */
    void build(const QVector<QVector<double>>& points);

    /**
     * @brief 查询k个近似最近邻
     * @param point 查询点
     * @param k 返回的最近邻数量
     * @return 最近邻的索引列表，按距离排序
     */
    QVector<int> query(const QVector<double>& point, int k);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 查询完成信号 @param k 返回的邻居数量 */
    void queryCompleted(int k);

private:
    /** @brief 单个哈希表 */
    struct HashTable {
        QVector<QVector<double>> hyperplanes; ///< 随机超平面法向量
        QMap<QString, QVector<int>> buckets;  ///< 哈希桶: key -> 点索引列表
    };

    QString hashPoint(const QVector<double>& point, int tableIdx);
    double euclideanDist(const QVector<double>& a,
                         const QVector<double>& b) const;
    void generateHyperplanes();

    int m_numTables;         ///< 哈希表数量
    int m_numHashes;         ///< 每个表的哈希函数数量
    int m_dimensions;        ///< 数据维度
    QVector<QVector<double>> m_points; ///< 存储的点集
    QVector<HashTable> m_tables;       ///< 哈希表数组
    Stats m_stats;           ///< 统计信息
    double m_timeSum;        ///< 处理时间累加器
};

#endif // APPROXNEARESTNEIGHBOR_H
