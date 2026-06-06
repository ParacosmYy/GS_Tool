/**
 * @file GaussSeidel.h
 * @brief Gauss-Seidel迭代求解器(逐次超松弛+红黑排序) — Gauss-Seidel Iterative Solver with Successive Over-Relaxation (SOR) and Red-Black Ordering
 *
 * 功能: 实现Gauss-Seidel迭代求解器，支持SOR加速、红黑排序、
 *       对角占优检测和收敛性监测。
 *
 * 协作: ConjugateGradient5(共轭梯度) / Jacobi3(Jacobi) / SparseLU4(Sparse LU)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Gauss-Seidel迭代求解器(SOR+红黑排序)
 */
class GaussSeidel : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterationsUsed = 0;
        double finalResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussSeidel(QObject *parent = nullptr);
    ~GaussSeidel() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setOmega(double omega);
    void setRedBlackOrdering(bool enabled);

    /** @brief 求解 Ax = b */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief 求解带红黑排序 */
    QVector<double> solveRedBlack(const QVector<QVector<double>>& A,
                                  const QVector<double>& b);

    /** @brief 检查对角占优 */
    bool isDiagonallyDominant(const QVector<QVector<double>>& A) const;

    /** @brief 计算残差 */
    double residual(const QVector<QVector<double>>& A,
                    const QVector<double>& x,
                    const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double finalResidual);

private:
    int m_maxIterations = 1000;
    double m_tolerance = 1e-10;
    double m_omega = 1.0; // SOR relaxation (1.0 = pure Gauss-Seidel)
    bool m_redBlack = true;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Standard Gauss-Seidel iteration */
    QVector<double> solveStandard(const QVector<QVector<double>>& A,
                                   const QVector<double>& b);

    /** @brief Build red-black color map for 2D grid */
    QVector<int> buildColorMap(int n) const;
};
