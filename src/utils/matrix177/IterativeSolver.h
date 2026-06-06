/**
 * @file IterativeSolver.h
 * @brief 迭代线性求解器集(Jacobi/Gauss-Seidel/SOR+收敛监控) — Iterative Linear Solver Collection: Jacobi, Gauss-Seidel, SOR with Convergence Monitoring
 *
 * 功能: 实现迭代线性方程组求解器集，支持Jacobi、Gauss-Seidel、SOR方法、
 *       收敛监控、残差追踪。
 *
 * 协作: IterativeRefinement3(迭代精化) / ConjugateGradient5(共轭梯度) / SparseSolver4(稀疏求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 迭代线性求解器
 */
class IterativeSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 求解方法 */
    enum Method { Jacobi = 0, GaussSeidel = 1, SOR = 2 };

    /** @brief 收敛信息 */
    struct ConvergenceInfo {
        int iterations = 0;           ///< 实际迭代次数
        double finalResidual = 0.0;   ///< 最终残差
        bool converged = false;       ///< 是否收敛
        QVector<double> residualHistory; ///< 残差历史
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;          ///< 累计求解次数
        int totalIterations = 0;          ///< 累计迭代次数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        int lastN = 0;                    ///< 最近矩阵维度
    };

    explicit IterativeSolver(QObject *parent = nullptr);
    ~IterativeSolver() override;

    void setMethod(Method method);
    void setMaxIterations(int maxIter);
    void setTolerance(double tol);
    void setRelaxation(double omega);

    /**
     * @brief 求解 Ax = b
     * @param A 系数矩阵(n x n)
     * @param b 右端向量(n)
     * @param x0 初始猜测(n)，空则用零向量
     * @return 解向量
     */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b,
                          const QVector<double>& x0 = {});

    /** @brief 获取最近收敛信息 */
    const ConvergenceInfo& lastConvergence() const { return m_conv; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, bool converged);
    void iterationProgress(int iter, double residual);

private:
    /** @brief Jacobi迭代 */
    QVector<double> solveJacobi(const QVector<QVector<double>>& A,
                                 const QVector<double>& b,
                                 const QVector<double>& x0);

    /** @brief Gauss-Seidel迭代 */
    QVector<double> solveGaussSeidel(const QVector<QVector<double>>& A,
                                      const QVector<double>& b,
                                      const QVector<double>& x0);

    /** @brief SOR迭代 */
    QVector<double> solveSOR(const QVector<QVector<double>>& A,
                              const QVector<double>& b,
                              const QVector<double>& x0);

    /** @brief 计算残差 ||Ax - b||_inf */
    static double residualNorm(const QVector<QVector<double>>& A,
                                const QVector<double>& b,
                                const QVector<double>& x);

    Method m_method = GaussSeidel;
    int m_maxIter = 1000;
    double m_tol = 1e-10;
    double m_omega = 1.5; ///< SOR松弛因子

    ConvergenceInfo m_conv;
    Stats m_stats;
    double m_timeSum = 0.0;
};
