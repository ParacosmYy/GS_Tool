/**
 * @file GaussSeidel4.h
 * @brief Gauss-Seidel迭代(多色排序+异步混沌迭代并行稀疏系统) — Gauss-Seidel with Multicolor Ordering and Asynchronous Chaotic Iteration for Parallel Sparse Systems
 *
 * 功能: 实现Gauss-Seidel迭代求解稀疏线性系统，支持多色排序(multicolor ordering)
 *       实现颜色内并行化，以及异步混沌迭代(asynchronous chaotic iteration)模式。
 *
 * 协作: GaussSeidel3(GS迭代) / SparseMatrix4(稀疏矩阵) / ConjugateGradient5(共轭梯度)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Gauss-Seidel迭代(多色排序+异步混沌)
 */
class GaussSeidel4 : public QObject {
    Q_OBJECT

public:
    /** @brief Iteration mode */
    enum Mode {
        Standard,        // Classic Gauss-Seidel
        Multicolor,      // Multicolor ordering parallel
        Chaotic          // Asynchronous chaotic iteration
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numNonZeros = 0;
        int iterationsUsed = 0;
        double finalResidual = 0.0;
        int numColors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussSeidel4(QObject *parent = nullptr);
    ~GaussSeidel4() override;

    /** @brief Set solver parameters */
    void setParameters(int maxIterations = 1000,
                       double tolerance = 1e-10, Mode mode = Multicolor);

    /** @brief Set sparse system A*x = b (CSR format) */
    void setSystem(int n,
                   const QVector<double>& values,
                   const QVector<int>& colIndices,
                   const QVector<int>& rowPtr,
                   const QVector<double>& rhs);

    /** @brief Solve the system, return solution vector */
    QVector<double> solve();

    /** @brief Compute residual ||Ax - b|| */
    double residual(const QVector<double>& x) const;

    /** @brief Perform multicolor graph coloring of matrix */
    QVector<int> computeColoring() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double residual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tolerance = 1e-10;
    Mode m_mode = Multicolor;

    // CSR sparse matrix
    int m_n = 0;
    QVector<double> m_values;
    QVector<int> m_colIdx;
    QVector<int> m_rowPtr;
    QVector<double> m_rhs;

    // Coloring
    QVector<int> m_colorOf;
    int m_numColors = 0;
    QVector<QVector<int>> m_colorGroups;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Standard Gauss-Seidel sweep */
    void sweepStandard(QVector<double>& x);

    /** @brief Multicolor parallel sweep */
    void sweepMulticolor(QVector<double>& x);

    /** @brief Chaotic asynchronous sweep */
    void sweepChaotic(QVector<double>& x);
};
