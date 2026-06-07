/**
 * @file SignalClassifier2.cpp
 * @brief SignalClassifier2 实现
 *
 * 实现信号分类：统计矩(偏度/峰度)、谱熵特征、多类别模板匹配。
 */

#include "utils/signal192/SignalClassifier2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SignalClassifier2::SignalClassifier2(QObject *parent) : QObject(parent) {}
SignalClassifier2::~SignalClassifier2() = default;

/* ---- Configuration ---- */

void SignalClassifier2::setSampleRate(double sr) { m_sampleRate = qMax(1.0, sr); }
void SignalClassifier2::setFftSize(int N) { m_fftSize = qMax(64, N); }

/* ---- Compute statistical moments ---- */

void SignalClassifier2::computeMoments(const QVector<double>& sig, Features& f) const
{
    int N = sig.size();
    if (N == 0) return;

    // Mean
    double sum = 0.0;
    for (double v : sig) sum += v;
    f.mean = sum / N;

    // Variance
    double varSum = 0.0;
    for (double v : sig) {
        double d = v - f.mean;
        varSum += d * d;
    }
    f.variance = varSum / N;

    // Standard deviation
    double sd = qSqrt(f.variance);
    if (sd < 1e-15) return;

    // Skewness (3rd moment)
    double skewSum = 0.0;
    for (double v : sig) {
        double d = (v - f.mean) / sd;
        skewSum += d * d * d;
    }
    f.skewness = skewSum / N;

    // Kurtosis (4th moment, excess)
    double kurtSum = 0.0;
    for (double v : sig) {
        double d = (v - f.mean) / sd;
        kurtSum += d * d * d * d;
    }
    f.kurtosis = kurtSum / N - 3.0; // Excess kurtosis

    // RMS
    double rmsSum = 0.0;
    for (double v : sig) rmsSum += v * v;
    f.rms = qSqrt(rmsSum / N);

    // Zero-crossing rate
    int crossings = 0;
    for (int i = 1; i < N; ++i) {
        if ((sig[i] >= 0.0 && sig[i - 1] < 0.0) ||
            (sig[i] < 0.0 && sig[i - 1] >= 0.0))
            crossings++;
    }
    f.zeroCrossRate = static_cast<double>(crossings) / (N - 1);

    // Peak factor (crest factor)
    double peak = 0.0;
    for (double v : sig) peak = qMax(peak, qAbs(v));
    f.peakFactor = (f.rms > 1e-15) ? peak / f.rms : 0.0;
}

/* ---- Compute FFT magnitude spectrum ---- */

QVector<double> SignalClassifier2::computeSpectrum(const QVector<double>& sig) const
{
    int N = m_fftSize;
    QVector<double> re(N, 0.0), im(N, 0.0);
    int len = qMin(sig.size(), N);
    for (int i = 0; i < len; ++i) {
        // Apply Hann window
        double win = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (N - 1)));
        re[i] = sig[i] * win;
    }

    // Cooley-Tukey radix-2 FFT
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }

    for (int l = 2; l <= N; l *= 2) {
        double angle = -2.0 * M_PI / l;
        for (int i = 0; i < N; i += l) {
            double cRe = 1.0, cIm = 0.0;
            double wRe = qCos(angle), wIm = qSin(angle);
            for (int j = 0; j < l / 2; ++j) {
                int u = i + j, v = i + j + l / 2;
                double tRe = cRe * re[v] - cIm * im[v];
                double tIm = cRe * im[v] + cIm * re[v];
                re[v] = re[u] - tRe; im[v] = im[u] - tIm;
                re[u] += tRe; im[u] += tIm;
                double nr = cRe * wRe - cIm * wIm;
                cIm = cRe * wIm + cIm * wRe;
                cRe = nr;
            }
        }
    }

    int halfN = N / 2;
    QVector<double> mag(halfN);
    for (int k = 0; k < halfN; ++k)
        mag[k] = qSqrt(re[k] * re[k] + im[k] * im[k]);
    return mag;
}

/* ---- Compute spectral entropy ---- */

double SignalClassifier2::computeSpectralEntropy(const QVector<double>& power) const
{
    double total = 0.0;
    for (double p : power) total += p;
    if (total < 1e-15) return 0.0;

    double entropy = 0.0;
    for (double p : power) {
        double prob = p / total;
        if (prob > 1e-15)
            entropy -= prob * qLn(prob);
    }

    // Normalize to [0, 1]
    double maxEntropy = qLn(static_cast<double>(power.size()));
    return (maxEntropy > 1e-15) ? entropy / maxEntropy : 0.0;
}

/* ---- Extract features ---- */

SignalClassifier2::Features SignalClassifier2::extractFeatures(
    const QVector<double>& signal) const
{
    Features f;
    computeMoments(signal, f);

    auto spectrum = computeSpectrum(signal);

    // Power spectrum
    QVector<double> power(spectrum.size());
    for (int i = 0; i < spectrum.size(); ++i)
        power[i] = spectrum[i] * spectrum[i];

    // Spectral entropy
    f.spectralEntropy = computeSpectralEntropy(power);

    // Spectral centroid
    double totalPower = 0.0, weightedSum = 0.0;
    for (int k = 0; k < power.size(); ++k) {
        double freq = static_cast<double>(k) * m_sampleRate / m_fftSize;
        weightedSum += freq * power[k];
        totalPower += power[k];
    }
    f.spectralCentroid = (totalPower > 1e-15) ? weightedSum / totalPower : 0.0;

    // Spectral flatness (geometric mean / arithmetic mean)
    double geoSum = 0.0, ariSum = 0.0;
    int validCount = 0;
    for (double p : power) {
        if (p > 1e-15) {
            geoSum += qLn(p);
            ariSum += p;
            validCount++;
        }
    }
    if (validCount > 0 && ariSum > 1e-15) {
        double geoMean = qExp(geoSum / validCount);
        double ariMean = ariSum / validCount;
        f.spectralFlatness = geoMean / ariMean;
    }

    return f;
}

/* ---- Score templates ---- */

QVector<QPair<SignalClassifier2::SignalType, double>> SignalClassifier2::scoreTemplates(
    const Features& f) const
{
    QVector<QPair<SignalType, double>> scores;

    // Sine: low kurtosis (~-1.5), low spectral entropy
    double sineScore = qExp(-qAbs(f.kurtosis + 1.5)) * (1.0 - f.spectralEntropy);
    scores.append({SineWave, sineScore});

    // Square: high kurtosis, high zero-cross rate
    double squareScore = qExp(-qAbs(f.kurtosis - 1.0)) * f.zeroCrossRate;
    scores.append({SquareWave, squareScore});

    // Triangle: moderate negative kurtosis
    double triScore = qExp(-qAbs(f.kurtosis + 1.2)) * f.zeroCrossRate;
    scores.append({TriangleWave, triScore});

    // Sawtooth: moderate kurtosis
    double sawScore = qExp(-qAbs(f.kurtosis + 0.6)) * f.zeroCrossRate;
    scores.append({SawtoothWave, sawScore});

    // White noise: high spectral entropy, low peak factor
    double noiseScore = f.spectralEntropy * f.spectralFlatness;
    scores.append({WhiteNoise, noiseScore});

    // Impulse: very high peak factor
    double impScore = qExp(-(f.peakFactor - 5.0) * (f.peakFactor - 5.0) / 4.0);
    scores.append({Impulse, impScore});

    // AM: moderate spectral entropy
    double amScore = (1.0 - f.spectralEntropy) * (1.0 - qAbs(f.kurtosis));
    scores.append({AmModulated, qMax(0.0, amScore)});

    // FM: moderate spectral entropy with wide spread
    double fmScore = f.spectralEntropy * (1.0 - qAbs(f.peakFactor - 1.4));
    scores.append({FmModulated, qMax(0.0, fmScore)});

    // Sort descending by score
    std::sort(scores.begin(), scores.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    return scores;
}

/* ---- Classify ---- */

SignalClassifier2::Result SignalClassifier2::classify(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    Features f = extractFeatures(signal);
    auto scores = scoreTemplates(f);

    Result result;
    result.scores = scores;
    if (!scores.isEmpty()) {
        result.type = scores[0].first;
        // Normalize confidence
        double totalScore = 0.0;
        for (const auto& s : scores) totalScore += s.second;
        result.confidence = (totalScore > 1e-15) ? scores[0].second / totalScore : 0.0;
    }

    m_stats.totalClassifications++;
    m_stats.signalLength = signal.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClassifications;

    emit classificationCompleted(static_cast<int>(result.type), result.confidence,
                                  timer.elapsed());
    return result;
}

/* ---- Signal type name ---- */

QString SignalClassifier2::signalTypeName(SignalType type)
{
    switch (type) {
    case SineWave:     return QStringLiteral("SineWave");
    case SquareWave:   return QStringLiteral("SquareWave");
    case TriangleWave: return QStringLiteral("TriangleWave");
    case SawtoothWave: return QStringLiteral("SawtoothWave");
    case WhiteNoise:   return QStringLiteral("WhiteNoise");
    case Impulse:      return QStringLiteral("Impulse");
    case AmModulated:  return QStringLiteral("AM");
    case FmModulated:  return QStringLiteral("FM");
    default:           return QStringLiteral("Unknown");
    }
}

/* ---- Reset ---- */

void SignalClassifier2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
