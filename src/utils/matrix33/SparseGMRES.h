/**
 * @file SparseGMRES.h
 * @brief 稀疏GMRES求解器 — Krylov子空间/Arnoldi/重启GMRES/预条件
 */
#pragma once
#include <QObject>
#include <QVector>
class SparseGMRES : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalIterations = 0; double avgProcessingTimeMs = 0.0; };
    explicit SparseGMRES(QObject* parent = nullptr);
    void buildFromCOO(const QVector<int>& rows, const QVector<int>& cols, const QVector<double>& vals, int n);
    void setRestart(int m);
    void setTolerance(double tol);
    void setMaxIterations(int maxIter);
    QVector<double> solve(const QVector<double>& rhs);
    QVector<double> solveWithPrec(const QVector<double>& rhs, const QVector<double>& precDiag);
    int iterations() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveComplete(bool converged, int iterations);
private:
    QVector<double> spmv(const QVector<double>& x) const;
    void arnoldi(QVector<QVector<double>>& V, QVector<double>& h, int j, const QVector<double>& rhs);
    int m_n = 0; int m_restart = 30;
    double m_tol = 1e-8; int m_maxIter = 1000;
    int m_lastIter = 0;
    QVector<int> m_rowPtr, m_colIdx;
    QVector<double> m_values;
    Stats m_stats; double m_timeSum = 0.0;
};
