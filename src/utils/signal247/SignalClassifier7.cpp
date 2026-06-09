/**
 * @file SignalClassifier7.cpp
 * @brief SignalClassifier7 实现
 *
 * 实现信号分类器：频谱特征提取与k近邻距离加权概率投票。
 */

#include "utils/signal247/SignalClassifier7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SignalClassifier7::SignalClassifier7(QObject *parent) : QObject(parent) {}
SignalClassifier7::~SignalClassifier7() = default;

/* ---- Configuration ---- */

void SignalClassifier7::setK(int k) { m_k = qMax(1, k); }
void SignalClassifier7::setSampleRate(int sr) { m_sampleRate = qMax(1, sr); }

/* ---- FFT helper (radix-2 in-place) ---- */

void SignalClassifier7::fft1D(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    int bits = 0;
    while ((1 << bits) < n) bits++;
    for (int i = 0; i < n; ++i) {
        int rev = 0, val = i;
        for (int b = 0; b < bits; ++b) { rev = (rev << 1) | (val & 1); val >>= 1; }
        if (i < rev) { std::swap(re[i], re[rev]); std::swap(im[i], im[rev]); }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uRe = re[i+j], uIm = im[i+j];
                double vRe = re[i+j+len/2]*curRe - im[i+j+len/2]*curIm;
                double vIm = re[i+j+len/2]*curIm + im[i+j+len/2]*curRe;
                re[i+j] = uRe + vRe; im[i+j] = uIm + vIm;
                re[i+j+len/2] = uRe - vRe; im[i+j+len/2] = uIm - vIm;
                double nr = curRe*wRe - curIm*wIm;
                curIm = curRe*wIm + curIm*wRe; curRe = nr;
            }
        }
    }
}

/* ---- Magnitude spectrum ---- */

QVector<double> SignalClassifier7::magnitudeSpectrum(const QVector<double>& signal) const
{
    int n = signal.size();
    int padded = 1;
    while (padded < n) padded <<= 1;
    QVector<double> re(padded, 0.0), im(padded, 0.0);
    // Apply Hann window
    for (int i = 0; i < n; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
        re[i] = signal[i] * w;
    }
    fft1D(re, im);
    QVector<double> mag(padded / 2);
    for (int i = 0; i < padded / 2; ++i)
        mag[i] = qSqrt(re[i] * re[i] + im[i] * im[i]);
    return mag;
}

/* ---- Mel filter bank energies ---- */

QVector<double> SignalClassifier7::melEnergies(const QVector<double>& spectrum) const
{
    int numFilters = 26;
    int fftSize = spectrum.size() * 2;
    double maxMel = 2595.0 * qLn10(1.0 + m_sampleRate / 1400.0);
    QVector<double> melEdges(numFilters + 2);
    for (int i = 0; i < numFilters + 2; ++i)
        melEdges[i] = i * maxMel / (numFilters + 1);

    // Convert mel back to frequency bin indices
    QVector<int> binIdx(numFilters + 2);
    for (int i = 0; i < numFilters + 2; ++i) {
        double f = 700.0 * (qPow(10.0, melEdges[i] / 2595.0) - 1.0);
        binIdx[i] = qBound(0, static_cast<int>(f * fftSize / m_sampleRate), spectrum.size() - 1);
    }

    QVector<double> energies(numFilters, 0.0);
    for (int m = 0; m < numFilters; ++m) {
        for (int k = binIdx[m]; k <= binIdx[m + 2]; ++k) {
            double weight = 0.0;
            if (k >= binIdx[m] && k < binIdx[m + 1])
                weight = static_cast<double>(k - binIdx[m]) / (binIdx[m + 1] - binIdx[m] + 1);
            else if (k >= binIdx[m + 1] && k <= binIdx[m + 2])
                weight = static_cast<double>(binIdx[m + 2] - k) / (binIdx[m + 2] - binIdx[m + 1] + 1);
            energies[m] += weight * spectrum[k];
        }
    }
    return energies;
}

/* ---- DCT-II ---- */

QVector<double> SignalClassifier7::dctII(const QVector<double>& input, int numCoeffs) const
{
    int n = input.size();
    QVector<double> coeffs(numCoeffs);
    for (int k = 0; k < numCoeffs; ++k) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i)
            sum += input[i] * qCos(M_PI * k * (2 * i + 1) / (2.0 * n));
        coeffs[k] = sum;
    }
    return coeffs;
}

/* ---- Extract features ---- */

SignalClassifier7::Features SignalClassifier7::extractFeatures(const QVector<double>& signal) const
{
    Features f;
    int n = signal.size();
    if (n == 0) return f;

    QVector<double> spec = magnitudeSpectrum(signal);
    int specN = spec.size();
    double totalPower = 0.0;
    for (int i = 0; i < specN; ++i) totalPower += spec[i] * spec[i];

    // Spectral centroid
    double weightedSum = 0.0;
    for (int i = 0; i < specN; ++i)
        weightedSum += i * spec[i];
    f.meanFreq = (totalPower > 0) ? weightedSum / totalPower * m_sampleRate / (2 * specN) : 0.0;

    // Peak frequency
    int peakIdx = 0;
    double peakVal = 0.0;
    for (int i = 0; i < specN; ++i) {
        if (spec[i] > peakVal) { peakVal = spec[i]; peakIdx = i; }
    }
    f.peakFreq = peakIdx * m_sampleRate / (2 * specN);

    // Bandwidth
    double bwSum = 0.0;
    for (int i = 0; i < specN; ++i)
        bwSum += spec[i] * (i - peakIdx) * (i - peakIdx);
    f.bandwidth = qSqrt(bwSum / (totalPower + 1e-10));

    // Spectral rolloff (85% energy)
    double cumEnergy = 0.0;
    f.spectralRolloff = specN - 1;
    for (int i = 0; i < specN; ++i) {
        cumEnergy += spec[i] * spec[i];
        if (cumEnergy >= 0.85 * totalPower) { f.spectralRolloff = i; break; }
    }
    f.spectralRolloff *= m_sampleRate / (2.0 * specN);

    // Spectral flatness
    double logSum = 0.0, linSum = 0.0;
    for (int i = 0; i < specN; ++i) {
        logSum += qLn(qMax(spec[i], 1e-10));
        linSum += spec[i];
    }
    double geoMean = qExp(logSum / specN);
    double arithMean = linSum / specN;
    f.spectralFlatness = (arithMean > 0) ? geoMean / arithMean : 0.0;

    // Zero crossing rate
    int zcr = 0;
    for (int i = 1; i < n; ++i)
        if ((signal[i] >= 0) != (signal[i - 1] >= 0)) zcr++;
    f.zeroCrossRate = static_cast<double>(zcr) / (n - 1);

    // RMS energy
    double rmsSum = 0.0;
    for (int i = 0; i < n; ++i) rmsSum += signal[i] * signal[i];
    f.rms = qSqrt(rmsSum / n);

    // Spectral skewness
    double mean = f.meanFreq;
    double m3 = 0.0, m2 = 0.0;
    for (int i = 0; i < specN; ++i) {
        double d = i - mean;
        m2 += d * d;
        m3 += d * d * d;
    }
    m2 /= specN; m3 /= specN;
    f.spectralSkew = (m2 > 1e-10) ? m3 / qPow(m2, 1.5) : 0.0;

    // MFCC
    QVector<double> melE = melEnergies(spec);
    for (int i = 0; i < melE.size(); ++i)
        melE[i] = qLn(qMax(melE[i], 1e-10));
    f.mfcc = dctII(melE, 13);

    return f;
}

/* ---- Flatten features to vector ---- */

QVector<double> SignalClassifier7::flattenFeatures(const Features& f) const
{
    QVector<double> v;
    v.append(f.meanFreq); v.append(f.peakFreq); v.append(f.bandwidth);
    v.append(f.spectralRolloff); v.append(f.spectralFlatness);
    v.append(f.zeroCrossRate); v.append(f.rms); v.append(f.spectralSkew);
    v.append(f.mfcc);
    return v;
}

/* ---- Feature distance ---- */

double SignalClassifier7::featureDistance(const Features& a, const Features& b) const
{
    QVector<double> va = flattenFeatures(a);
    QVector<double> vb = flattenFeatures(b);
    double sum = 0.0;
    for (int i = 0; i < qMin(va.size(), vb.size()); ++i) {
        double d = va[i] - vb[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Add template ---- */

void SignalClassifier7::addTemplate(const QString& label, const QVector<double>& signal)
{
    Template t;
    t.label = label;
    t.features = extractFeatures(signal);
    m_templates.append(t);
    m_stats.numTemplates = m_templates.size();
    m_stats.numClasses = classes().size();
}

/* ---- Classify ---- */

QString SignalClassifier7::classify(const QVector<double>& signal)
{
    auto probs = classifyWithProbability(signal);
    if (probs.isEmpty()) return QString();
    return probs[0].first;
}

/* ---- Classify with probability ---- */

QVector<QPair<QString, double>> SignalClassifier7::classifyWithProbability(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    Features f = extractFeatures(signal);

    // Find k nearest neighbors
    QVector<QPair<double, int>> dists;
    for (int i = 0; i < m_templates.size(); ++i)
        dists.append({featureDistance(f, m_templates[i].features), i});

    std::sort(dists.begin(), dists.end());
    int k = qMin(m_k, dists.size());

    // Distance-weighted voting
    QMap<QString, double> classWeights;
    double totalWeight = 0.0;
    for (int i = 0; i < k; ++i) {
        double d = qMax(dists[i].first, 1e-10);
        double w = 1.0 / (d * d);  // Inverse squared distance weighting
        classWeights[m_templates[dists[i].second].label] += w;
        totalWeight += w;
    }

    // Sort by weight
    QVector<QPair<QString, double>> results;
    for (auto it = classWeights.begin(); it != classWeights.end(); ++it)
        results.append({it.key(), it.value() / (totalWeight > 0 ? totalWeight : 1.0)});

    std::sort(results.begin(), results.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    m_stats.kNeighbors = k;
    m_stats.featureDim = flattenFeatures(f).size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    if (!results.isEmpty())
        emit classificationCompleted(results[0].first, results[0].second, timer.elapsed());

    return results;
}

/* ---- Get class labels ---- */

QVector<QString> SignalClassifier7::classes() const
{
    QVector<QString> cls;
    for (const auto& t : m_templates)
        if (!cls.contains(t.label)) cls.append(t.label);
    return cls;
}

/* ---- Reset ---- */

void SignalClassifier7::resetStatistics()
{
    m_templates.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
