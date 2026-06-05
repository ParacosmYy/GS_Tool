/**
 * @file SymmetricEigenSolver.h
 * @brief 对称矩阵特征值求解器 — Jacobi旋转法
 *
 * 功能: 求解对称矩阵的全部特征值和特征向量，基于经典Jacobi旋转迭代，
 *       适用于中小规模稠密对称矩阵，统计求解次数与平均耗时。
 *
 * 协作: TridiagEigenSolver(三对角优化) / QrDecomposition(QR迭代)
 */
#ifndef SYMMETRICEIGENSOLVER_H
#define SYMMETRICEIGENSOLVER_H

#include <QObject>
#include <QVector>

class SymmetricEigenSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves    = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit SymmetricEigenSolver(QObject* parent = nullptr);

    /**
     * @brief 求解对称矩阵的全部特征值和特征向量
     * @param matA 对称矩阵(n×n)
     * @param maxIter 最大Jacobi扫描次数(默认1000)
     * @return {特征值数组, 特征向量矩阵(每列一个向量)}
     */
    QPair<QVector<double>, QVector<QVector<double>>>
        solve(const QVector<QVector<double>>& matA,
              int maxIter = 1000);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 @param sz 矩阵尺寸 @param iterations 迭代次数 */
    void solveCompleted(int sz, int iterations);

private:
    Stats  m_stats;
    double m_timeSum;
};

#endif // SYMMETRICEIGENSOLVER_H
