#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SparseLU2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int totalSolves = 0; double avgProcessingTimeMs = 0.0; };
    explicit SparseLU2(QObject* parent = nullptr);
    void setMatrix(int n, const QVector<int>& rows, const QVector<int>& cols, const QVector<double>& vals);
    bool decompose();
    QVector<double> solve(const QVector<double>& b);
    double determinant() const { return m_det; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionCompleted(int n, double det);
private:
    int m_n = 0; double m_det = 1.0;
    QVector<int> m_pivot;
    QVector<double> m_Lval; QVector<double> m_Uval;
    Stats m_stats; double m_timeSum = 0.0;
};
