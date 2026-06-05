#pragma once
#include <QObject>
#include <QVector>
class SparseLU2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int totalSolves = 0; double avgProcessingTimeMs = 0.0; };
    explicit SparseLU2(QObject* parent = nullptr);
    void buildFromCOO(const QVector<int>& rows, const QVector<int>& cols, const QVector<double>& vals, int n);
    bool decompose();
    QVector<double> solve(const QVector<double>& rhs) const;
    int rank() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionComplete(int n, bool success);
private:
    int m_n = 0; bool m_factored = false;
    QVector<double> m_LU; QVector<int> m_pivot;
    Stats m_stats; double m_timeSum = 0.0;
};
