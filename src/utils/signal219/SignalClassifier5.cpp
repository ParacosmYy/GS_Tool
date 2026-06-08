/**
 * @file SignalClassifier5.cpp
 * @brief SignalClassifier5 实现
 *
 * 实现信号分类器：卷积特征提取、最近质心投票、留一交叉验证。
 */

#include "utils/signal219/SignalClassifier5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SignalClassifier5::SignalClassifier5(QObject *parent) : QObject(parent) {}
SignalClassifier5::~SignalClassifier5() = default;

/* ---- Configuration ---- */

void SignalClassifier5::setParameters(int numKernels, int kernelSize)
{
    m_numKernels = qMax(1, numKernels);
    m_kernelSize = qMax(3, kernelSize | 1);  // Ensure odd
}

/* ---- Initialize Gabor-like kernels ---- */

void SignalClassifier5::initKernels(int signalLength)
{
    m_kernels.resize(m_numKernels);
    int poolSize = 4;
    int convLen = signalLength - m_kernelSize + 1;
    int pooledLen = convLen / poolSize;
    int featureLen = m_numKernels * pooledLen;
    m_stats.numFeatures = featureLen;

    for (int k = 0; k < m_numKernels; ++k) {
        m_kernels[k].resize(m_kernelSize);
        double freq = (k + 1) * M_PI / m_kernelSize;
        double phase = k * M_PI / m_numKernels;
        for (int i = 0; i < m_kernelSize; ++i) {
            double t = i - m_kernelSize / 2.0;
            m_kernels[k][i] = qCos(freq * t + phase) * qExp(-t * t / (m_kernelSize / 2.0));
        }
    }
}

/* ---- 1D convolution ---- */

QVector<double> SignalClassifier5::convolve1d(const QVector<double>& signal,
                                                const QVector<double>& kernel) const
{
    int n = signal.size();
    int k = kernel.size();
    int outLen = n - k + 1;
    QVector<double> result(outLen, 0.0);
    for (int i = 0; i < outLen; ++i) {
        double sum = 0.0;
        for (int j = 0; j < k; ++j)
            sum += signal[i + j] * kernel[j];
        result[i] = sum;
    }
    return result;
}

/* ---- Max pooling ---- */

QVector<double> SignalClassifier5::maxPool(const QVector<double>& input,
                                             int poolSize) const
{
    int outLen = input.size() / poolSize;
    QVector<double> result(outLen);
    for (int i = 0; i < outLen; ++i) {
        double mx = -std::numeric_limits<double>::max();
        for (int j = 0; j < poolSize; ++j) {
            int idx = i * poolSize + j;
            if (idx < input.size()) mx = qMax(mx, input[idx]);
        }
        result[i] = mx;
    }
    return result;
}

/* ---- Extract features ---- */

QVector<double> SignalClassifier5::extractFeatures(
    const QVector<double>& signal) const
{
    QVector<double> features;
    features.reserve(m_numKernels * 32);
    int poolSize = 4;
    for (int k = 0; k < m_kernels.size(); ++k) {
        QVector<double> conv = convolve1d(signal, m_kernels[k]);
        QVector<double> pooled = maxPool(conv, poolSize);
        for (double v : pooled) features.append(v);
    }
    return features;
}

/* ---- Euclidean distance ---- */

double SignalClassifier5::featureDistance(const QVector<double>& a,
                                            const QVector<double>& b) const
{
    double d2 = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i)
        d2 += qPow(a[i] - b[i], 2);
    return qSqrt(d2);
}

/* ---- Nearest centroid vote ---- */

int SignalClassifier5::nearestCentroidVote(
    const QVector<double>& features) const
{
    int best = 0;
    double bestDist = std::numeric_limits<double>::max();
    for (int c = 0; c < m_centroids.size(); ++c) {
        double d = featureDistance(features, m_centroids[c]);
        if (d < bestDist) { bestDist = d; best = c; }
    }
    return best;
}

/* ---- Train ---- */

void SignalClassifier5::train(const QVector<QVector<double>>& samples,
                                const QVector<int>& labels)
{
    QElapsedTimer timer;
    timer.start();

    int n = samples.size();
    if (n == 0 || labels.size() != n) return;

    // Determine number of classes
    int maxLabel = 0;
    for (int l : labels) maxLabel = qMax(maxLabel, l);
    m_numClasses = maxLabel + 1;

    // Initialize kernels
    initKernels(samples[0].size());

    // Extract features for all samples
    QVector<QVector<double>> features(n);
    for (int i = 0; i < n; ++i)
        features[i] = extractFeatures(samples[i]);

    // Compute centroids
    m_centroids.resize(m_numClasses);
    QVector<int> counts(m_numClasses, 0);
    for (int c = 0; c < m_numClasses; ++c) {
        m_centroids[c].resize(features[0].size(), 0.0);
    }
    for (int i = 0; i < n; ++i) {
        int c = labels[i];
        counts[c]++;
        for (int j = 0; j < features[i].size(); ++j)
            m_centroids[c][j] += features[i][j];
    }
    for (int c = 0; c < m_numClasses; ++c) {
        if (counts[c] > 0) {
            for (int j = 0; j < m_centroids[c].size(); ++j)
                m_centroids[c][j] /= counts[c];
        }
    }

    // Compute training accuracy
    int correct = 0;
    for (int i = 0; i < n; ++i) {
        if (nearestCentroidVote(features[i]) == labels[i]) correct++;
    }

    m_stats.numSamples = n;
    m_stats.numClasses = m_numClasses;
    m_stats.trainAccuracy = static_cast<double>(correct) / n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit trainingCompleted(m_numClasses, m_stats.trainAccuracy, timer.elapsed());
}

/* ---- Classify single ---- */

int SignalClassifier5::classify(const QVector<double>& signal) const
{
    if (m_centroids.isEmpty()) return -1;
    QVector<double> features = extractFeatures(signal);
    return nearestCentroidVote(features);
}

/* ---- Batch classify ---- */

QVector<int> SignalClassifier5::classifyBatch(
    const QVector<QVector<double>>& samples) const
{
    QVector<int> results(samples.size());
    for (int i = 0; i < samples.size(); ++i)
        results[i] = classify(samples[i]);
    return results;
}

/* ---- LOO validation ---- */

double SignalClassifier5::looValidate(
    const QVector<QVector<double>>& samples,
    const QVector<int>& labels)
{
    QElapsedTimer timer;
    timer.start();

    int n = samples.size();
    if (n == 0 || labels.size() != n) return 0.0;

    initKernels(samples[0].size());
    QVector<QVector<double>> features(n);
    for (int i = 0; i < n; ++i)
        features[i] = extractFeatures(samples[i]);

    int correct = 0;
    m_perClassAcc.resize(m_numClasses, 0.0);
    QVector<int> classCorrect(m_numClasses, 0);
    QVector<int> classTotal(m_numClasses, 0);

    for (int leave = 0; leave < n; ++leave) {
        int maxLabel = 0;
        for (int l : labels) maxLabel = qMax(maxLabel, l);
        int nc = maxLabel + 1;

        // Recompute centroids excluding sample 'leave'
        QVector<QVector<double>> looCentroids(nc,
            QVector<double>(features[0].size(), 0.0));
        QVector<int> counts(nc, 0);
        for (int i = 0; i < n; ++i) {
            if (i == leave) continue;
            int c = labels[i];
            counts[c]++;
            for (int j = 0; j < features[i].size(); ++j)
                looCentroids[c][j] += features[i][j];
        }
        for (int c = 0; c < nc; ++c)
            if (counts[c] > 0)
                for (int j = 0; j < looCentroids[c].size(); ++j)
                    looCentroids[c][j] /= counts[c];

        // Classify left-out sample
        int pred = 0;
        double bestDist = std::numeric_limits<double>::max();
        for (int c = 0; c < nc; ++c) {
            double d = featureDistance(features[leave], looCentroids[c]);
            if (d < bestDist) { bestDist = d; pred = c; }
        }

        classTotal[labels[leave]]++;
        if (pred == labels[leave]) {
            correct++;
            classCorrect[labels[leave]]++;
        }
    }

    for (int c = 0; c < m_numClasses; ++c)
        if (classTotal[c] > 0)
            m_perClassAcc[c] = static_cast<double>(classCorrect[c]) / classTotal[c];

    m_stats.looAccuracy = static_cast<double>(correct) / n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return m_stats.looAccuracy;
}

/* ---- Accessors ---- */

QVector<QVector<double>> SignalClassifier5::centroids() const
{
    return m_centroids;
}

QVector<double> SignalClassifier5::perClassAccuracy() const
{
    return m_perClassAcc;
}

/* ---- Reset ---- */

void SignalClassifier5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_kernels.clear();
    m_centroids.clear();
    m_perClassAcc.clear();
    m_numClasses = 0;
}
