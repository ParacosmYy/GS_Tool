/**
 * @file AgglomerativeClusterer.h
 * @brief 层次凝聚聚类 — 自底向上的聚类合并
 *
 * 功能: 实现层次凝聚聚类算法，支持 Single/Complete/
 *       Average/Ward 四种链接策略。提供簇标签分配、
 *       树状图合并顺序和距离矩阵计算。
 *
 * 协作: DbScan(密度聚类) / KMeansClusterer(中心聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <functional>

/**
 * @brief 层次凝聚聚类器
 */
class AgglomerativeClusterer : public QObject {
    Q_OBJECT

public:
    /** @brief 链接策略 */
    enum class Linkage {
        Single,   ///< 单链接(最小距离)
        Complete, ///< 全链接(最大距离)
        Average,  ///< 平均链接
        Ward      ///< Ward方差最小化
    };
    Q_ENUM(Linkage)

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalClusterings = 0;     ///< 累计聚类次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param numClusters 目标簇数(默认2)
     * @param parent 父对象
     */
    explicit AgglomerativeClusterer(int numClusters = 2,
                                      QObject* parent = nullptr);

    /**
     * @brief 设置链接策略
     * @param type 链接类型
     */
    void setLinkage(Linkage type);

    /**
     * @brief 执行层次凝聚聚类
     * @param data 数据点集(每行一个点)
     * @return 每个点的簇标签(0~numClusters-1)
     *
     * 自底向上逐步合并最接近的两个簇，直到剩余簇数
     * 等于 numClusters。
     */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /**
     * @brief 获取树状图合并顺序
     * @return 合并序列，每对为 (簇A, 簇B) 的索引
     *
     * 必须在 fit() 之后调用，返回完整的合并历史。
     */
    QVector<QPair<int, int>> dendrogram() const;

    /**
     * @brief 计算距离矩阵
     * @param data 数据点集
     * @return 对称距离矩阵(N x N)
     *
     * 使用欧氏距离计算所有点对之间的距离。
     */
    QVector<QVector<double>> distanceMatrix(
        const QVector<QVector<double>>& data) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 设置目标簇数 */
    void setNumClusters(int k);

    /** @brief 获取当前链接策略 */
    Linkage linkage() const { return m_linkage; }

signals:
    /** @brief 聚类完成 @param numClusters 最终簇数 @param numPoints 数据点数 */
    void clusteringCompleted(int numClusters, int numPoints);

private:
    /**
     * @brief 计算两个点之间的欧氏距离
     * @param a 点A @param b 点B @return 距离值
     */
    double euclidean(const QVector<double>& a,
                      const QVector<double>& b) const;

    /**
     * @brief 合并两个簇后的链接距离
     * @param dAC 簇A与簇C的距离
     * @param dBC 簇B与簇C的距离
     * @param dAB 簇A与簇B的距离
     * @param sizeA 簇A大小
     * @param sizeB 簇B大小
     * @param sizeC 簇C大小
     * @return 合并簇(A+B)与簇C的距离
     */
    double linkageDistance(double dAC, double dBC, double dAB,
                            int sizeA, int sizeB, int sizeC) const;

    int                        m_numClusters; ///< 目标簇数
    Linkage                    m_linkage;     ///< 链接策略
    double                     m_timeSum;     ///< 处理时间累加器
    Stats                      m_stats;       ///< 统计信息
    QVector<QPair<int, int>>   m_dendrogram;  ///< 合并历史
};
