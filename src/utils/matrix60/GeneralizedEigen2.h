#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GeneralizedEigen2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalDimensions = 0; double avgProcessingTimeMs = 0.0; };
    explicit GeneralizedEigen2(QObject* parent = nullptr);
    void setMatrices(const QVector<QVector<double>>& A, const QVector<QVector<double>>& B);
    bool solve();
    QVector<double> eigenvalues() const { return m_eigenvalues; }
    QVector<QVector<double>> eigenvectors() const { return m_eigvecs; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int n, double maxEigenvalue);
private:
    int m_n = 0;
    QVector<double> m_eigenvalues; QVector<QVector<double>> m_eigvecs;
    bool qzDecomposition(QVector<QVector<double>>& AA, QVector<QVector<double>>& BB);
    Stats m_stats; double m_timeSum = 0.0;
};
