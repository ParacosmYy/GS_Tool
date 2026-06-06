/**
 * @file SparseSolver.h
 * @brief 稀疏线性系统求解器(共轭梯度+不完全Cholesky预处理) — Sparse Linear System Solver via Conjugate Gradient with Incomplete Cholesky Preconditioner
 *
 * 功能: 实现PCG(预处理共轭梯度)求解稀疏对称正定线性系统Ax=b。
 *       使用不完全Cholesky分解(IC(0))作为预处理子，加速收敛。
 *
 * 协作: SparseMatrix(稀疏矩阵) / GaussElimination(高斯消元) / ConjugateGradient(CG)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏线性系统求解器
 */
class SparseSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 稀疏矩阵(CSR格式) */
    struct SparseMatrix {
        int n;                              ///< 维度
        QVector<double> values;             ///< 非零值
        QVector<int> colIndices;            ///< 列索引
        QVector<int> rowPtr;                ///< 行指针

        SparseMatrix() : n(0) {}
    };

    /** @brief 求解结果 */
    struct SolveResult {
        QVector<double> x;                  ///< 解向量
        int iterations;                     ///< 迭代次数
        double residual;                    ///< 最终残差
        bool converged;                     ///< 是否收敛
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        int lastIterations = 0;             ///< 最近迭代次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit SparseSolver(QObject* parent = nullptr);
    ~SparseSolver() override;

    /** @brief 设置最大迭代次数 */
    void setMaxIterations(int maxIter);

    /** @brief 设置收敛阈值 */
    void setTolerance(double tol);

    /**
     * @brief 求解稀疏线性系统Ax=b
     * @param A 稀疏矩阵(CSR)
     * @param b 右端向量
     * @param x0 初始猜测(空则用零向量)
     * @return 求解结果
     */
    SolveResult solve(const SparseMatrix& A, const QVector<double>& b,
                      const QVector<double>& x0 = QVector<double>());

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param iter 迭代次数 @param residual 残差 */
    void solveCompleted(int iter, double residual);

private:
    /** @brief 不完全Cholesky分解IC(0) */
    void incompleteCholesky(const SparseMatrix& A, SparseMatrix& L) const;

    /** @brief CSR稀疏矩阵-向量乘 */
    static void spmv(const SparseMatrix& A, const QVector<double>& x,
                     QVector<double>& y);

    /** @brief 前代求解 L*y=r */
    void forwardSolve(const SparseMatrix& L, const QVector<double>& r,
                      QVector<double>& y) const;

    /** @brief 回代求解 L^T*x=y */
    void backwardSolve(const SparseMatrix& L, const QVector<double>& y,
                       QVector<double>& x) const;

    int m_maxIterations = 1000;
    double m_tolerance = 1e-8;

    Stats m_stats;
    double m_timeSum = 0.0;
};
