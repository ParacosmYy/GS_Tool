/**
 * @file GaussianMixture8.h
 * @brief 高斯混合模型8 — 变分贝叶斯推断
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class GaussianMixture8 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalFits = 0;
        int totalSamplesProcessed = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture8(QObject* parent = nullptr);

    void setComponents(int k);
    void fit(const QVector<QVector<double>>& data, int maxIter = 200, double tol = 1e-6);
    int predict(const QVector<double>& sample) const;
    QVector<double> predictProb(const QVector<double>& sample) const;
    double lowerBound() const { return m_lowerBound; }

    int components() const { return m_k; }
    QVector<double> weights() const { return m_weights; }
    QVector<QVector<double>> means() const { return m_means; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitCompleted(int k, int iterations, double lowerBound);

private:
    int m_k = 3;
    int m_dim = 0;
    int m_n = 0;
    QVector<double> m_weights;
    QVector<QVector<double>> m_means;
    QVector<QVector<QVector<double>>> m_precisions;
    double m_lowerBound = 0.0;
    double m_concentrationSum = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
