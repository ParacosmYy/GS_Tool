/**
 * @file BiCGSTAB15.h
 * @brief BiCGSTAB求解器(可变预条件器切换与残差平滑的双共轭梯度稳定收敛) — BiCGSTAB with Variable Preconditioner Switching and Residual Smoothing for Stabilized Biconjugate Gradient Convergence
 *
 * 功能: 实现BiCGSTAB求解器(BiCGSTAB)，采用可变预条件器切换(variable preconditioner switching)
 *       与残差平滑(residual smoothing)实现双共轭梯度稳定收敛(stabilized biconjugate gradient convergence)。
 *
 * 协作: CGSolver13(共轭梯度) / GMRES14(GMRES) / SparseMatrix12(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BiCGSTAB求解器(可变预条件器切换与残差平滑的双共轭梯度稳定收敛)
 */
class BiCGSTAB15 : public QObject {
    Q_OBJECT

public:
    /** @brief Preconditioner type */
    enum class PrecondType {
        None = 0,
        Jacobi,
        ILU0,
        SGSType
    };

    /** @brief Sparse matrix entry (COO format) */
    struct SparseEntry {
        int row = 0;
        int col = 0;
        double value = 0.0;
    };

    /** @brief Solver result */
    struct SolveResult {
        QVector<double> x;
        double residualNorm = 0.0;
        double initialResidual = 0.0;
        int iterations = 0;
        bool converged = false;
        PrecondType usedPrecond = PrecondType::None;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int nonZeros = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB15(QObject *parent = nullptr);
    ~BiCGSTAB15() override;

    void setMaxIter(int iters);
    void setTolerance(double tol);
    void setPrecond(PrecondType type);

    /** @brief Set matrix in dense format (n x n) */
    void setMatrix(const QVector<QVector<double>>& mat);

    /** @brief Set matrix in sparse COO format */
    void setMatrixSparse(const QVector<SparseEntry>& entries, int n);

    /** @brief Solve Ax = b */
    SolveResult solve(const QVector<double>& b);

    /** @brief Solve with given initial guess */
    SolveResult solveWithGuess(const QVector<double>& b, const QVector<double>& x0);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int iters, double residual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-10;
    PrecondType m_precond = PrecondType::ILU0;
    Stats m_stats;
    double m_timeSum = 0.0;

    int m_n = 0;
    QVector<QVector<double>> m_mat;        // Dense storage

    // ILU0 factorization storage
    QVector<QVector<double>> m_iluL;
    QVector<QVector<double>> m_iluU;
    bool m_iluReady = false;

    /** @brief Matrix-vector product y = A*x */
    QVector<double> matVec(const QVector<double>& x) const;

    /** @brief Dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;

    /** @brief Apply preconditioner: solve Mz = r */
    QVector<double> applyPrecond(const QVector<double>& r) const;

    /** @brief Build ILU(0) factorization */
    void buildILU();

    /** @brief Jacobi preconditioner */
    QVector<double> jacobiSolve(const QVector<double>& r) const;

    /** @brief ILU forward-backward solve */
    QVector<double> iluSolve(const QVector<double>& r) const;

    /** @brief Select best preconditioner based on residual stagnation */
    PrecondType autoSwitch(const QVector<double>& residualHistory) const;
};
