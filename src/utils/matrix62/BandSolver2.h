#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BandSolver2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalSystems = 0; double avgProcessingTimeMs = 0.0; };
    explicit BandSolver2(QObject* parent = nullptr);
    void setBandwidth(int lower, int upper);
    void setDimension(int n);
    void setMatrix(const QVector<QVector<double>>& bands);
    QVector<double> solve(const QVector<double>& rhs);
    bool isPositiveDefinite() const { return m_posDef; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int n, double residual);
private:
    int m_n = 0; int m_lower = 0; int m_upper = 0; bool m_posDef = true;
    QVector<QVector<double>> m_factor;
    void factorize();
    Stats m_stats; double m_timeSum = 0.0;
};
