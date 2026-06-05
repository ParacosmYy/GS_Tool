/**
 * @file SVD3.h
 * @brief SVD分解增强 — 双边Jacobi/Golub-Kahan/截断SVD/低秩近似
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class SVD3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalDecompositions = 0;
        int totalSolves = 0;
        double avgProcessingTimeMs = 0.0;
    };
    explicit SVD3(QObject* parent = nullptr);
    void decompose(const QVector<double>& matrix, int rows, int cols);
    QVector<double> solve(const QVector<double>& b) const;
    QVector<double> pseudoInverse() const;
    QVector<double> truncatedSVD(int rank) const;
    double conditionNumber() const;
    int rank(double tol = 1e-10) const;
    QVector<double> singularValues() const { return m_sigma; }
    QVector<double> matrixU() const { return m_U; }
    QVector<double> matrixV() const { return m_V; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionComplete(int rows, int cols, int rank);
private:
    void bidiagonalize(QVector<double>& A, int m, int n,
                       QVector<double>& U, QVector<double>& V);
    void golubKahanStep(QVector<double>& d, QVector<double>& e,
                        QVector<double>& U, QVector<double>& V,
                        int m, int n);
    int m_rows = 0, m_cols = 0;
    QVector<double> m_U, m_sigma, m_V;
    Stats m_stats;
    double m_timeSum = 0.0;
};
