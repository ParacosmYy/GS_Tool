#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SchurDecomp5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int matrixSize = 0; double avgProcessingTimeMs = 0.0; };
    explicit SchurDecomp5(QObject* parent = nullptr);
    bool decompose(const QVector<double>& A, int n);
    QVector<double> schurForm() const { return m_T; }
    QVector<double> schurVectors() const { return m_Q; }
    QVector<QPair<double,double>> eigenvalues() const;
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionCompleted(int n);
private:
    int m_n = 0; QVector<double> m_T; QVector<double> m_Q;
    void francisQR(QVector<double>& H, QVector<double>& Q, int n);
    Stats m_stats; double m_timeSum = 0.0;
};
