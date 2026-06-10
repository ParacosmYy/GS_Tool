/**
 * @file SignalClassifier9.cpp
 * @brief SignalClassifier9 实现
 *
 * 实现信号分类器：统计矩特征与K近邻决策调制识别。
 */

#include "utils/signal275/SignalClassifier9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SignalClassifier9::SignalClassifier9(QObject *parent)
    : QObject(parent) {}

SignalClassifier9::~SignalClassifier9() = default;

/* ---- Configuration ---- */

void SignalClassifier9::setK(int k) { m_k = qBound(1, k, 50); }

/* ---- Compute n-th central moment ---- */

double SignalClassifier9::centralMoment(const QVector<double>& signal, int order) const
{
    int n = signal.size();
    if (n == 0) return 0.0;

    // Compute mean
    double mean = 0.0;
    for (double v : signal) mean += v;
    mean /= n;

    // Compute central moment
    double moment = 0.0;
    for (double v : signal) {
        double diff = v - mean;
        double val = 1.0;
        for (int p = 0; p < order; ++p) val *= diff;
        moment += val;
    }
    return moment / n;
}

/* ---- Zero-crossing rate ---- */

double SignalClassifier9::zeroCrossingRate(const QVector<double>& signal) const
{
    int n = signal.size();
    if (n < 2) return 0.0;

    int crossings = 0;
    for (int i = 1; i < n; ++i) {
        if ((signal[i] >= 0.0) != (signal[i - 1] >= 0.0))
            crossings++;
    }
    return static_cast<double>(crossings) / (n - 1);
}

/* ---- Euclidean distance ---- */

double SignalClassifier9::euclideanDist(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- k-NN majority vote ---- */

QString SignalClassifier9::knnVote(const QVector<QPair<double, int>>& distances) const
{
    int kk = qMin(m_k, distances.size());
    QMap<QString, int> votes;

    for (int i = 0; i < kk; ++i) {
        int idx = distances[i].second;
        const QString& label = m_templates[idx].label;
        // Weight by inverse distance
        double w = 1.0 / (distances[i].first + 1e-10);
        votes[label] += static_cast<int>(w * 1000);  // Integer approximation
    }

    QString best;
    int maxVotes = 0;
    for (auto it = votes.constBegin(); it != votes.constEnd(); ++it) {
        if (it.value() > maxVotes) {
            maxVotes = it.value();
            best = it.key();
        }
    }
    return best;
}

/* ---- Extract statistical moment features ---- */

QVector<double> SignalClassifier9::extractFeatures(const QVector<double>& signal) const
{
    QVector<double> features(FEATURE_DIM, 0.0);
    if (signal.isEmpty()) return features;

    int n = signal.size();

    // Mean (1st raw moment)
    double mean = 0.0;
    for (double v : signal) mean += v;
    mean /= n;

    // Variance (2nd central moment)
    double var = centralMoment(signal, 2);
    double stdDev = qSqrt(qMax(var, 0.0));

    // 3rd and 4th central moments (skewness, kurtosis)
    double m3 = centralMoment(signal, 3);
    double m4 = centralMoment(signal, 4);

    double skewness = (stdDev > 1e-12) ? m3 / (stdDev * stdDev * stdDev) : 0.0;
    double kurtosis = (var > 1e-12) ? m4 / (var * var) : 0.0;

    // Peak-to-RMS ratio
    double peak = 0.0;
    double rmsSum = 0.0;
    for (double v : signal) {
        double av = qFabs(v);
        if (av > peak) peak = av;
        rmsSum += v * v;
    }
    double rms = qSqrt(rmsSum / n);
    double peakToRms = (rms > 1e-12) ? peak / rms : 0.0;

    // Zero-crossing rate
    double zcr = zeroCrossingRate(signal);

    // 5th and 6th central moments
    double m5 = centralMoment(signal, 5);
    double m6 = centralMoment(signal, 6);

    // Spectral centroid approximation via autocorrelation at lag 1
    double autocorr = 0.0;
    for (int i = 0; i < n - 1; ++i)
        autocorr += signal[i] * signal[i + 1];
    autocorr /= (n - 1);
    double normAutocorr = (var > 1e-12) ? autocorr / var : 0.0;

    features[0] = mean;
    features[1] = stdDev;
    features[2] = skewness;
    features[3] = kurtosis;
    features[4] = peakToRms;
    features[5] = zcr;
    features[6] = m5;
    features[7] = m6;
    features[8] = normAutocorr;
    features[9] = peak;

    return features;
}

/* ---- Add template ---- */

void SignalClassifier9::addTemplate(const QString& label, const QVector<double>& features)
{
    Template t;
    t.label = label;
    t.features = features;
    m_templates.append(t);
    m_stats.numTemplates = m_templates.size();
}

/* ---- Batch train ---- */

void SignalClassifier9::train(const QVector<QPair<QString, QVector<double>>>& dataset)
{
    for (const auto& item : dataset)
        addTemplate(item.first, item.second);
}

/* ---- Classify signal ---- */

QString SignalClassifier9::classify(const QVector<double>& signal) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> features = extractFeatures(signal);

    // Compute distances to all templates
    QVector<QPair<double, int>> distances;
    distances.reserve(m_templates.size());
    for (int i = 0; i < m_templates.size(); ++i) {
        double dist = euclideanDist(features, m_templates[i].features);
        distances.append({dist, i});
    }

    // Sort by distance
    std::sort(distances.begin(), distances.end());

    m_lastDistance = distances.isEmpty() ? 0.0 : distances[0].first;
    QString label = knnVote(distances);

    double elapsed = timer.elapsed();
    const_cast<SignalClassifier9*>(this)->m_stats.numClassified++;
    const_cast<SignalClassifier9*>(this)->m_stats.totalOps++;
    const_cast<SignalClassifier9*>(this)->m_timeSum += elapsed;
    const_cast<SignalClassifier9*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit classificationDone(label, m_lastDistance, elapsed);

    return label;
}

/* ---- Accessors ---- */

double SignalClassifier9::lastDistance() const { return m_lastDistance; }

/* ---- Reset ---- */

void SignalClassifier9::resetStatistics()
{
    m_templates.clear();
    m_lastDistance = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
