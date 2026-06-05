#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class Hessenberg2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalReductions = 0; int totalDimensions = 0; double avgProcessingTimeMs = 0.0; };
    explicit Hessenberg2(QObject* parent = nullptr);
    void setMatrix(const QVector<QVector<double>>& A);
    bool reduce();
    QVector<QVector<double>> hessenbergForm() const { return m_H; }
    QVector<QVector<double>> transform() const { return m_Q; }
    int dimension() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void reductionCompleted(int n, double offDiagNorm);
private:
    int m_n = 0;
    QVector<QVector<double>> m_A; QVector<QVector<double>> m_H; QVector<QVector<double>> m_Q;
    void householderStep(int col);
    Stats m_stats; double m_timeSum = 0.0;
};
