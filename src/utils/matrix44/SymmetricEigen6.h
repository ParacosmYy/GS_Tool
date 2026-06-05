/**
 * @file SymmetricEigen6.h
 * @brief 对称特征值6 — 分治法+三对角化
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class SymmetricEigen6 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalDecompositions = 0;
        int totalEigenvectors = 0;
        int matrixSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SymmetricEigen6(QObject* parent = nullptr);

    bool decompose(const QVector<double>& A, int n);
    QVector<double> eigenvalues() const { return m_eigenvalues; }
    QVector<double> eigenvectors() const { return m_eigenvectors; }
    QVector<double> eigenvector(int index) const;

    int size() const { return m_n; }
    bool isPositiveDefinite() const;
    double conditionNumber() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int n, int iterations);

private:
    int m_n = 0;
    QVector<double> m_eigenvalues;
    QVector<double> m_eigenvectors;
    bool m_decomposed = false;

    void tridiagonalize(QVector<double>& mat, QVector<double>& diag,
                        QVector<double>& subdiag, int n);
    void divideConquer(QVector<double>& diag, QVector<double>& subdiag,
                       QVector<double>& Q, int n);
    void mergeEigenvalues(QVector<double>& d1, QVector<double>& d2,
                          double rho, const QVector<double>& Q1,
                          const QVector<double>& Q2, int n1, int n2,
                          QVector<double>& result,
                          QVector<double>& resultQ);

    Stats m_stats;
    double m_timeSum = 0.0;
};
