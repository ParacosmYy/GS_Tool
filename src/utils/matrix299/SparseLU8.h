/**
 * @file SparseLU8.h
 * @brief 稀疏LU分解(填充消减列排序与符号数值分解实现大规模稀疏系统高效直接求解) — Sparse LU with Fill-reducing Column Ordering and Symbolic-numeric Factorization for Efficient Direct Solution of Large Sparse Systems
 *
 * 功能: 实现稀疏LU分解(sparse LU)，采用填充消减列排序(fill-reducing column ordering)
 *       与符号数值分解(symbolic-numeric factorization)实现大规模稀疏系统高效直接求解(efficient direct solution of large sparse systems)。
 *
 * 协作: SparseMatrix7(稀疏矩阵) / ConjugateGradient8(共轭梯度) / DenseLU6(稠密LU)
 */
#pragma once

#include <QObject>
#include <QVector>

class SparseLU8 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse entry (row, value) */
    struct SparseEntry {
        int row = -1;
        double value = 0.0;
    };

    /** @brief LU factorization result */
    struct FactorResult {
        QVector<QVector<SparseEntry>> L;
        QVector<QVector<SparseEntry>> U;
        QVector<int> rowPermutation;
        QVector<int> colPermutation;
        double determinant = 0.0;
        int fillCount = 0;
        bool success = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int nonZeros = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseLU8(QObject *parent = nullptr);
    ~SparseLU8() override;

    /** @brief Set matrix from COO format and factorize */
    FactorResult factorize(int n,
                            const QVector<int>& rows,
                            const QVector<int>& cols,
                            const QVector<double>& values);

    /** @brief Solve Ax=b after factorization */
    QVector<double> solve(const FactorResult& factors,
                           const QVector<double>& b) const;

    /** @brief Fill-reducing column ordering (approximate minimum degree) */
    QVector<int> columnOrdering(int n,
                                 const QVector<int>& rows,
                                 const QVector<int>& cols) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorDone(int n, int fill, double det, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build column-offset (CSC) from COO */
    void buildCSC(int n, const QVector<int>& rows, const QVector<int>& cols,
                   const QVector<double>& values,
                   QVector<int>& colPtr, QVector<int>& rowIdx,
                   QVector<double>& vals) const;

    /** @brief Symbolic factorization: determine fill pattern */
    QVector<QVector<int>> symbolicFactor(int n,
                                           const QVector<int>& colPtr,
                                           const QVector<int>& rowIdx,
                                           const QVector<int>& colPerm) const;

    /** @brief Sparse triangular solve (Lx=b) */
    QVector<double> forwardSolve(const QVector<QVector<SparseEntry>>& L,
                                  const QVector<double>& b) const;

    /** @brief Sparse triangular solve (Ux=b) */
    QVector<double> backwardSolve(const QVector<QVector<SparseEntry>>& U,
                                   const QVector<double>& b) const;
};
