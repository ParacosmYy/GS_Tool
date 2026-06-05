#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Toeplitz矩阵求解器
 *
 * 利用Levinson-Durbin算法高效求解Toeplitz线性系统。
 */
class ToeplitzSolver3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSystemsSolved = 0;
        int totalSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ToeplitzSolver3(QObject* parent = nullptr);

    /** @brief 求解Toeplitz系统 Tx=b */
    QVector<double> solve(const QVector<double>& firstRow, const QVector<double>& rhs);

    /** @brief Levinson-Durbin求解Yule-Walker方程 */
    QVector<double> yuleWalker(const QVector<double>& autocorrelation);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void systemSolved(int dimension, double residual);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
