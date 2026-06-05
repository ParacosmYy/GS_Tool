/**
 * @file SVD4.h
 * @brief SVD4 — 双边Jacobi+条件数估计
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class SVD4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalDecompositions = 0;
        int totalSolves = 0;
        int matrixRows = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SVD4(QObject* parent = nullptr);

    bool decompose(const QVector<double>& A, int rows, int cols);
    QVector<double> singularValues() const { return m_sigma; }
    QVector<double> matrixU() const { return m_U; }
    QVector<double> matrixV() const { return m_V; }
    int rank(double tol = 1e-10) const;

    QVector<double> solve(const QVector<double>& b) const;
    double conditionNumber() const;
    QVector<double> pseudoInverse() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int rows, int cols, int rank);

private:
    int m_rows = 0;
    int m_cols = 0;
    QVector<double> m_U;
    QVector<double> m_sigma;
    QVector<double> m_V;

    void jacobiSweep(QVector<double>& S, QVector<double>& U,
                     QVector<double>& V, int m, int n);
    void rotate(QVector<double>& S, QVector<double>& U,
                QVector<double>& V, int p, int q, int m, int n);

    Stats m_stats;
    double m_timeSum = 0.0;
};
