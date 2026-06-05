/**
 * @file KMedoids.h
 * @brief K-Medoids聚类引擎 — PAM(Partitioning Around Medoids)算法
 *
 * 实现K-Medoids/PAM聚类算法, 以实际数据点为中心(Medoid),
 * 比K-Means对离群点更鲁棒, 适用于传感器数据分群、信号模式分类等场景。
 */
#ifndef KMEDOIDS_H
#define KMEDOIDS_H

#include <QObject>
#include <QVector>

/**
 * @class KMedoids
 * @brief K-Medoids聚类 — PAM算法实现
 *
 * 典型用法:
 * @code
 *   KMedoids km;
 *   km.fit(data, 3);
 *   int cluster = km.predict(point);
 * @endcode
 */
class KMedoids : public QObject {
    Q_OBJECT

public:
    /** @brief 数据点类型(多维坐标) */
    using Point = QVector<double>;

    /** @brief 拟合结果结构 */
    struct FitResult {
        QVector<int> labels;        ///< 每个数据点的簇标签
        QVector<int> medoidIndices;  ///< Medoid点在原始数据中的索引
        int    iterations = 0;      ///< 迭代次数
        double totalCost = 0.0;     ///< 总代价(距离之和)
        bool   converged = false;   ///< 是否收敛
    };

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalFits = 0;          ///< 总拟合次数
        quint64 totalPredictions = 0;   ///< 总预测次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit KMedoids(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~KMedoids() override;

    // ── 核心接口 ──

    /**
     * @brief 拟合K-Medoids模型
     * @param data 数据点集合(每个Point为一组坐标)
     * @param k 簇数量
     * @param maxIter 最大迭代次数(默认100)
     * @return 拟合结果
     */
    FitResult fit(const QVector<Point>& data, int k, int maxIter = 100);

    /**
     * @brief 预测数据点所属簇
     * @param point 待预测数据点
     * @return 簇标签(-1表示模型未训练)
     */
    int predict(const Point& point) const;

    /**
     * @brief 模型是否已训练
     * @return true表示已拟合
     */
    bool isFitted() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 拟合完成信号 @param k 簇数 @param iterations 迭代次数 */
    void fitCompleted(int k, int iterations);
    /** @brief 预测完成信号 @param label 簇标签 */
    void predictionCompleted(int label);

private:
    /** @brief 计算两点间欧氏距离 */
    double distance(const Point& a, const Point& b) const;

    /** @brief 计算指定medoid集合的总代价 */
    double computeCost(const QVector<Point>& data,
                       const QVector<int>& medoidIndices,
                       QVector<int>& labels) const;

    /** @brief 当前medoid点 */
    QVector<Point> m_medoids;

    /** @brief 是否已拟合 */
    bool m_fitted = false;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // KMEDOIDS_H
