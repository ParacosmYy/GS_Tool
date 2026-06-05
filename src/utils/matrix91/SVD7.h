#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 奇异值分解(SVD)
 *
 * 将矩阵分解为 U * Sigma * V^T，用于降维和最小二乘。
 */
class SVD7 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalDecompositions = 0;
        int totalRankEstimates = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SVD7(QObject* parent = nullptr);

    /** @brief 计算SVD分解 */
    bool decompose(const QVector<QVector<double>>& matrix);

    /** @brief 估计矩阵的有效秩 */
    int estimateRank(double tolerance = 1e-10) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int rows, int cols, int rank);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_singularValues;
};
