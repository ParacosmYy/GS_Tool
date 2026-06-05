#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class TridiagonalEigen2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int totalDimensions = 0; double avgProcessingTimeMs = 0.0; };
    explicit TridiagonalEigen2(QObject* parent = nullptr);
    void setDiagonal(const QVector<double>& diag);
    void setSubdiagonal(const QVector<double>& sub);
    bool solve();
    QVector<double> eigenvalues() const { return m_eigenvalues; }
    QVector<QVector<double>> eigenvectors() const { return m_eigvecs; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionCompleted(int n, double maxEigen);
private:
    QVector<double> m_diag; QVector<double> m_sub;
    QVector<double> m_eigenvalues; QVector<QVector<double>> m_eigvecs;
    void implicitQR(QVector<double>& d, QVector<double>& e, QVector<QVector<double>>& Q);
    Stats m_stats; double m_timeSum = 0.0;
};
