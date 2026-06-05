#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 稀疏Cholesky分解
 *
 * 针对稀疏对称正定矩阵的Cholesky分解，利用稀疏结构减少计算量。
 */
class SparseCholesky3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFactorizations = 0;
        int totalSolves = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseCholesky3(QObject* parent = nullptr);

    /** @brief 对稀疏SPD矩阵进行分解 */
    bool factorize(const QVector<QVector<double>>& matrix);

    /** @brief 求解线性系统 Ax=b */
    QVector<double> solve(const QVector<double>& rhs) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int dimension, double nnzRatio);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QVector<double>> m_factor;
};
