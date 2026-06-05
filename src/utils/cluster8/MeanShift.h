/**
 * @file MeanShift.h
 * @brief Mean Shift聚类引擎 — 核密度估计的模态搜索聚类
 *
 * 功能: 实现Mean Shift聚类算法，通过核密度估计(KDE)迭代搜索
 *       数据分布的模态(密度峰值)。支持多维数据、高斯核/均匀核、
 *       自动带宽选择、聚类合并。适用于串口数据模式识别、
 *       协议帧分类、传感器数据异常簇检测。
 *
 * 协作: AnomalyDetector(异常检测) / DataClassifier(分类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief Mean Shift聚类引擎 — 核密度估计模态搜索
 */
class MeanShift : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalClusterings = 0;              ///< 累计聚类次数
        int totalPointsProcessed = 0;          ///< 累计处理数据点数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    /** @brief 核函数类型 */
    enum class KernelType {
        Gaussian,      ///< 高斯核(默认)
        Uniform,       ///< 均匀核(Epanechnikov)
        Cosine         ///< 余弦核
    };
    Q_ENUM(KernelType)

    /** @brief 聚类结果 */
    struct Cluster {
        QVector<double> center;    ///< 聚类中心(模态)
        QList<int> memberIndices;  ///< 成员索引
        double density = 0.0;      ///< 中心密度估计
    };

    /** @brief 多维数据点类型 */
    using Point = QVector<double>;

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MeanShift(QObject* parent = nullptr);

    /**
     * @brief 设置带宽参数
     * @param bandwidth 核函数带宽(>0)
     */
    void setBandwidth(double bandwidth);

    /**
     * @brief 设置核函数类型
     * @param kernel 核类型
     */
    void setKernel(KernelType kernel);

    /**
     * @brief 设置收敛阈值
     * @param threshold 位移阈值(默认1e-4)
     */
    void setConvergenceThreshold(double threshold);

    /**
     * @brief 设置最大迭代次数
     * @param maxIter 最大迭代(默认300)
     */
    void setMaxIterations(int maxIter);

    /**
     * @brief 执行Mean Shift聚类
     * @param points 输入数据点(每个点为多维向量)
     * @return 聚类列表
     */
    QList<Cluster> cluster(const QList<Point>& points);

    /**
     * @brief 对单个点执行Mean Shift迭代(搜索模态)
     * @param point 起始点
     * @param allPoints 所有数据点
     * @return 收敛到的模态点
     */
    Point shiftPoint(const Point& point,
                     const QList<Point>& allPoints) const;

    /**
     * @brief 核密度估计
     * @param point 目标点
     * @param allPoints 数据点
     * @return 密度估计值
     */
    double kernelDensity(const Point& point,
                         const QList<Point>& allPoints) const;

    /**
     * @brief 自动估计最佳带宽(Silverman法则)
     * @param points 数据点
     * @return 建议带宽
     */
    double estimateBandwidth(const QList<Point>& points) const;

    /**
     * @brief 获取统计信息
     * @return 统计引用
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief 聚类完成
     * @param clusterCount 聚类数
     * @param pointCount 数据点数
     */
    void clusteringCompleted(int clusterCount, int pointCount);

private:
    /**
     * @brief 计算两个多维点之间的欧氏距离
     * @param a 点a
     * @param b 点b
     * @return 欧氏距离
     */
    static double distance(const Point& a, const Point& b);

    /**
     * @brief 核函数值
     * @param distSq 距离平方
     * @return 核权重
     */
    double kernelValue(double distSq) const;

    /**
     * @brief 合并相近的模态为聚类
     * @param modes 所有点收敛到的模态
     * @param points 原始数据点
     * @param mergeThreshold 合并阈值
     * @return 聚类列表
     */
    QList<Cluster> mergeModes(const QList<Point>& modes,
                              const QList<Point>& points,
                              double mergeThreshold) const;

    double m_bandwidth;                 ///< 带宽参数
    KernelType m_kernel;                ///< 核函数类型
    double m_convergenceThreshold;      ///< 收敛阈值
    int m_maxIterations;                ///< 最大迭代次数

    Stats m_stats;
    double m_timeSum = 0.0;             ///< 处理时间累加器
};
