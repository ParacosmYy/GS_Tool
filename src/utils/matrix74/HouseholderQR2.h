#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class HouseholderQR2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int totalSolves = 0; double avgProcessingTimeMs = 0.0; };
    explicit HouseholderQR2(QObject* parent = nullptr);
    void setMatrix(const QVector<QVector<double>>& A);
    bool decompose();
    QVector<QVector<double>> matrixQ() const { return m_Q; }
    QVector<QVector<double>> matrixR() const { return m_R; }
    QVector<double> solve(const QVector<double>& b);
    double residualNorm() const { return m_residual; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionCompleted(int m, int n);
private:
    int m_rows = 0; int m_cols = 0; double m_residual = 0.0;
    QVector<QVector<double>> m_Q, m_R;
    Stats m_stats; double m_timeSum = 0.0;
};
