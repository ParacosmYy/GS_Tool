/**
 * @file SparseCholesky4.h
 * @brief 稀疏Cholesky4 — 块稀疏+多线程分解
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class SparseCholesky4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalFactorizations = 0;
        int totalSolves = 0;
        int totalNonzeros = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseCholesky4(QObject* parent = nullptr);

    void setMatrix(int n, const QVector<int>& rowPtr,
                   const QVector<int>& colIdx,
                   const QVector<double>& values);
    void setBlocksize(int blocksize);
    bool factorize();
    QVector<double> solve(const QVector<double>& rhs);

    int nnz() const { return m_nnz; }
    double fillRatio() const;
    int supernodes() const { return m_supernodes; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int n, int supernodes, double fillRatio);

private:
    int m_n = 0;
    int m_blocksize = 64;
    int m_nnz = 0;
    int m_supernodes = 0;
    QVector<int> m_rowPtr;
    QVector<int> m_colIdx;
    QVector<double> m_values;
    QVector<int> m_perm;
    QVector<double> m_factor;
    bool m_factored = false;

    void supernodalAnalysis();
    void blockFactorize();

    Stats m_stats;
    double m_timeSum = 0.0;
};
