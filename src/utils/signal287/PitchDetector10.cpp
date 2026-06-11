/**
 * @file PitchDetector10.cpp
 * @brief PitchDetector10 实现
 *
 * 实现基频检测器：谐波乘积谱与多基频估计的复调F0分析。
 */

#include "utils/signal287/PitchDetector10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PitchDetector10::PitchDetector10(QObject *parent)
    : QObject(parent) {}

PitchDetector10::~PitchDetector10() = default;

/* ---- Configuration ---- */

void PitchDetector10::setConfig(const PitchConfig& cfg)
{
    m_config = cfg;
    m_config.fftSize = qBound(256, cfg.fftSize, 16384);
    m_config.minFreq = qBound(20.0, cfg.minFreq, 5000.0);
    m_config.maxFreq = qBound(cfg.minFreq + 10.0, cfg.maxFreq, m_config.sampleRate / 2.0);
}

/* ---- Bit reverse ---- */

int PitchDetector10::bitReverse(int x, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

/* ---- In-place radix-2 FFT ---- */

void PitchDetector10::fft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    if (n <= 1) return;

    int bits = 0;
    while ((1 << bits) < n) ++bits;

    for (int i = 0; i < n; ++i) {
        int j = bitReverse(i, bits);
        if (j > i) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    for (int size = 2; size <= n; size <<= 1) {
        int half = size >> 1;
        double angleStep = -2.0 * M_PI / size;
        for (int i = 0; i < n; i += size) {
            for (int j = 0; j < half; ++j) {
                double angle = angleStep * j;
                double wr = qCos(angle), wi = qSin(angle);
                double tr = re[i + j + half] * wr - im[i + j + half] * wi;
                double ti = re[i + j + half] * wi + im[i + j + half] * wr;
                re[i + j + half] = re[i + j] - tr;
                im[i + j + half] = im[i + j] - ti;
                re[i + j] += tr;
                im[i + j] += ti;
            }
        }
    }
}

/* ---- Compute HPS ---- */

QVector<double> PitchDetector10::computeHPS(const QVector<double>& magnitude) const
{
    int N = magnitude.size();
    int hpsLen = N / m_config.numHarmonics;
    QVector<double> hps(hpsLen, 1.0);

    for (int h = 1; h <= m_config.numHarmonics; ++h) {
        for (int i = 0; i < hpsLen; ++i) {
            int idx = i * h;
            if (idx < N)
                hps[i] *= magnitude[idx];
        }
    }
    return hps;
}

/* ---- Find peaks ---- */

QVector<int> PitchDetector10::findPeaks(const QVector<double>& spectrum, int maxPeaks) const
{
    QVector<QPair<double, int>> peaks;
    for (int i = 1; i < spectrum.size() - 1; ++i) {
        if (spectrum[i] > spectrum[i - 1] && spectrum[i] > spectrum[i + 1])
            peaks.append(qMakePair(spectrum[i], i));
    }

    std::sort(peaks.begin(), peaks.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    QVector<int> result;
    for (int i = 0; i < qMin(peaks.size(), maxPeaks); ++i)
        result.append(peaks[i].second);
    return result;
}

/* ---- Bin/freq conversions ---- */

double PitchDetector10::binToFreq(int bin) const
{
    return static_cast<double>(bin) * m_config.sampleRate / m_config.fftSize;
}

int PitchDetector10::freqToBin(double freq) const
{
    return qBound(0, static_cast<int>(freq * m_config.fftSize / m_config.sampleRate),
                  m_config.fftSize / 2);
}

/* ---- Compute clarity ---- */

double PitchDetector10::computeClarity(const QVector<double>& magnitude,
                                        int fundBin, int numHarm) const
{
    if (fundBin <= 0 || fundBin >= magnitude.size()) return 0.0;

    double fundPower = magnitude[fundBin] * magnitude[fundBin];
    double harmPower = 0.0;
    int count = 0;

    for (int h = 2; h <= numHarm; ++h) {
        int hBin = fundBin * h;
        if (hBin < magnitude.size()) {
            harmPower += magnitude[hBin] * magnitude[hBin];
            count++;
        }
    }

    if (count == 0) return 0.0;
    double totalPower = fundPower + harmPower;
    return (totalPower > 0.0) ? fundPower / totalPower : 0.0;
}

/* ---- Spectral GCD for multi-pitch ---- */

QVector<double> PitchDetector10::spectralGCD(const QVector<double>& magnitude,
                                               const QVector<double>& freqAxis) const
{
    // Find prominent peaks in spectrum
    QVector<int> peaks = findPeaks(magnitude, 20);

    // Compute frequency ratios between peak pairs as GCD candidates
    QVector<double> f0Candidates;
    for (int i = 0; i < peaks.size(); ++i) {
        double f1 = freqAxis[peaks[i]];
        if (f1 < m_config.minFreq || f1 > m_config.maxFreq) continue;

        // Check if f1 could be a fundamental by checking harmonic series
        int numMatching = 0;
        for (int h = 2; h <= m_config.numHarmonics; ++h) {
            double fh = f1 * h;
            int fhBin = freqToBin(fh);
            if (fhBin < magnitude.size() && magnitude[fhBin] > 0.1 * magnitude[peaks[i]])
                numMatching++;
        }
        if (numMatching >= 1) f0Candidates.append(f1);
    }

    return f0Candidates;
}

/* ---- Detect single pitch ---- */

PitchDetector10::PitchResult PitchDetector10::detect(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    PitchResult result;
    int N = m_config.fftSize;
    int n = frame.size();

    // Apply window and zero-pad
    QVector<double> re(N, 0.0), im(N, 0.0);
    for (int i = 0; i < qMin(n, N); ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / qMin(n, N)));
        re[i] = frame[i] * w;
    }

    fft(re, im);

    // Compute magnitude spectrum
    int halfN = N / 2 + 1;
    QVector<double> mag(halfN);
    for (int i = 0; i < halfN; ++i)
        mag[i] = qSqrt(re[i] * re[i] + im[i] * im[i]) / N;

    // HPS
    QVector<double> hps = computeHPS(mag);
    result.hpsSpectrum = hps;

    // Search range
    int minBin = freqToBin(m_config.minFreq);
    int maxBin = qMin(freqToBin(m_config.maxFreq), hps.size() - 1);

    // Find peak in HPS within range
    int bestBin = minBin;
    double bestVal = 0.0;
    for (int i = minBin; i <= maxBin; ++i) {
        if (hps[i] > bestVal) {
            bestVal = hps[i];
            bestBin = i;
        }
    }

    result.primary.frequency = binToFreq(bestBin);
    result.primary.clarity = computeClarity(mag, bestBin, m_config.numHarmonics);
    result.primary.confidence = (bestVal > 0.0) ? qMin(bestVal, 1.0) : 0.0;
    result.primary.voiced = (result.primary.confidence >= m_config.confidenceThreshold
                             && result.primary.frequency >= m_config.minFreq);

    // Multi-pitch if enabled
    if (m_config.multiPitch) {
        QVector<double> freqAxis(halfN);
        for (int i = 0; i < halfN; ++i) freqAxis[i] = binToFreq(i);
        result.multipitch = detectMultiPitch(frame);
    }

    double elapsed = timer.elapsed();
    m_stats.fftSize = N;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit pitchDetected(0, result.primary.frequency, result.primary.confidence, elapsed);

    return result;
}

/* ---- Detect multi-pitch ---- */

QVector<PitchDetector10::PitchEstimate> PitchDetector10::detectMultiPitch(
    const QVector<double>& frame)
{
    int N = m_config.fftSize;
    QVector<double> re(N, 0.0), im(N, 0.0);
    int n = frame.size();
    for (int i = 0; i < qMin(n, N); ++i)
        re[i] = frame[i] * 0.5 * (1.0 - qCos(2.0 * M_PI * i / qMin(n, N)));

    fft(re, im);

    int halfN = N / 2 + 1;
    QVector<double> mag(halfN), freqAxis(halfN);
    for (int i = 0; i < halfN; ++i) {
        mag[i] = qSqrt(re[i] * re[i] + im[i] * im[i]) / N;
        freqAxis[i] = binToFreq(i);
    }

    QVector<double> candidates = spectralGCD(mag, freqAxis);

    QVector<PitchEstimate> estimates;
    for (double f : candidates) {
        PitchEstimate est;
        est.frequency = f;
        int bin = freqToBin(f);
        est.clarity = computeClarity(mag, bin, m_config.numHarmonics);
        est.confidence = (bin < halfN) ? qMin(mag[bin], 1.0) : 0.0;
        est.voiced = (est.confidence >= m_config.confidenceThreshold);
        estimates.append(est);
    }

    return estimates;
}

/* ---- Reset ---- */

void PitchDetector10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
