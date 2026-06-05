#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SymmetricEigen7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int matrixSize = 0; double avgProcessingTimeMs = 0.0; };
    explicit SymmetricEigen7(QObject* parent = nullptr);
    bool decompose(const QVector<double>& A, int n);
    QVector<double> eigenvalues() const { return m_eigenvalues; }
    QVector<double> eigenvectors() const { return m_eigenvectors; }
    bool isPositiveDefinite() const;
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionCompleted(int n);
private:
    int m_n = 0; bool m_decomposed = false;
    QVector<double> m_eigenvalues; QVector<double> m_eigenvectors;
    void jacobiIteration(QVector<double>& mat, QVector<double>& ev, int n);
    Stats m_stats; double m_timeSum = 0.0;
};
