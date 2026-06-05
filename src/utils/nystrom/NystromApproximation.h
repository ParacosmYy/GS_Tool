/**
 * @file NystromApproximation.h
 * @brief Nystrom方法 — 大规模核矩阵的低秩近似
 *
 * 功能: 使用Nystrom方法对大规模核矩阵K进行低秩近似。
 *       通过采样m个landmark点，构造K_nm * K_mm^{-1} * K_nm^T
 *       来近似完整核矩阵。适用于核PCA、高斯过程等场景。
 *       统计近似次数和平均耗时。
 */
#ifndef NYSTROMAPPROXIMATION_H
#define NYSTROMAPPROXIMATION_H

#include <QObject>
#include <QVector>

#include <functional>

/**
 * @class NystromApproximation
 * @brief Nystrom方法工具类，大规模核矩阵低秩近似
 */
class NystromApproximation : public QObject {
    Q_OBJECT
public:
    /** 统计信息结构体 */
    struct Stats {
        quint64 totalApproximations = 0;  /**< 总近似次数 */
        double  avgProcessingTimeMs = 0.0; /**< 平均处理耗时(ms) */
    };

    /**
     * @brief 构造函数
     * @param parent 父QObject
     */
    explicit NystromApproximation(QObject* parent = nullptr);

    /**
     * @brief 使用Nystrom方法近似核矩阵
     * @param kernelFunc 核函数，接受两个索引(i,j)返回核值
     * @param n 核矩阵维度
     * @param numLandmarks landmark采样点数
     * @return 近似的n x n核矩阵
     */
    QVector<QVector<double>>
    approximate(std::function<double(int, int)> kernelFunc,
                int n, int numLandmarks);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 近似完成信号 @param numLandmarks landmark点数 */
    void approximationCompleted(int numLandmarks);

private:
    /**
     * @brief 使用Gauss消元求解线性系统
     * @param A 系数矩阵
     * @param b 右端向量
     * @return 解向量x
     */
    QVector<double> solveSystem(QVector<QVector<double>> A,
                                QVector<double> b);

    Stats  m_stats;   /**< 统计数据 */
    double m_timeSum; /**< 累计耗时(ms) */
};

#endif // NYSTROMAPPROXIMATION_H
