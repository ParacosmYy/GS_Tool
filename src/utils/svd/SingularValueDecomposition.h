/**
 * @file SingularValueDecomposition.h
 * @brief SVD奇异值分解 — 矩阵分解与降维
 *
 * 功能: 实现SVD奇异值分解(A=UΣV^T)，支持低秩近似、
 *       伪逆计算、条件数估计，统计分解次数/耗时。
 */
#ifndef SINGULARVALUEDECOMPOSITION_H
#define SINGULARVALUEDECOMPOSITION_H

#include <QObject>
#include <QVector>

class SingularValueDecomposition : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalDecompositions = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit SingularValueDecomposition(QObject* parent = nullptr);

    /** @brief SVD分解 @param A 矩阵 @return {U, singularValues, V} */
    struct SVDResult {
        QVector<QVector<double>> U;
        QVector<double> singularValues;
        QVector<QVector<double>> V;
    };

    SVDResult decompose(const QVector<QVector<double>>& A);

    /** @brief 低秩近似 @param A 矩阵 @param rank 目标秩 @return 近似矩阵 */
    QVector<QVector<double>> lowRankApproximation(
        const QVector<QVector<double>>& A, int rank);

    /** @brief 伪逆 @param A 矩阵 @return 伪逆矩阵 */
    QVector<QVector<double>> pseudoInverse(
        const QVector<QVector<double>>& A);

    /** @brief 条件数 @param A 矩阵 @return 条件数 */
    double conditionNumber(const QVector<QVector<double>>& A);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int rows, int cols, int rank);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // SINGULARVALUEDECOMPOSITION_H
