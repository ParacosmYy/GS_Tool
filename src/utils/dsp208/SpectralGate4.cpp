/**
 * @file SpectralGate4.cpp
 * @brief SpectralGate4 实现
 *
 * 实现频谱门控：Wiener滤波增益、心理声学掩蔽阈值、STFT处理。
 */

#include "utils/dsp208/SpectralGate4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SpectralGate4::SpectralGate4(QObject *parent) : QObject(parent) {}
SpectralGate4::~SpectralGate4() = default;

/* ---- Configuration ---- */

void SpectralGate4::setFrameSize(int size) { m_frameSize = qMax(64, size); }
void SpectralGate4::setHopSize(int hop) { m_hopSize = qMax(1, hop); }
void SpectralGate4::setNoiseEstimateFrames(int frames) { m_noiseFrames = qMax(1, frames); }
void SpectralGate4::setMaskingOffset(double offset) { m_maskOffset = offset; }

/* ---- In-place radix-2 FFT ---- */

void SpectralGate4::fftImpl(QVector<QPair<double, double>>& data, bool inverse)
{
    int N = data.size();
    if (N <= 1) return;
    int bits = 0;
    while ((1 << bits) < N) ++bits;
    for (int i = 0; i < N; ++i) {
        int j = 0, x = i;
        for (int b = 0; b < bits; ++b) { j = (j << 1) | (x & 1); x >>= 1; }
        if (j > i) std::swap(data[i], data[j]);
    }
    for (int len = 2; len <= N; len <<= 1) {
        double angle = 2.0 * M_PI / len * (inverse ? -1.0 : 1.0);
        QPair<double, double> wn{qCos(angle), qSin(angle)};
        for (int i = 0; i < N; i += len) {
            QPair<double, double> w{1.0, 0.0};
            for (int j = 0; j < len / 2; ++j) {
                auto u = data[i + j];
                auto v = QPair<double,double>{
                    w.first * data[i+j+len/2].first - w.second * data[i+j+len/2].second,
                    w.first * data[i+j+len/2].second + w.second * data[i+j+len/2].first};
                data[i+j] = {u.first+v.first, u.second+v.second};
                data[i+j+len/2] = {u.first-v.first, u.second-v.second};
                double nw0 = w.first*wn.first - w.second*wn.second;
                double nw1 = w.first*wn.second + w.second*wn.first;
                w = {nw0, nw1};
            }
        }
    }
    if (inverse) for (auto& z : data) { z.first /= N; z.second /= N; }
}

/* ---- Hann window ---- */

QVector<double> SpectralGate4::hannWindow(int size)
{
    QVector<double> w(size);
    for (int i = 0; i < size; ++i)
        w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (size - 1)));
    return w;
}

/* ---- Frequency to Bark ---- */

double SpectralGate4::freqToBark(double freq) const
{
    return 13.0 * qAtan(0.00076 * freq) + 3.5 * qAtan(qPow(freq / 7500.0, 2));
}

/* ---- Spread function for masking ---- */

QVector<double> SpectralGate4::spreadFunction(const QVector<double>& excitation) const
{
    int N = excitation.size();
    QVector<double> spread(N, 0.0);
    // Simplified spreading: triangular convolution
    int spreadWidth = qMax(3, N / 32);
    for (int i = 0; i < N; ++i) {
        double sum = 0.0;
        for (int d = -spreadWidth; d <= spreadWidth; ++d) {
            int j = i + d;
            if (j >= 0 && j < N) {
                double weight = 1.0 - qAbs(d) / (spreadWidth + 1.0);
                sum += excitation[j] * weight;
            }
        }
        spread[i] = sum;
    }
    return spread;
}

/* ---- Estimate noise profile ---- */

void SpectralGate4::estimateNoiseProfile(const QVector<double>& signal)
{
    int noiseSamples = qMin(m_noiseFrames * m_hopSize, signal.size());
    auto window = hannWindow(m_frameSize);
    QVector<double> avgMag(m_frameSize / 2 + 1, 0.0);
    int count = 0;

    for (int pos = 0; pos + m_frameSize <= noiseSamples; pos += m_hopSize) {
        QVector<QPair<double, double>> frame(m_frameSize, {0.0, 0.0});
        for (int i = 0; i < m_frameSize; ++i)
            frame[i] = {signal[pos + i] * window[i], 0.0};
        fftImpl(frame, false);

        for (int k = 0; k <= m_frameSize / 2; ++k)
            avgMag[k] += qSqrt(frame[k].first * frame[k].first + frame[k].second * frame[k].second);
        ++count;
    }

    if (count > 0) {
        m_noiseProfile.resize(m_frameSize / 2 + 1);
        for (int k = 0; k <= m_frameSize / 2; ++k)
            m_noiseProfile[k] = avgMag[k] / count;
    }
}

/* ---- Compute masking threshold ---- */

QVector<double> SpectralGate4::computeMaskingThreshold(
    const QVector<QPair<double, double>>& spectrum) const
{
    int N = spectrum.size();
    QVector<double> threshold(N, 0.0);
    QVector<double> excitation(N, 0.0);

    // Compute excitation (power spectrum in dB)
    for (int k = 0; k < N; ++k) {
        double mag = qSqrt(spectrum[k].first * spectrum[k].first
                           + spectrum[k].second * spectrum[k].second);
        excitation[k] = qMax(mag, 1e-10);
    }

    // Apply spreading
    auto spread = spreadFunction(excitation);

    // Convert to threshold with offset
    for (int k = 0; k < N; ++k) {
        double freq = static_cast<double>(k) * m_sampleRate / (2 * N);
        double barkOffset = (freq > 0) ? m_maskOffset : m_maskOffset * 2;
        threshold[k] = spread[k] * qPow(10.0, barkOffset / 10.0);
    }
    return threshold;
}

/* ---- Compute Wiener gain ---- */

QVector<double> SpectralGate4::computeWienerGain(
    const QVector<QPair<double, double>>& spectrum) const
{
    int N = spectrum.size();
    QVector<double> gain(N, 1.0);
    if (m_noiseProfile.isEmpty()) return gain;

    for (int k = 0; k < N; ++k) {
        double mag2 = spectrum[k].first * spectrum[k].first
                       + spectrum[k].second * spectrum[k].second;
        double noise2 = (k < m_noiseProfile.size())
                         ? m_noiseProfile[k] * m_noiseProfile[k] : 0.0;
        // Wiener gain: max(0, 1 - noise/signal)
        double snr = mag2 / qMax(noise2, 1e-15);
        gain[k] = qMax(0.0, 1.0 - 1.0 / qMax(snr, 1.0));
        // Floor to avoid complete suppression
        gain[k] = qMax(gain[k], 0.01);
    }
    return gain;
}

/* ---- Process full signal ---- */

QVector<double> SpectralGate4::process(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();
    if (signal.isEmpty()) return {};

    // Auto-estimate noise if not done
    if (m_noiseProfile.isEmpty())
        estimateNoiseProfile(signal);

    int N = m_frameSize;
    int hop = m_hopSize;
    auto window = hannWindow(N);
    QVector<double> output(signal.size(), 0.0);
    QVector<double> overlap(signal.size(), 0.0);
    int numFrames = 0;

    for (int pos = 0; pos + N <= signal.size(); pos += hop) {
        // Windowed FFT
        QVector<QPair<double, double>> frame(N, {0.0, 0.0});
        for (int i = 0; i < N; ++i)
            frame[i] = {signal[pos + i] * window[i], 0.0};
        fftImpl(frame, false);

        // Compute combined gain: Wiener * masking threshold
        auto wienerGain = computeWienerGain(frame);
        auto maskThreshold = computeMaskingThreshold(frame);

        for (int k = 0; k <= N / 2; ++k) {
            double mag = qSqrt(frame[k].first * frame[k].first + frame[k].second * frame[k].second);
            double g = wienerGain[k];
            // Below masking threshold: suppress more
            if (mag < maskThreshold[k])
                g *= 0.1;
            frame[k] = {frame[k].first * g, frame[k].second * g};
            if (k > 0 && k < N / 2)
                frame[N - k] = {frame[k].first, -frame[k].second};
        }

        // IFFT + overlap-add
        fftImpl(frame, true);
        for (int i = 0; i < N && pos + i < output.size(); ++i) {
            output[pos + i] += frame[i].first * window[i];
            overlap[pos + i] += window[i] * window[i];
        }
        ++numFrames;
    }

    // Normalize by overlap
    for (int i = 0; i < output.size(); ++i)
        if (overlap[i] > 1e-10) output[i] /= overlap[i];

    m_stats.numFrames = numFrames;
    m_stats.frameSize = N;
    double noiseSum = 0.0;
    for (double v : m_noiseProfile) noiseSum += v;
    m_stats.avgNoiseFloor = (m_noiseProfile.size() > 0) ? noiseSum / m_noiseProfile.size() : 0.0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(numFrames, timer.elapsed());
    return output;
}

/* ---- Get noise profile ---- */

QVector<double> SpectralGate4::getNoiseProfile() const { return m_noiseProfile; }

/* ---- Reset ---- */

void SpectralGate4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_noiseProfile.clear();
}
