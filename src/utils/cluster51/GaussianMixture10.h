#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GaussianMixture10 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFits = 0; int totalPredictions = 0; double avgProcessingTimeMs = 0.0; };
    explicit GaussianMixture10(QObject* parent = nullptr);
    void setComponents(int k);
    QVector<int> fit(const QVector<QVector<double>>& data, int maxIter = 100);
    QVector<double> predictProb(const QVector<double>& sample) const;
    int components() const { return m_k; }
    double bic() const { return m_bic; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void fitCompleted(int k, double bic);
private:
    int m_k = 3; int m_dim = 0;
    QVector<double> m_weights; QVector<QVector<double>> m_means;
    QVector<QVector<QVector<double>>> m_covs; double m_bic = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
