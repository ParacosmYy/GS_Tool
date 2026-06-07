/**
 * @file ConjugateGradient4.h
 * @brief 预条件共轭梯度(不完全Cholesky分解+Eisenstat技巧快速应用) — Preconditioned Conjugate Gradient with Incomplete Cholesky and Eisenstat Trick for Fast Applies
 *
 * 功能: 实现预条件共轭梯度(PCG)求解器，支持不完全Cholesky预条件、
 *       Eisenstat技巧加速和稀疏矩阵向量乘优化。
 *
 * 协作: SparseMatrix6(稀疏矩阵) / GaussSeidel5(迭代求解) / LDLDecomposition4(LDL分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 预条件共轭梯度(不完全Cholesky分解+Eisenstat技巧快速应用)
 */
class ConjugateGradient4 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse matrix in CSR format */
    struct SparseMatrix {
        int n = 0;
        QVector<int> rowPtr;
        QVector<int> colIdx;
        QVector<double> values;

        /** @brief Matrix-vector product */
        QVector<double> multiply(const QVector<double>& x) const;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterations = 0;
        double finalResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConjugateGradient4(QObject *parent = nullptr);
    ~ConjugateGradient4() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);

    /** @brief Build sparse matrix from triplet data */
    void setMatrix(int n, const QVector<int>& rows, const QVector<int>& cols,
                    const QVector<double>& vals);

    /** @brief Compute incomplete Cholesky factorization L */
    void computePreconditioner();

    /** @brief Solve Ax = b using PCG with Eisenstat trick */
    QVector<double> solve(const QVector<double>& b);

    /** @brief Forward/backward substitution with L */
    QVector<double> preconditionSolve(const QVector<double>& r) const;

    /** @brief Get residual history */
    QVector<double> getResidualHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, int iterations, double residual, double timeMs);

private:
    SparseMatrix m_A;
    int m_maxIter = 1000;
    double m_tol = 1e-8;

    /** @brief Incomplete Cholesky factor L (lower triangular, CSR) */
    SparseMatrix m_L;
    QVector<double> m_diagL;

    /** @brief Eisenstat scaling diagonal D such that A ≈ L D L^T */
    QVector<double> m_eisenstatD;

    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_residualHistory;

    /** @brief Forward substitution: L y = r */
    QVector<double> forwardSub(const QVector<double>& r) const;

    /** @brief Backward substitution: L^T x = y */
    QVector<double> backwardSub(const QVector<double>& y) const;

    /** @brief Eisenstat-transformed matrix-vector product */
    QVector<double> eisenstatMultiply(const QVector<double>& x,
                                       const QVector<double>& b) const;

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);
};
