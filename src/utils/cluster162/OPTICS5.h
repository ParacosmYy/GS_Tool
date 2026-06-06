/**
 * @file OPTICS5.h
 * @brief OPTICS排序与Xi聚类提取 — OPTICS Ordering with Reachability Plot and Xi Cluster Extraction
 *
 * 功能: 实现OPTICS聚类算法，生成可达距离排序(reachability plot)，
 *       支持Xi方法从排序中自动提取层次聚类结构。
 *       基于核心距离和可达距离的密度聚类，无需预设簇数。
 *
 * 协作: GaussianMixture11(GMM聚类) / DBSCAN8(密度聚类) / KMedoids13(PAM聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief OPTICS聚类排序器
 */
class OPTICS5 : public QObject {
    Q_OBJECT

public:
    /** @brief 排序结果条目 */
    struct OrderEntry {
        int index;                      ///< 原始数据索引
        double reachability;            ///< 可达距离(undefined用-1表示)
        double coreDistance;             ///< 核心距离(undefined用-1表示)
    };

    /** @brief Xi聚类提取结果 */
    struct Cluster {
        QVector<int> indices;           ///< 簇内样本索引
        int startOrder;                 ///< 排序中起始位置
        int endOrder;                   ///< 排序中结束位置
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;           ///< 累计运行次数
        int lastClusterCount = 0;        ///< 最近一次提取的簇数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
    };

    explicit OPTICS5(QObject* parent = nullptr);
    ~OPTICS5() override;

    /** @brief 设置邻域半径epsilon */
    void setEpsilon(double eps);
    /** @brief 设置最小点数MinPts */
    void setMinPoints(int minPts);
    /** @brief 设置Xi陡度阈值(0~1) */
    void setXi(double xi);

    /**
     * @brief 运行OPTICS排序
     * @param data 输入数据，每个元素为特征向量
     * @return 可达距离排序
     */
    QVector<OrderEntry> run(const QVector<QVector<double>>& data);

    /**
     * @brief 从排序结果提取Xi聚类
     * @param ordering OPTICS排序
     * @return 提取的簇列表
     */
    QVector<Cluster> extractXi(const QVector<OrderEntry>& ordering);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 排序完成 @param n 排序长度 */
    void orderingCompleted(int n);
    /** @brief Xi提取完成 @param k 簇数 */
    void xiExtractionCompleted(int k);

private:
    /** @brief 计算欧氏距离 */
    static double euclidean(const QVector<double>& a, const QVector<double>& b);

    /** @brief 计算核心距离 */
    double coreDistance(const QVector<int>& neighbors, const QVector<double>& dists) const;

    /** @brief 查找epsilon邻域 */
    void findNeighbors(const QVector<QVector<double>>& data, int idx,
                       QVector<int>& indices, QVector<double>& dists) const;

    /** @brief 更新种子列表的可达距离 */
    void updateSeeds(const QVector<QVector<double>>& data, int idx,
                     const QVector<int>& neighbors, const QVector<double>& dists,
                     QVector<double>& reachDist, QVector<bool>& processed,
                     QVector<int>& orderedList, QVector<OrderEntry>& result) const;

    double m_epsilon = 1.0;
    int m_minPts = 5;
    double m_xi = 0.05;

    Stats m_stats;
    double m_timeSum = 0.0;
};
