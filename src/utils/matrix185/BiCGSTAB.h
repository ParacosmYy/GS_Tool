/**
 * @file BiCGSTAB.h
 * @brief BiCGSTAB迭代求解器(ILU(0)预处理+非对称稀疏系统) — BiCGSTAB Iterative Solver with ILU(0) Preconditioner for Non-symmetric Sparse Systems
 *
 * 功能: 实现BiCGSTAB稳定化双共轭梯度法，支持ILU(0)不完全LU预处理、
 *       非对称稀疏矩阵系统求解和收敛监控。
 *
 * 协作: CGSolver6(CG迭代) / GaussSeidel7(Gauss-Seidel) / SparseMatrix8(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BiCGSTAB迭代求解器(ILU(0)预处理)
 */
class BiCGSTAB : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int numNonZeros = 0;
        int numIterations = 0;
        double finalResidual = 0.0;
        bool converged = false;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Sparse matrix entry (COO format) */
    struct SparseEntry {
        int row = 0;
        int col = 0;
        double value = 0.0;
    };

    explicit BiCGSTAB(QObject *parent = nullptr);
    ~BiCGSTAB() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setILUEnabled(bool enabled);

    /** @brief 设置稀疏矩阵(COO格式)并转换内部存储 */
    void setMatrix(int n, const QVector<SparseEntry>& entries);

    /** @brief 求解Ax=b */
    QVector<double> solve(const QVector<double>& b);

    /** @brief ILU(0)不完全分解 */
    void computeILU0();

    /** @brief ILU前代/回代 */
    QVector<double> iluSolve(const QVector<double>& r) const;

    /** @brief 计算残差 ||b - Ax|| */
    double residual(const QVector<double>& x, const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, bool converged);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-8;
    bool m_iluEnabled = true;
    int m_n = 0;

    /** @brief CSR sparse matrix storage */
    QVector<double> m_values;    ///< Non-zero values
    QVector<int> m_colIdx;       ///< Column indices
    QVector<int> m_rowPtr;       ///< Row pointers (size n+1)

    /** @brief ILU factors */
    QVector<double> m_luValues;  ///< ILU(0) L+U combined values
    QVector<int> m_luColIdx;
    QVector<int> m_luRowPtr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sparse matrix-vector multiply */
    QVector<double> spmv(const QVector<double>& x) const;

    /** @brief Dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector operations */
    QVector<double> vecAdd(const QVector<double>& a, const QVector<double>& b) const;
    QVector<double> vecSub(const QVector<double>& a, const QVector<double>& b) const;
    QVector<double> vecScale(const QVector<double>& v, double s) const;
    double vecNorm(const QVector<double>& v) const;
};
