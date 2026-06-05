#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SchurComplement2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalDimensions = 0; double avgProcessingTimeMs = 0.0; };
    explicit SchurComplement2(QObject* parent = nullptr);
    void setMatrix(const QVector<QVector<double>>& A);
    void setPartition(int n1, int n2);
    QVector<QVector<double>> compute();
    QVector<QVector<double>> complement() const { return m_complement; }
    double determinant() const { return m_det; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(int dim, double det);
private:
    int m_n1 = 0; int m_n2 = 0; double m_det = 0.0;
    QVector<QVector<double>> m_A; QVector<QVector<double>> m_complement;
    QVector<QVector<double>> matMul(const QVector<QVector<double>>& A, const QVector<QVector<double>>& B);
    QVector<QVector<double>> matInv(const QVector<QVector<double>>& A);
    Stats m_stats; double m_timeSum = 0.0;
};
