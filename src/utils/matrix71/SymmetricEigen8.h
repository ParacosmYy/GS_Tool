#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SymmetricEigen8 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int totalDimensions = 0; double avgProcessingTimeMs = 0.0; };
    explicit SymmetricEigen8(QObject* parent = nullptr);
    void setMatrix(const QVector<QVector<double>>& A);
    bool solve();
    QVector<double> eigenvalues() const { return m_eigenvalues; }
    QVector<QVector<double>> eigenvectors() const { return m_eigvecs; }
    double conditionNumber() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionCompleted(int n, double maxEigen);
private:
    int m_n = 0;
    QVector<QVector<double>> m_A;
    QVector<double> m_eigenvalues; QVector<QVector<double>> m_eigvecs;
    void tridiagonalize(QVector<QVector<double>>& Q);
    void implicitQL(QVector<double>& d, QVector<double>& e, QVector<QVector<double>>& Q);
    Stats m_stats; double m_timeSum = 0.0;
};
