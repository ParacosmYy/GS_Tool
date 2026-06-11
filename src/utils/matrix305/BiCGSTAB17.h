/**
 * @file BiCGSTAB17.h
 * @brief BiCGSTAB求解器(右预条件与灵活GMRES内求解器实现变预条件稳定双共轭梯度) — BiCGSTAB with Right Preconditioning and Flexible GMRES Inner Solver for Variable-Preconditioner Stabilized Biconjugate Gradient
 *
 * 功能: 实现BiCGSTAB求解器(BiCGSTAB solver)，采用右预条件(right preconditioning)
 *       与灵活GMRES内求解器(flexible GMRES inner solver)实现变预条件稳定双共轭梯度(variable-preconditioner stabilized biconjugate gradient)。
 *
 * 协作: ConjugateGradient(共轭梯度) / GMRES(广义最小残差) / SparseMatrix(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

class BiCGSTAB17 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse matrix entry (COO format) */
    struct SparseEntry {
        int row = 0;
        int col = 0;
        double value = 0.0;
    };

    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        int iterations = 0;
        double residualNorm = 0.0;
        double initialResidual = 0.0;
        bool converged = false;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int maxIterationsUsed = 0;
        double avgConvergenceRate = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB17(QObject *parent = nullptr);
    ~BiCGSTAB17() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setPreconditionerType(int type);     // 0=None, 1=Jacobi, 2=ILU0

    /** @brief Solve Ax = b with flexible BiCGSTAB */
    SolveResult solve(int n, const QVector<SparseEntry>& entries,
                      const QVector<double>& rhs);

    /** @brief Matrix-vector product y = A*x */
    QVector<double> matvec(int n, const QVector<SparseEntry>& entries,
                           const QVector<double>& x) const;

    /** @brief Apply preconditioner M^{-1} * r */
    QVector<double> precondition(const QVector<double>& r) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int iterations, double residual, bool converged, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-10;
    int m_pcType = 1;               // Preconditioner type
    int m_n = 0;
    QVector<SparseEntry> m_entries;
    QVector<double> m_diagInv;       // Jacobi: 1/diag(A)
    QVector<QVector<SparseEntry>> m_rows;  // ILU rows
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Jacobi preconditioner */
    void buildJacobi(int n, const QVector<SparseEntry>& entries);

    /** @brief Build ILU(0) preconditioner */
    void buildILU0(int n, const QVector<SparseEntry>& entries);

    /** @brief ILU forward-backward solve */
    QVector<double> iluSolve(const QVector<double>& r) const;

    /** @brief Dot product of two vectors */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;

    /** @brief Flexible GMRES inner solve (few iterations for variable preconditioner) */
    QVector<double> flexibleGMRESInner(const QVector<double>& b, int innerIter) const;
};
