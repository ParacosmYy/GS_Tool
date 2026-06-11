/**
 * @file ZoomFFT10.cpp
 * @brief ZoomFFT10 实现
 *
 * 实现缩放FFT：级联复频移与抽取链实现多级窄带频谱分析。
 */

#include "utils/fft299/ZoomFFT10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ZoomFFT10::ZoomFFT10(QObject *parent)
    : QObject(parent) {}

ZoomFFT10::~ZoomFFT10() = default;

/* ---- Configuration ---- */

void ZoomFFT10::setConfig(const ZoomConfig& config)
{
    m_config = config;
    m_config.centerFreqHz = qBound(0.0, config.centerFreqHz, config.sampleRateHz / 2.0);
    m_config.bandwidthHz = qBound(1.0, config.bandwidthHz, config.sampleRateHz / 2.0);
    m_config.sampleRateHz = qBound(1.0, config.sampleRateHz, 1e8);
    m_config.fftSize = qBound(16, config.fftSize, 1 << 20);
    m_config.decimationStages = qBound(1, config.decimationStages, 6);
    m_config.decimationFactor = qBound(2, config.decimationFactor, 16);
}

/* ---- Complex frequency shift: heterodyne centerFreq to DC ---- */

QVector<double> ZoomFFT10::frequencyShift(const QVector<double>& input,
                                            double freq, double fs) const
{
    int n = input.size();
    QVector<double> shifted(n);
    double phaseInc = -2.0 * M_PI * freq / fs;
    double phase = 0.0;

    for (int i = 0; i < n; ++i) {
        double cosVal = qCos(phase);
        shifted[i] = input[i] * cosVal;
        phase += phaseInc;
        if (phase > M_PI) phase -= 2.0 * M_PI;
        if (phase < -M_PI) phase += 2.0 * M_PI;
    }
    return shifted;
}

/* ---- Lowpass FIR via windowed sinc ---- */

QVector<double> ZoomFFT10::lowpassFilter(const QVector<double>& input,
                                           double cutoff, double fs) const
{
    int n = input.size();
    int halfLen = 32; // filter half-length
    int filtLen = 2 * halfLen + 1;
    double wc = 2.0 * cutoff / fs;

    // Precompute filter kernel
    QVector<double> kernel(filtLen);
    for (int k = 0; k < filtLen; ++k) {
        int j = k - halfLen;
        double sinc = (j == 0) ? wc : qSin(M_PI * wc * j) / (M_PI * j);
        double hann = 0.5 * (1.0 - qCos(2.0 * M_PI * k / (filtLen - 1)));
        kernel[k] = wc * sinc * hann;
    }

    QVector<double> filtered(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < filtLen; ++k) {
            int srcIdx = i - halfLen + k;
            if (srcIdx >= 0 && srcIdx < n)
                sum += input[srcIdx] * kernel[k];
        }
        filtered[i] = sum;
    }
    return filtered;
}

/* ---- Downsample by integer factor ---- */

QVector<double> ZoomFFT10::downsample(const QVector<double>& input, int factor) const
{
    int n = input.size() / factor;
    QVector<double> out;
    out.reserve(n);
    for (int i = 0; i < n; ++i)
        out.append(input[i * factor]);
    return out;
}

/* ---- Cascaded decimation: repeat lowpass + downsample ---- */

QVector<double> ZoomFFT10::decimate(const QVector<double>& input,
                                      int factor, int stages) const
{
    QVector<double> current = input;
    double currentFs = m_config.sampleRateHz;

    for (int s = 0; s < stages; ++s) {
        // Lowpass at half the post-decimation Nyquist
        double postFs = currentFs / factor;
        double cutoff = postFs / 2.0;
        current = lowpassFilter(current, cutoff, currentFs);
        current = downsample(current, factor);
        currentFs = postFs;
    }
    return current;
}

/* ---- In-place radix-2 FFT on interleaved [re, im, re, im, ...] ---- */

void ZoomFFT10::fft(QVector<double>& reIm) const
{
    int N = reIm.size() / 2;
    if (N <= 1) return;

    // Bit-reversal permutation
    int j = 0;
    for (int i = 1; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(reIm[2 * i], reIm[2 * j]);
            std::swap(reIm[2 * i + 1], reIm[2 * j + 1]);
        }
    }

    // Butterfly stages
    for (int len = 2; len <= N; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wRe = qCos(ang);
        double wIm = qSin(ang);

        for (int i = 0; i < N; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int k = 0; k < len / 2; ++k) {
                int idx1 = 2 * (i + k);
                int idx2 = 2 * (i + k + len / 2);

                double tRe = curRe * reIm[idx2] - curIm * reIm[idx2 + 1];
                double tIm = curRe * reIm[idx2 + 1] + curIm * reIm[idx2];

                reIm[idx2] = reIm[idx1] - tRe;
                reIm[idx2 + 1] = reIm[idx1 + 1] - tIm;
                reIm[idx1] += tRe;
                reIm[idx1 + 1] += tIm;

                double newCurRe = curRe * wRe - curIm * wIm;
                double newCurIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
                curIm = newCurIm;
            }
        }
    }
}

/* ---- Frequency axis for zoomed spectrum ---- */

QVector<double> ZoomFFT10::frequencyAxis(int nBins) const
{
    double totalDecim = qPow(m_config.decimationFactor, m_config.decimationStages);
    double effectiveFs = m_config.sampleRateHz / totalDecim;
    double df = effectiveFs / m_config.fftSize;

    QVector<double> freqs(nBins);
    for (int i = 0; i < nBins; ++i) {
        freqs[i] = m_config.centerFreqHz + (i - nBins / 2.0) * df;
    }
    return freqs;
}

/* ---- Main analyze ---- */

ZoomFFT10::SpectrumResult ZoomFFT10::analyze(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    SpectrumResult result;
    int n = input.size();
    if (n == 0) return result;

    // Step 1: Frequency shift center to DC
    auto shifted = frequencyShift(input, m_config.centerFreqHz, m_config.sampleRateHz);

    // Step 2: Cascaded decimation
    auto decimated = decimate(shifted, m_config.decimationFactor, m_config.decimationStages);

    // Step 3: Window + FFT
    int N = m_config.fftSize;
    int decN = decimated.size();

    // Pack into interleaved complex array with Hann window
    QVector<double> reIm(2 * N, 0.0);
    for (int i = 0; i < qMin(decN, N); ++i) {
        double win = 0.5 * (1.0 - qCos(2.0 * M_PI * i / N));
        reIm[2 * i] = decimated[i] * win;
        reIm[2 * i + 1] = 0.0;
    }

    fft(reIm);

    // Extract magnitude and phase
    int halfN = N / 2;
    result.numBins = halfN;
    result.magnitude.resize(halfN);
    result.phase.resize(halfN);

    for (int i = 0; i < halfN; ++i) {
        double re = reIm[2 * i];
        double im = reIm[2 * i + 1];
        result.magnitude[i] = 20.0 * qLog10(qMax(1e-10, qSqrt(re * re + im * im) / N));
        result.phase[i] = qAtan2(im, re);
    }

    // Frequency axis
    double totalDecim = qPow(m_config.decimationFactor, m_config.decimationStages);
    double effectiveFs = m_config.sampleRateHz / totalDecim;
    result.binResolutionHz = effectiveFs / N;
    result.frequencies = frequencyAxis(halfN);

    m_stats.totalTransforms++;
    m_stats.effectiveFFTSize = N;
    m_stats.freqResolution = result.binResolutionHz;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit analyzeDone(result.numBins, result.binResolutionHz, elapsed);
    return result;
}

/* ---- Reset ---- */

void ZoomFFT10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
