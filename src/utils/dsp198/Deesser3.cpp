/**
 * @file Deesser3.cpp
 * @brief Deesser3 实现
 *
 * 实现去齿音处理器：频谱平坦度分析、共振峰跟踪、自适应齿音抑制。
 */

#include "utils/dsp198/Deesser3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Deesser3::Deesser3(QObject *parent) : QObject(parent) {}
Deesser3::~Deesser3() = default;

/* ---- Configuration ---- */

void Deesser3::setThreshold(double db) { m_threshold = db; }
void Deesser3::setFrequencyRange(double lo, double hi) { m_freqLow = qMax(1000.0, lo); m_freqHigh = qMax(m_freqLow + 500.0, hi); }
void Deesser3::setReductionAmount(double db) { m_reduction = qBound(0.0, db, 30.0); }
void Deesser3::setAttackMs(double ms) { m_attackMs = qMax(0.1, ms); }
void Deesser3::setReleaseMs(double ms) { m_releaseMs = qMax(1.0, ms); }
void Deesser3::setSampleRate(double rate) { m_sampleRate = qMax(8000.0, rate); }
void Deesser3::setFftSize(int size) { m_fftSize = qBound(256, size, 8192); }

/* ---- Helpers ---- */

double Deesser3::coeffFromMs(double ms) const
{
    if (ms <= 0.0) return 1.0;
    return 1.0 - qExp(-1.0 / (ms * m_sampleRate / 1000.0));
}

QVector<double> Deesser3::hannWindow(int n)
{
    QVector<double> w(n);
    for (int i = 0; i < n; ++i)
        w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / n));
    return w;
}

int Deesser3::freqToBin(double hz) const
{
    return qBound(0, static_cast<int>(hz * m_fftSize / m_sampleRate), m_fftSize / 2);
}

/* ---- Internal FFT ---- */

void Deesser3::fft(QVector<double>& re, QVector<double>& im, bool inv) const
{
    int N = re.size();
    if (N <= 1) return;
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
    for (int len = 2; len <= N; len <<= 1) {
        double ang = (inv ? 2.0 : -2.0) * M_PI / len;
        double wR = qCos(ang), wI = qSin(ang);
        for (int i = 0; i < N; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uR = re[i+j], uI = im[i+j];
                double vR = re[i+j+len/2]*cR - im[i+j+len/2]*cI;
                double vI = re[i+j+len/2]*cI + im[i+j+len/2]*cR;
                re[i+j] = uR + vR; im[i+j] = uI + vI;
                re[i+j+len/2] = uR - vR; im[i+j+len/2] = uI - vI;
                double nR = cR*wR - cI*wI;
                cI = cR*wI + cI*wR; cR = nR;
            }
        }
    }
    if (inv) for (int i = 0; i < N; ++i) { re[i] /= N; im[i] /= N; }
}

/* ---- Spectral flatness ---- */

double Deesser3::spectralFlatness(const QVector<double>& mag, int loBin, int hiBin) const
{
    int lo = qMax(0, loBin), hi = qMin(mag.size() - 1, hiBin);
    if (hi <= lo) return 0.0;

    double logSum = 0.0, linSum = 0.0;
    int count = 0;
    for (int i = lo; i <= hi; ++i) {
        double v = qMax(mag[i], 1e-10);
        logSum += qLn(v);
        linSum += v;
        count++;
    }

    double geoMean = qExp(logSum / count);
    double arithMean = linSum / count;
    if (arithMean < 1e-10) return 0.0;
    return geoMean / arithMean;   // 0..1, high = flat (noise-like)
}

/* ---- Formant tracking ---- */

QPair<double, double> Deesser3::trackFormants(const QVector<double>& magnitude) const
{
    // Find two largest spectral peaks as formant candidates
    double f1 = 0.0, f2 = 0.0;
    double p1 = 0.0, p2 = 0.0;

    for (int i = 1; i < magnitude.size() - 1; ++i) {
        // Local peak detection
        if (magnitude[i] > magnitude[i-1] && magnitude[i] > magnitude[i+1]) {
            if (magnitude[i] > p1) {
                p2 = p1; f2 = f1;
                p1 = magnitude[i];
                f1 = i * m_sampleRate / m_fftSize;
            } else if (magnitude[i] > p2) {
                p2 = magnitude[i];
                f2 = i * m_sampleRate / m_fftSize;
            }
        }
    }
    return {f1, f2};
}

/* ---- Sibilance detection ---- */

Deesser3::DetectionResult Deesser3::detectSibilance(const QVector<double>& magnitude) const
{
    DetectionResult r;
    int loBin = freqToBin(m_freqLow);
    int hiBin = freqToBin(m_freqHigh);

    r.spectralFlatness = spectralFlatness(magnitude, loBin, hiBin);

    auto formants = trackFormants(magnitude);
    r.formantFreq1 = formants.first;
    r.formantFreq2 = formants.second;

    // Sibilance energy in target band
    r.sibilanceEnergy = 0.0;
    for (int i = loBin; i <= hiBin && i < magnitude.size(); ++i)
        r.sibilanceEnergy += magnitude[i] * magnitude[i];
    r.sibilanceEnergy = 10.0 * qLn(qMax(r.sibilanceEnergy, 1e-10)) / qLn(10.0);

    // Decision: high spectral flatness in sibilance band + energy above threshold
    r.isSibilant = (r.spectralFlatness > 0.4) && (r.sibilanceEnergy > m_threshold);

    if (r.isSibilant) {
        double excess = r.sibilanceEnergy - m_threshold;
        r.gainReduction = qMin(m_reduction, excess * 0.5);
    }
    return r;
}

/* ---- Process ---- */

QVector<double> Deesser3::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    int N = frame.size();
    if (N == 0) return frame;

    // Apply window and FFT
    int fftN = qMax(N, m_fftSize);
    QVector<double> re(fftN, 0.0), im(fftN, 0.0);
    auto win = hannWindow(N);
    for (int i = 0; i < N; ++i) re[i] = frame[i] * win[i];

    fft(re, im, false);

    // Compute magnitude
    QVector<double> mag(fftN / 2);
    for (int i = 0; i < fftN / 2; ++i)
        mag[i] = qSqrt(re[i]*re[i] + im[i]*im[i]);

    // Detect sibilance
    auto det = detectSibilance(mag);

    // Smooth gain envelope
    double targetGain = det.isSibilant ? qPow(10.0, -det.gainReduction / 20.0) : 1.0;
    double coeff = (det.isSibilant) ? coeffFromMs(m_attackMs) : coeffFromMs(m_releaseMs);
    m_envelope += coeff * (targetGain - m_envelope);
    m_envelope = qBound(qPow(10.0, -m_reduction / 20.0), m_envelope, 1.0);

    // Apply gain to sibilance band
    int loBin = freqToBin(m_freqLow);
    int hiBin = freqToBin(m_freqHigh);
    for (int i = loBin; i <= hiBin && i < fftN / 2; ++i) {
        double g = m_envelope;
        re[i] *= g; im[i] *= g;
        re[fftN - i] *= g; im[fftN - i] *= g;
    }

    // Inverse FFT
    fft(re, im, true);

    // Overlap-add style output
    QVector<double> output(N);
    for (int i = 0; i < N; ++i)
        output[i] = re[i] / (win[i] > 0 ? win[i] : 1.0);

    // Stats
    m_stats.totalFrames++;
    if (det.isSibilant) m_stats.sibilanceFrames++;
    m_totalReduction += det.gainReduction;
    m_stats.avgReduction = m_totalReduction / m_stats.totalFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    if (det.isSibilant)
        emit sibilanceDetected(det.formantFreq1, det.gainReduction, timer.elapsed());

    return output;
}

/* ---- Reset ---- */

void Deesser3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelope = 0.0;
    m_totalReduction = 0.0;
}
