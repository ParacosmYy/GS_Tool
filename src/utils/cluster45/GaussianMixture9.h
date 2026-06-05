/**
 * @file GaussianMixture9.h
 * @brief 高斯混合模型9 — 半监督EM+标签传播
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class GaussianMixture9 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalFits = 0;
        int totalSamplesProcessed = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture9(QObject* parent = nullptr);

    void setComponents(int k);
    void setLabeledData(const QVector<QVector<double>>& data,
                         const QVector<int>& labels);
    void setUnlabeledData(const QVector<QVector<double>>& data);
    QVector<int> fit(int maxIter = 200, double tol = 1e-6);
    QVector<int> predict(const QVector<QVector<double>>& data) const;
    QVector<double> predictProb(const QVector<double>& sample) const;

    int components() const { return m_k; }
    double logLikelihood() const { return m_logLikelihood; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitCompleted(int k, int iterations, double logLikelihood);

private:
    int m_k = 3;
    int m_dim = 0;
    QVector<double> m_weights;
    QVector<QVector<double>> m_means;
    QVector<QVector<QVector<double>>> m_covariances;
    double m_logLikelihood = 0.0;
    QVector<int> m_labeledLabels;
    QVector<QVector<double>> m_labeledData;
    QVector<QVector<double>> m_unlabeledData;

    void initializeFromLabeled();
    double gaussianPDF(const QVector<double>& x, int comp) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
