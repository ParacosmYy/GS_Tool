#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GaussianMixture12 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFits = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit GaussianMixture12(QObject* parent = nullptr);
    void setNumComponents(int k);
    void setMaxIterations(int iter);
    void setCovarianceType(const QString& type);
    bool fit(const QVector<QVector<double>>& data);
    QVector<int> predict(const QVector<QVector<double>>& data);
    double bic() const { return m_bic; }
    double aic() const { return m_aic; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void fitCompleted(int components, double llh);
private:
    int m_k = 3; int m_maxIter = 100; QString m_covType = "full";
    double m_bic = 0.0; double m_aic = 0.0;
    QVector<QVector<double>> m_means; QVector<double> m_weights;
    Stats m_stats; double m_timeSum = 0.0;
};
