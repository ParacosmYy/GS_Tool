#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SparseQR3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int totalSolves = 0; double avgProcessingTimeMs = 0.0; };
    explicit SparseQR3(QObject* parent = nullptr);
    void setDimensions(int rows, int cols);
    void addEntry(int row, int col, double val);
    bool decompose();
    QVector<double> solve(const QVector<double>& b);
    double residualNorm() const { return m_residual; }
    int rank() const { return m_rank; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionCompleted(int rank, double residual);
private:
    int m_rows = 0; int m_cols = 0; int m_rank = 0; double m_residual = 0.0;
    QVector<double> m_Qval; QVector<double> m_Rval;
    QVector<int> m_perm;
    Stats m_stats; double m_timeSum = 0.0;
};
