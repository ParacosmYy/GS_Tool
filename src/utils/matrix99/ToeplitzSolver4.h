#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Toeplitz矩阵线性求解器
 *
 * 利用Levinson-Durbin算法在O(n^2)时间内求解Toeplitz方程组,
 * 适用于自回归模型参数估计、维纳滤波器设计与语音线性预测。
 */
class ToeplitzSolver4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit ToeplitzSolver4(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int dim);

    /** @brief 设置第一行元素定义Toeplitz矩阵 */
    void setFirstRow(const QVector<double>& row);

    /** @brief 求解 Toeplitz * x = rhs */
    void solve(const QVector<double>& rhs);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号,返回维度与残差 */
    void solveCompleted(int dimension, double residual);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
};
