/**
 * @file MeanShift2.h
 * @brief 均值漂移聚类 — 自适应带宽核密度估计 + 模式搜索
 *
 * 功能: 实现Mean Shift聚类算法，支持高斯核和Epanechnikov核，
 *       自适应带宽选择，自动确定簇数量，模式搜索收敛判定。
 *       适用于点云聚类、图像分割、密度估计、异常检测。
 *
 * 协作: DataClassifier(分类) / OpticsClustering(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

#include <vector>

/**
 * @brief 均值漂移聚类 — 自适应带宽核密度估计
 */
class MeanShift2 : public QObject {
    Q_OBJECT

public:
    /** @brief 核函数类型 */
    enum KernelType {
        Gaussian = 0,          ///< 高斯核(无限支撑，平滑)
        Epanechnikov = 1       ///< Epanechnikov核(有限支撑，高效)
    };
    Q_ENUM(KernelType)

    /** @brief 聚类参数 */
    struct Parameters {
        double bandwidth = 1.0;            ///< 核带宽(>0)
        KernelType kernel = Gaussian;      ///< 核函数类型
        int maxIterations = 300;           ///< 最大迭代次数
        double convergenceTol = 1e-4;      ///< 收敛容差
        double clusterMergeDist = 0.1;     ///< 簇合并距离(相对带宽)
        bool adaptiveBandwidth = false;    ///< 是否启用自适应带宽
        int kNearest = 5;                  ///< 自适应带宽K近邻数
        double minDensity = 0.0;           ///< 最小密度阈值(过滤噪声)
    };

    /** @brief 聚类结果 */
    struct ClusterResult {
        QVector<int> labels;               ///< 每个点簇标签(-1为噪声)
        QVector<QVector<double>> modes;    ///< 模式点(簇中心)
        QVector<double> densities;         ///< 各模式密度值
        int numClusters = 0;               ///< 簇数量
        int iterations = 0;                ///< 总迭代次数
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalPointsProcessed = 0;  ///< 累计处理点数
        quint64 totalClustersFound = 0;    ///< 累计发现簇数
        quint64 totalShiftsComputed = 0;   ///< 累计漂移计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MeanShift2(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~MeanShift2() override;

    // ── 配置 ──

    /** @brief 设置聚类参数 @param params 参数 */
    void setParameters(const Parameters& params);

    /** @brief 获取当前参数 @return 参数 */
    Parameters parameters() const;

    // ── 核心聚类 ──

    /**
     * @brief 对1D数据进行均值漂移聚类
     * @param data 输入数据点
     * @return 聚类结果
     */
    ClusterResult cluster1D(const QVector<double>& data);

    /**
     * @brief 对多维数据进行均值漂移聚类
     * @param data 输入数据(每行一个样本，每列一个特征)
     * @return 聚类结果
     */
    ClusterResult cluster(const QVector<QVector<double>>& data);

    // ── 密度估计 ──

    /**
     * @brief 核密度估计(单点)
     * @param point 查询点
     * @param data 数据集
     * @return 密度值
     */
    double kernelDensity(const QVector<double>& point,
                         const QVector<QVector<double>>& data) const;

    /**
     * @brief 单次均值漂移向量
     * @param point 当前点
     * @param data 数据集
     * @return 漂移后的新位置
     */
    QVector<double> shiftPoint(const QVector<double>& point,
                               const QVector<QVector<double>>& data) const;

    // ── 统计 ──

    /** @brief 获取统计 @return 统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param numClusters 簇数量 @param numPoints 数据点数 */
    void clusteringCompleted(int numClusters, int numPoints);

    /** @brief 模式收敛 @param mode 模式点 @param density 密度值 @param iters 迭代次数 */
    void modeConverged(const QVector<double>& mode, double density, int iters);

private:
    /**
     * @brief 高斯核函数值
     * @param distanceSq 距离平方
     * @param bw 带宽
     * @return 核值
     */
    double kernelGaussian(double distanceSq, double bw) const;

    /**
     * @brief Epanechnikov核函数值
     * @param distanceSq 距离平方
     * @param bw 带宽
     * @return 核值
     */
    double kernelEpanechnikov(double distanceSq, double bw) const;

    /**
     * @brief 计算两点欧氏距离平方
     * @param a 点A
     * @param b 点B
     * @return 距离平方
     */
    double distSq(const QVector<double>& a, const QVector<double>& b) const;

    /**
     * @brief 计算自适应带宽
     * @param point 查询点
     * @param data 数据集
     * @return 局部带宽
     */
    double adaptiveBW(const QVector<double>& point,
                      const QVector<QVector<double>>& data) const;

    /**
     * @brief 合并相近模式
     * @param modes 模式列表
     * @param densities 密度列表
     * @param mergeDist 合并距离
     */
    void mergeModes(QVector<QVector<double>>& modes,
                    QVector<double>& densities,
                    double mergeDist) const;

    /**
     * @brief 分配标签(按最近模式)
     * @param data 数据集
     * @param modes 模式列表
     * @param minDensity 最小密度阈值
     * @return 标签列表
     */
    QVector<int> assignLabels(const QVector<QVector<double>>& data,
                              const QVector<QVector<double>>& modes,
                              double minDensity) const;

    Parameters m_params;                ///< 聚类参数
    Stats m_stats;                      ///< 操作统计
    double m_timeSum = 0.0;             ///< 累计耗时
};
