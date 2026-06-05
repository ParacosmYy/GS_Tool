#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Toeplitz矩阵线性方程求解器
 *
 * 利用Levinson-Durbin算法O(N^2)求解Toeplitz系统，
 * 广泛用于自回归模型和信号预测。
 */
class ToeplitzSolver5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit ToeplitzSolver5(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int dim);

    /** @brief 设置矩阵首行元素 */
    void setFirstRow(const QVector<double>& row);

    /** @brief 求解线性方程组 Tx=b */
    QVector<double> solve(const QVector<double>& rhs);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号 */
    void solveCompleted(int dimension, double residual);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
};
