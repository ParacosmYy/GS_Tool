#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class TridiagonalSolver3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalSystemsSize = 0; double avgProcessingTimeMs = 0.0; };
    explicit TridiagonalSolver3(QObject* parent = nullptr);
    QVector<double> solve(const QVector<double>& lower,
                           const QVector<double>& main,
                           const QVector<double>& upper,
                           const QVector<double>& rhs);
    QVector<QVector<double>> solveCyclic(const QVector<double>& lower,
                                          const QVector<double>& main,
                                          const QVector<double>& upper,
                                          const QVector<double>& rhs,
                                          double cornerTL, double cornerBR);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int n);
private:
    Stats m_stats; double m_timeSum = 0.0;
};
