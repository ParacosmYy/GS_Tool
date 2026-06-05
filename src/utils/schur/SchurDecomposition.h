/**
 * @file SchurDecomposition.h
 * @brief 实 Schur 分解 — QR 迭代
 *
 * 功能: 将实方阵 A 分解为 A = Q T Q^T，其中 Q 正交、T 拟上三角。
 *       T 的对角块为 1×1(实特征值) 或 2×2(共轭复特征值对)。
 *
 * 协作: EigenSolver(特征值) / HessenbergReduction(预处理)
 */
#ifndef SCHURDECOMPOSITION_H
#define SCHURDECOMPOSITION_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 实 Schur 分解器
 */
class SchurDecomposition : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDecompositions = 0;  ///< 累计分解次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SchurDecomposition(QObject* parent = nullptr);

    /**
     * @brief 对实方阵执行 Schur 分解
     * @param A 输入方阵(n×n)
     * @param maxIter 最大 QR 迭代次数
     * @return QPair(正交阵 Q, 拟上三角阵 T)
     */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>>
    decompose(QVector<QVector<double>> A, int maxIter = 500);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成 @param size 矩阵阶数 @param iterations 实际迭代次数 */
    void decompositionCompleted(int size, int iterations);

private:
    Stats  m_stats;          ///< 统计信息
    double m_timeSum = 0.0;  ///< 累计耗时

    /** @brief 2×2 子块特征值判定阈值 */
    static constexpr double kEpsilon = 1e-12;
};

#endif // SCHURDECOMPOSITION_H
