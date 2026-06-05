/**
 * @file SchurDecomp4.h
 * @brief Schur分解4 — 实Schur+特征值排序
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class SchurDecomp4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalDecompositions = 0;
        int totalSolves = 0;
        int matrixSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SchurDecomp4(QObject* parent = nullptr);

    bool decompose(const QVector<double>& A, int n);
    QVector<double> schurForm() const { return m_T; }
    QVector<double> schurVectors() const { return m_Q; }
    QVector<QPair<double,double>> eigenvalues() const;
    void sortByReal();
    void sortByMagnitude();

    QVector<double> solveSylvester(const QVector<double>& B, int m) const;
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int n, bool converged);

private:
    int m_n = 0;
    QVector<double> m_T;
    QVector<double> m_Q;
    bool m_decomposed = false;

    void qrStep(QVector<double>& H, QVector<double>& Q, int lo, int hi, int n);
    void francisDoubleShift(QVector<double>& H, QVector<double>& Q, int n);

    Stats m_stats;
    double m_timeSum = 0.0;
};
