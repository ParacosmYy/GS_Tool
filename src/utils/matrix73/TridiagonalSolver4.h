#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class TridiagonalSolver4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalSystems = 0; double avgProcessingTimeMs = 0.0; };
    explicit TridiagonalSolver4(QObject* parent = nullptr);
    void setDimension(int n);
    void setDiagonals(const QVector<double>& lower, const QVector<double>& main, const QVector<double>& upper);
    QVector<double> solve(const QVector<double>& rhs);
    double determinant() const { return m_det; }
    bool isDiagonallyDominant() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int n, double residual);
private:
    int m_n = 0; double m_det = 1.0;
    QVector<double> m_lower; QVector<double> m_main; QVector<double> m_upper;
    Stats m_stats; double m_timeSum = 0.0;
};
