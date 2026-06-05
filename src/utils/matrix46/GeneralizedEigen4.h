/**
 * @file GeneralizedEigen4.h
 * @brief 广义特征值4 — QZ迭代+正定预条件
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class GeneralizedEigen4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalDecompositions = 0;
        int totalSolves = 0;
        int matrixSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GeneralizedEigen4(QObject* parent = nullptr);

    bool decompose(const QVector<double>& A, const QVector<double>& B, int n);
    QVector<QPair<double,double>> eigenvalues() const;
    QVector<double> eigenvectors() const { return m_V; }

    QVector<double> solveGEVP(const QVector<double>& C) const;
    bool isRegular() const;
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int n, bool converged);

private:
    int m_n = 0;
    QVector<double> m_S;
    QVector<double> m_T;
    QVector<double> m_Q;
    QVector<double> m_Z;
    QVector<double> m_V;
    bool m_decomposed = false;

    void qzIteration(QVector<double>& S, QVector<double>& T,
                     QVector<double>& Q, QVector<double>& Z, int n);

    Stats m_stats;
    double m_timeSum = 0.0;
};
