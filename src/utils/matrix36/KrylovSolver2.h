/**
 * @file KrylovSolver2.h
 * @brief Krylov solver enhanced - BiCGSTAB/CGS/IDR(s)/deflation
 */
#pragma once
#include <QObject>
#include <QVector>
class KrylovSolver2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalIterations = 0; double avgProcessingTimeMs = 0.0; };
    explicit KrylovSolver2(QObject* parent = nullptr);
    void buildFromCOO(const QVector<int>& rows, const QVector<int>& cols,
                      const QVector<double>& vals, int n);
    void setTolerance(double tol);
    void setMaxIterations(int maxIter);
    QVector<double> solveCG(const QVector<double>& rhs);
    QVector<double> solveBiCGSTAB(const QVector<double>& rhs);
    QVector<double> solveCGS(const QVector<double>& rhs);
    int lastIterations() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveComplete(bool converged, int iterations);
private:
    QVector<double> spmv(const QVector<double>& x) const;
    double dot(const QVector<double>& a, const QVector<double>& b) const;
    void updateStats(double elapsedMs);
    int m_n = 0; double m_tol = 1e-8; int m_maxIter = 1000; int m_lastIter = 0;
    QVector<int> m_rowPtr, m_colIdx; QVector<double> m_values;
    Stats m_stats; double m_timeSum = 0.0;
};
