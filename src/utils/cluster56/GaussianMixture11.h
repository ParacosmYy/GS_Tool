#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GaussianMixture11 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFits = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit GaussianMixture11(QObject* parent = nullptr);
    void setNumComponents(int k);
    void setMaxIterations(int iter);
    void setConvergence(double tol);
    bool fit(const QVector<QVector<double>>& data);
    QVector<int> predict(const QVector<QVector<double>>& data);
    QVector<double> logLikelihood() const { return m_logLik; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void fitCompleted(int components, double llh);
private:
    int m_k = 3; int m_maxIter = 100; double m_tol = 1e-6;
    QVector<double> m_logLik;
    QVector<QVector<double>> m_means; QVector<QVector<QVector<double>>> m_covs; QVector<double> m_weights;
    double gaussianPdf(const QVector<double>& x, const QVector<double>& mu, const QVector<QVector<double>>& cov);
    Stats m_stats; double m_timeSum = 0.0;
};
