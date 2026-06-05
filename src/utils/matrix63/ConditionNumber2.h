#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ConditionNumber2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalDimensions = 0; double avgProcessingTimeMs = 0.0; };
    explicit ConditionNumber2(QObject* parent = nullptr);
    void setMatrix(const QVector<QVector<double>>& A);
    double compute(int normType = 2);
    double conditionNumber() const { return m_cond; }
    double norm1() const { return m_norm1; }
    double normInf() const { return m_normInf; }
    double normFrobenius() const { return m_normFrob; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(int n, double cond);
private:
    int m_n = 0; double m_cond = 0.0;
    double m_norm1 = 0.0; double m_normInf = 0.0; double m_normFrob = 0.0;
    QVector<QVector<double>> m_A;
    double computeNorm1(const QVector<QVector<double>>& A) const;
    double computeNormInf(const QVector<QVector<double>>& A) const;
    double computeNormFrob(const QVector<QVector<double>>& A) const;
    Stats m_stats; double m_timeSum = 0.0;
};
