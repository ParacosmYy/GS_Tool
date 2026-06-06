/**
 * @file SignalClassifier.cpp
 * @brief SignalClassifier 实现
 *
 * 实现信号分类器：时域特征(ZCR/RMS/波峰因子)、频域特征(频谱质心/带宽)、KNN分类。
 */

#include "utils/signal184/SignalClassifier.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SignalClassifier::SignalClassifier(QObject *parent) : QObject(parent) {}
SignalClassifier::~SignalClassifier() = default;

/* ---- Configuration ---- */

void SignalClassifier::setSampleRate(double rate) { m_sampleRate = qMax(8000.0, rate); }
void SignalClassifier::setKNNNeighbors(int k) { m_knnK = qMax(1, k); }

/* ---- ZCR ---- */

double SignalClassifier::zeroCrossingRate(const QVector<double>& signal) const
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

/* ---- RMS ---- */

double SignalClassifier::rmsEnergy(const QVector<double>& signal) const
{
    if (signal.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double s : signal) sum += s * s;
    return qSqrt(sum / signal.size());
}

/* ---- Crest factor ---- */

double SignalClassifier::crestFactor(const QVector<double>& signal) const
{
    if (signal.isEmpty()) return 0.0;
    double peak = 0.0;
    for (double s : signal) peak = qMax(peak, qAbs(s));
    double rms = rmsEnergy(signal);
    return (rms > 1e-10) ? peak / rms : 0.0;
}

/* ---- Spectral centroid ---- */

double SignalClassifier::spectralCentroid(const QVector<double>& signal) const
{
    int n = signal.size();
    if (n < 2) return 0.0;

    // Compute magnitude spectrum via DFT (small N)
    int fftN = 1;
    while (fftN < n) fftN *= 2;

    double num = 0.0, den = 0.0;
    for (int k = 0; k < fftN / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / fftN;
            re += signal[i] * qCos(angle);
            im += signal[i] * qSin(angle);
        }
        double mag = qSqrt(re * re + im * im);
        double freq = static_cast<double>(k) * m_sampleRate / fftN;
        num += freq * mag;
        den += mag;
    }
    return (den > 1e-10) ? num / den : 0.0;
}

/* ---- Spectral bandwidth ---- */

double SignalClassifier::spectralBandwidth(const QVector<double>& signal) const
{
    int n = signal.size();
    if (n < 2) return 0.0;

    int fftN = 1;
    while (fftN < n) fftN *= 2;

    double centroid = spectralCentroid(signal);

    double num = 0.0, den = 0.0;
    for (int k = 0; k < fftN / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / fftN;
            re += signal[i] * qCos(angle);
            im += signal[i] * qSin(angle);
        }
        double mag = qSqrt(re * re + im * im);
        double freq = static_cast<double>(k) * m_sampleRate / fftN;
        double diff = freq - centroid;
        num += diff * diff * mag;
        den += mag;
    }
    return (den > 1e-10) ? qSqrt(num / den) : 0.0;
}

/* ---- Spectral rolloff ---- */

double SignalClassifier::spectralRolloff(const QVector<double>& signal) const
{
    int n = signal.size();
    if (n < 2) return 0.0;

    int fftN = 1;
    while (fftN < n) fftN *= 2;

    double totalEnergy = 0.0;
    QVector<double> mag(fftN / 2, 0.0);
    for (int k = 0; k < fftN / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / fftN;
            re += signal[i] * qCos(angle);
            im += signal[i] * qSin(angle);
        }
        mag[k] = re * re + im * im;
        totalEnergy += mag[k];
    }

    double threshold = 0.85 * totalEnergy; // 85% rolloff
    double cumEnergy = 0.0;
    for (int k = 0; k < fftN / 2; ++k) {
        cumEnergy += mag[k];
        if (cumEnergy >= threshold)
            return static_cast<double>(k) * m_sampleRate / fftN;
    }
    return m_sampleRate / 2.0;
}

/* ---- Extract features ---- */

SignalClassifier::Features SignalClassifier::extractFeatures(const QVector<double>& signal) const
{
    Features f;
    f.zcr = zeroCrossingRate(signal);
    f.rms = rmsEnergy(signal);
    f.crest = crestFactor(signal);
    f.spectralCentroid = spectralCentroid(signal);
    f.bandwidth = spectralBandwidth(signal);
    f.spectralRolloff = spectralRolloff(signal);
    return f;
}

/* ---- Add template ---- */

void SignalClassifier::addTemplate(const QString& label, const QVector<double>& signal)
{
    Template t;
    t.label = label;
    t.features = extractFeatures(signal);
    m_templates.append(t);

    m_stats.numTemplates = m_templates.size();
    QSet<QString> classes;
    for (const auto& tmpl : m_templates) classes.insert(tmpl.label);
    m_stats.numClasses = classes.size();
}

/* ---- Feature distance ---- */

double SignalClassifier::featureDistance(const Features& a, const Features& b) const
{
    double d = 0.0;
    d += (a.zcr - b.zcr) * (a.zcr - b.zcr);
    d += (a.rms - b.rms) * (a.rms - b.rms);
    d += (a.crest - b.crest) * (a.crest - b.crest);

    // Normalize frequency features to [0,1] range
    double maxFreq = m_sampleRate / 2.0;
    double sc1 = a.spectralCentroid / maxFreq, sc2 = b.spectralCentroid / maxFreq;
    double bw1 = a.bandwidth / maxFreq, bw2 = b.bandwidth / maxFreq;
    double sr1 = a.spectralRolloff / maxFreq, sr2 = b.spectralRolloff / maxFreq;

    d += (sc1 - sc2) * (sc1 - sc2);
    d += (bw1 - bw2) * (bw1 - bw2);
    d += (sr1 - sr2) * (sr1 - sr2);

    return qSqrt(d);
}

/* ---- Classify ---- */

QString SignalClassifier::classify(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (m_templates.isEmpty()) return {};

    Features f = extractFeatures(signal);

    // KNN: find k nearest templates
    QVector<QPair<double, QString>> distances;
    for (const auto& tmpl : m_templates)
        distances.append(qMakePair(featureDistance(f, tmpl.features), tmpl.label));

    std::sort(distances.begin(), distances.end());

    // Vote among k nearest
    QMap<QString, int> votes;
    int k = qMin(m_knnK, distances.size());
    for (int i = 0; i < k; ++i)
        votes[distances[i].second]++;

    QString bestLabel;
    int bestVotes = 0;
    for (auto it = votes.begin(); it != votes.end(); ++it) {
        if (it.value() > bestVotes) { bestVotes = it.value(); bestLabel = it.key(); }
    }

    double confidence = static_cast<double>(bestVotes) / k;

    m_stats.totalClassifications++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClassifications;

    emit classificationCompleted(bestLabel, confidence);
    return bestLabel;
}

/* ---- Reset ---- */

void SignalClassifier::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_templates.clear();
}
