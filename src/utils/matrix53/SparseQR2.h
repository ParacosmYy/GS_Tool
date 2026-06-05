#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SparseQR2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int totalSolves = 0; int matrixSize = 0; double avgProcessingTimeMs = 0.0; };
    explicit SparseQR2(QObject* parent = nullptr);
    void setMatrix(int rows, int cols, const QVector<int>& rowPtr,
                   const QVector<int>& colIdx, const QVector<double>& values);
    bool decompose();
    QVector<double> solve(const QVector<double>& rhs);
    int rank() const { return m_rank; }
    double residualNorm() const { return m_resNorm; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionCompleted(int rows, int cols, int rank);
private:
    int m_rows = 0; int m_cols = 0; int m_rank = 0; double m_resNorm = 0.0;
    QVector<int> m_rowPtr; QVector<int> m_colIdx; QVector<double> m_values;
    QVector<double> m_R; QVector<double> m_Qdiag; bool m_decomposed = false;
    Stats m_stats; double m_timeSum = 0.0;
};
