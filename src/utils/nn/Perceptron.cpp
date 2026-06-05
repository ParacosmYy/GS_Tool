/**
 * @file Perceptron.cpp
 * @brief 感知机实现
 */

#include "Perceptron.h"
#include <QElapsedTimer>

Perceptron::Perceptron(int numFeatures, double learningRate, int maxEpochs,
                         QObject* parent)
    : QObject(parent)
    , m_weights(numFeatures, 0.0)
    , m_bias(0.0)
    , m_lr(learningRate)
    , m_maxEpochs(maxEpochs)
    , m_timeSum(0.0)
{
}

bool Perceptron::train(const QVector<QPair<QVector<double>, int>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_stats.totalTrained++;
        m_timeSum += timer.elapsed();
        return true;
    }

    m_weights.fill(0.0);
    m_bias = 0.0;

    bool converged = false;
    int epoch;

    for (epoch = 0; epoch < m_maxEpochs; ++epoch) {
        int errors = 0;
        for (const auto& [features, label] : data) {
            int pred = predict(features);
            if (pred != label) {
                double delta = m_lr * label;
                for (int i = 0; i < m_weights.size() && i < features.size(); ++i)
                    m_weights[i] += delta * features[i];
                m_bias += delta;
                errors++;
            }
        }
        if (errors == 0) { converged = true; break; }
    }

    m_stats.totalTrained++;
    m_stats.totalEpochs += epoch + 1;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTrained;

    emit trainCompleted(epoch + 1, converged);
    return converged;
}

int Perceptron::predict(const QVector<double>& features) const
{
    return (confidence(features) >= 0) ? 1 : -1;
}

double Perceptron::confidence(const QVector<double>& features) const
{
    double sum = m_bias;
    for (int i = 0; i < m_weights.size() && i < features.size(); ++i)
        sum += m_weights[i] * features[i];

    m_stats.totalPredictions++;
    return sum;
}

QVector<double> Perceptron::weights() const { return m_weights; }
double Perceptron::bias() const { return m_bias; }
Perceptron::Stats Perceptron::stats() const { return m_stats; }

void Perceptron::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
