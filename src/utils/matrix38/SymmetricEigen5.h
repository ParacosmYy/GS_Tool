#pragma once
#include <QObject>
#include <QVector>
/** @brief Symmetric eigenvalue 5 - tridiag QR/bisection/divide-conquer */
class SymmetricEigen5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int totalEigenvalues = 0; double avgProcessingTimeMs = 0.0; };
    explicit SymmetricEigen5(QObject* parent = nullptr);
    void decompose(const QVector<double>& matrix, int n);
    QVector<double> eigenvalues() const;
    QVector<double> eigenvectors() const;
    int rank(double tol = 1e-10) const;
    double conditionNumber() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionComplete(int n, int eigenvalues);
private:
    int m_n = 0;
    QVector<double> m_eigenvalues; QVector<double> m_eigenvectors;
    Stats m_stats; double m_timeSum = 0.0;
};
