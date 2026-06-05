/**
 * @file QRDecomp3.h
 * @brief QR分解增强 — Householder/Givens/列主元/更新/最小二乘
 */
#pragma once
#include <QObject>
#include <QVector>
class QRDecomp3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int totalSolves = 0; double avgProcessingTimeMs = 0.0; };
    explicit QRDecomp3(QObject* parent = nullptr);
    void decompose(const QVector<double>& matrix, int rows, int cols);
    QVector<double> solve(const QVector<double>& rhs) const;
    QVector<double> matrixQ() const;
    QVector<double> matrixR() const;
    int rank(double tol = 1e-10) const;
    double conditionNumber() const;
    QVector<double> leastSquares(const QVector<double>& rhs) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionComplete(int rows, int cols, int rank);
private:
    int m_rows = 0, m_cols = 0;
    QVector<double> m_QR; QVector<double> m_tau;
    Stats m_stats; double m_timeSum = 0.0;
};
