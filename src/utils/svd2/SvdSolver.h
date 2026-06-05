/**
 * @file SvdSolverV2.h
 * @brief SVD求解器 — Golub-Kahan双对角化 + 隐式QR
 *
 * 功能: 对矩阵进行奇异值分解，计算全部奇异值和右奇异向量V^T，
 *       基于Householder双对角化 + 隐式QR位移迭代，统计分解次数与平均耗时。
 *
 * 协作: QrDecomposition(QR分解) / SymmetricEigenSolver(特征值)
 */
#ifndef SVDSOLVER2_H
#define SVDSOLVER2_H

#include <QObject>
#include <QVector>

class SvdSolverV2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDecompositions = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit SvdSolverV2(QObject* parent = nullptr);

    /**
     * @brief 对矩阵进行SVD分解
     * @param matA 矩阵(m×n)
     * @return {奇异值数组(长度min(m,n)), V^T矩阵(n×n)}
     */
    QPair<QVector<double>, QVector<QVector<double>>>
        decompose(const QVector<QVector<double>>& matA);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 分解完成信号 @param sz min(rows, cols) */
    void decompositionCompleted(int sz);

private:
    Stats  m_stats;
    double m_timeSum;
};

#endif // SVDSOLVER2_H
