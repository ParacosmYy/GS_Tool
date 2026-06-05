/**
 * @file SparseCholesky2.h
 * @brief 稀疏Cholesky分解 — 符号分解/列消元/填充减少/重排序
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
class SparseCholesky2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int totalSolves = 0; double avgProcessingTimeMs = 0.0; };
    explicit SparseCholesky2(QObject* parent = nullptr);
    void buildFromCOO(const QVector<int>& rows, const QVector<int>& cols, const QVector<double>& vals, int n);
    bool decompose();
    QVector<double> solve(const QVector<double>& rhs) const;
    double logDeterminant() const;
    bool isPositiveDefinite() const;
    QVector<int> permutation() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionComplete(int n, bool success);
private:
    void symbolicDecompose();
    void applyPermutation();
    int m_n = 0; bool m_factored = false; bool m_spd = false;
    QVector<int> m_perm; QVector<int> m_invPerm;
    QVector<int> m_colPtr; QVector<int> m_rowIdx; QVector<double> m_values;
    double m_logDet = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
