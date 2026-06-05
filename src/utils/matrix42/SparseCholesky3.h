/**
 * @file SparseCholesky3.h
 * @brief 稀疏Cholesky分解3 — 填充缩减+多波前
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class SparseCholesky3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalFactorizations = 0;
        int totalSolves = 0;
        int totalNonzeros = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseCholesky3(QObject* parent = nullptr);

    void setMatrix(int n, const QVector<int>& rowPtr,
                   const QVector<int>& colIdx,
                   const QVector<double>& values);
    bool factorize();
    QVector<double> solve(const QVector<double>& rhs);

    int nnz() const { return m_totalNnz; }
    double fillRatio() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int n, int nnz, double fillRatio);

private:
    int m_n = 0;
    QVector<int> m_rowPtr;
    QVector<int> m_colIdx;
    QVector<double> m_values;
    QVector<int> m_perm;
    QVector<int> m_invPerm;
    int m_totalNnz = 0;
    bool m_factored = false;

    void amdOrdering();
    void symbolicFactorize();

    Stats m_stats;
    double m_timeSum = 0.0;
};
