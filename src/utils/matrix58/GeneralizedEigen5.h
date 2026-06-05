#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GeneralizedEigen5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int matrixSize = 0; double avgProcessingTimeMs = 0.0; };
    explicit GeneralizedEigen5(QObject* parent = nullptr);
    bool decompose(const QVector<double>& A, const QVector<double>& B, int n);
    QVector<QPair<double,double>> eigenvalues() const;
    QVector<double> eigenvectors() const { return m_V; }
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionCompleted(int n);
private:
    int m_n = 0; QVector<double> m_S; QVector<double> m_T;
    QVector<double> m_Q; QVector<double> m_Z; QVector<double> m_V;
    void qzStep(QVector<double>& S, QVector<double>& T, int n);
    Stats m_stats; double m_timeSum = 0.0;
};
