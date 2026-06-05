/**
 * @file DynamicTimeWarping.h
 * @brief 动态时间规整(DTW)距离 — Sakoe-Chiba带约束
 *
 * 功能: 计算两个时间序列之间的DTW距离，支持全局约束
 *       (Sakoe-Chiba带)和局部步进模式，返回最优规整路径。
 *
 * 协作: DataSynchronizer(时间对齐) / CrossCorrelator(时延检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 动态时间规整 — Sakoe-Chiba带约束
 */
class DynamicTimeWarping : public QObject {
    Q_OBJECT

public:
    /** @brief 步进模式 */
    enum class StepPattern {
        Symmetric,      ///< 对称步进: (1,1)/(2,1)/(1,2)
        Asymmetric,     ///< 非对称步进: (1,0)/(1,1)/(1,2)
        SymmetricP0     ///< 对称P0: (1,1)/(1,0)/(0,1)
    };
    Q_ENUM(StepPattern)

    /** @brief 距离度量 */
    enum class DistanceMetric {
        Euclidean,      ///< 欧氏距离
        Manhattan,      ///< 曼哈顿距离
        Cosine          ///< 余弦距离
    };
    Q_ENUM(DistanceMetric)

    /** @brief 统计 */
    struct Stats {
        int totalComputations = 0;          ///< 累计计算次数
        int totalWarpsComputed = 0;         ///< 累计规整计算数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double totalDistanceSum = 0.0;       ///< 累计距离总和
        quint64 totalCellsEvaluated = 0;    ///< 累计评估单元格数
    };

    /** @brief DTW结果 */
    struct DtwResult {
        double distance = 0.0;          ///< DTW距离
        double normalizedDistance = 0.0; ///< 归一化DTW距离
        QVector<QPair<int,int>> path;   ///< 最优规整路径
        QVector<QVector<double>> costMatrix; ///< 代价矩阵(可选)
    };

    explicit DynamicTimeWarping(QObject* parent = nullptr);

    /** @brief 设置步进模式 @param pattern 步进模式 */
    void setStepPattern(StepPattern pattern);

    /** @brief 设置距离度量 @param metric 距离度量 */
    void setDistanceMetric(DistanceMetric metric);

    /** @brief 设置Sakoe-Chiba带宽度 @param bandwidth 带宽(0=无约束) */
    void setBandWidth(int bandwidth);

    /**
     * @brief 计算DTW距离
     * @param series1 时间序列1
     * @param series2 时间序列2
     * @return DTW结果(含路径)
     */
    DtwResult compute(const QVector<double>& series1,
                      const QVector<double>& series2);

    /**
     * @brief 计算快速DTW(降采样后递归)
     * @param series1 时间序列1
     * @param series2 时间序列2
     * @param radius 收缩半径
     * @return DTW结果
     */
    DtwResult fastDtw(const QVector<double>& series1,
                      const QVector<double>& series2, int radius = 1);

    /**
     * @brief 计算DTW距离矩阵(多序列)
     * @param series 序列集合
     * @return 距离矩阵
     */
    QVector<QVector<double>> distanceMatrix(
        const QVector<QVector<double>>& series);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief DTW计算完成 @param distance 距离 @param pathLength 路径长度 */
    void computationComplete(double distance, int pathLength);

private:
    /** @brief 计算两点距离 @param a 点A @param b 点B @return 距离 */
    double pointDistance(double a, double b) const;

    /** @brief 回溯最优路径 @param cost 代价矩阵 @param n 行数 @param m 列数 @return 路径 */
    QVector<QPair<int,int>> backtracePath(
        const QVector<QVector<double>>& cost, int n, int m) const;

    /** @brief 降采样 @param series 序列 @param factor 因子 @return 降采样序列 */
    QVector<double> downsample(const QVector<double>& series, int factor) const;

    /** @brief 扩展窗口约束 @param path 粗路径 @param radius 半径 @param n 序列1长度 @param m 序列2长度 */
    QVector<QVector<bool>> expandWindow(
        const QVector<QPair<int,int>>& path, int radius,
        int n, int m) const;

    StepPattern m_stepPattern = StepPattern::Symmetric;     ///< 步进模式
    DistanceMetric m_metric = DistanceMetric::Euclidean;    ///< 距离度量
    int m_bandWidth = 0;            ///< 带宽约束
    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 累计耗时
};
