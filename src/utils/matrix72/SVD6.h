#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SVD6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int totalDimensions = 0; double avgProcessingTimeMs = 0.0; };
    explicit SVD6(QObject* parent = nullptr);
    void setMatrix(const QVector<QVector<double>>& A);
    bool decompose();
    QVector<QVector<double>> matrixU() const { return m_U; }
    QVector<double> singularValues() const { return m_S; }
    QVector<QVector<double>> matrixV() const { return m_V; }
    int rank(double tol) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionCompleted(int m, int n, int rank);
private:
    int m_rows = 0; int m_cols = 0;
    QVector<QVector<double>> m_U, m_V; QVector<double> m_S;
    void bidiagonalize(QVector<QVector<double>>& U, QVector<double>& d, QVector<double>& e, QVector<QVector<double>>& V);
    void golubKahan(QVector<double>& d, QVector<double>& e, QVector<QVector<double>>& U, QVector<QVector<double>>& V);
    Stats m_stats; double m_timeSum = 0.0;
};
