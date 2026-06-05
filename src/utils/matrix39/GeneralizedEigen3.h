#pragma once
#include <QObject>
#include <QVector>
class GeneralizedEigen3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; double avgProcessingTimeMs = 0.0; };
    explicit GeneralizedEigen3(QObject* parent = nullptr);
    void decompose(const QVector<double>& A, const QVector<double>& B, int n);
    QVector<double> eigenvaluesReal() const;
    QVector<double> eigenvaluesImag() const;
    QVector<double> eigenvectors() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionComplete(int n);
private:
    int m_n = 0;
    QVector<double> m_alphaReal, m_alphaImag, m_beta;
    QVector<double> m_eigenvectors;
    Stats m_stats; double m_timeSum = 0.0;
};
