/**
 * @file TridiagonalSolver3.h
 * @brief 三对角求解器(循环约化并行对角优势+Sherman-Morrison环绕修正) — Tridiagonal Solver with Cyclic Reduction for Parallel Diagonal Dominance and Sherman-Morrison Wrap-Around Correction
 *
 * 功能: 实现三对角线性系统求解器，支持循环约化并行消元、
 *       Sherman-Morrison周期边界修正和Thomas串行算法。
 *
 * 协作: SORSolver2(SOR迭代) / ConjugateGradient4(共轭梯度) / MatrixDecomp5(矩阵分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 三对角求解器(循环约化+Sherman-Morrison)
 */
class TridiagonalSolver3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int systemSize = 0;
        int method = 0;          // 0=Thomas, 1=CyclicReduction
        double residual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TridiagonalSolver3(QObject *parent = nullptr);
    ~TridiagonalSolver3() override;

    /** @brief Solve tridiagonal system via Thomas algorithm */
    QVector<double> thomasSolve(const QVector<double>& lower,
                                const QVector<double>& diag,
                                const QVector<double>& upper,
                                const QVector<double>& rhs);

    /** @brief Solve via cyclic reduction (parallelizable) */
    QVector<double> cyclicReduction(const QVector<double>& lower,
                                     const QVector<double>& diag,
                                     const QVector<double>& upper,
                                     const QVector<double>& rhs);

    /** @brief Solve periodic tridiagonal via Sherman-Morrison */
    QVector<double> periodicSolve(const QVector<double>& lower,
                                   const QVector<double>& diag,
                                   const QVector<double>& upper,
                                   const QVector<double>& rhs,
                                   double wrapLower = 0.0,
                                   double wrapUpper = 0.0);

    /** @brief Compute residual ||Ax - b|| */
    double residual(const QVector<double>& lower,
                    const QVector<double>& diag,
                    const QVector<double>& upper,
                    const QVector<double>& rhs,
                    const QVector<double>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, int method, double residual, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Internal Thomas solve modifying input arrays */
    void thomasInternal(QVector<double>& a, QVector<double>& b,
                        QVector<double>& c, QVector<double>& d);
};
