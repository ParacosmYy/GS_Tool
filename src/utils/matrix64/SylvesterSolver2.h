#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SylvesterSolver2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalDimensions = 0; double avgProcessingTimeMs = 0.0; };
    explicit SylvesterSolver2(QObject* parent = nullptr);
    void setMatrices(const QVector<QVector<double>>& A, const QVector<QVector<double>>& B, const QVector<QVector<double>>& C);
    QVector<QVector<double>> solve();
    double residual() const { return m_residual; }
    bool isSolved() const { return m_solved; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int n, double residual);
private:
    int m_n = 0; double m_residual = 0.0; bool m_solved = false;
    QVector<QVector<double>> m_A, m_B, m_C;
    QVector<QVector<double>> bartelsStewart();
    Stats m_stats; double m_timeSum = 0.0;
};
