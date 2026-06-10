/**
 * @file ZoomFFT8.cpp
 * @brief ZoomFFT8 实现
 *
 * 实现缩放FFT：多级抽取与复数解调的超窄带高分辨率频谱分析。
 */

#include "utils/fft285/ZoomFFT8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ZoomFFT8::ZoomFFT8(QObject *parent)
    : QObject(parent)
{
    designLPF();
}

ZoomFFT8::~ZoomFFT8() = default;

/* ---- Configuration ---- */

void ZoomFFT8::setConfig(const ZoomConfig& cfg)
{
    m_config = cfg;
    m_config.fftSize = qBound(64, cfg.fftSize, 65536);
    m_config.decimationStages = qBound(1, cfg.decimationStages, 8);
    m_config.decimationFactor = qBound(2, cfg.decimationFactor, 8);
    m_config.centerFreq = qBound(0.0, cfg.centerFreq, 0.5);
    m_config.bandwidth = qBound(0.001, cfg.bandwidth, 0.5);
    m_phaseIndex = 0;
    designLPF();
}

/* ---- Design decimation LPF ---- */

void ZoomFFT8::designLPF()
{
    int order = 63;
    int D = m_config.decimationFactor;
    double cutoff = 1.0 / static_cast<double>(D) * 0.8;

    m_lpfCoeffs.resize(order + 1);
    double sum = 0.0;

    for (int i = 0; i <= order; ++i) {
        int center = order / 2;
        double n = static_cast<double>(i - center);
        double x = M_PI * cutoff * n;
        double sinc = (qAbs(n) < 1e-10) ? 1.0 : qSin(x) / x;

        // Hamming window
        double w = 0.54 - 0.46 * qCos(2.0 * M_PI * i / order);

        m_lpfCoeffs[i] = sinc * w;
        sum += m_lpfCoeffs[i];
    }

    for (auto& c : m_lpfCoeffs)
        c /= sum;
}

/* ---- Apply FIR filter ---- */

QVector<double> ZoomFFT8::applyFIR(const QVector<double>& input,
                                     const QVector<double>& coeffs) const
{
    int n = input.size();
    int order = coeffs.size();
    QVector<double> out(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double acc = 0.0;
        for (int k = 0; k < order; ++k) {
            int idx = i - k;
            if (idx >= 0 && idx < n)
                acc += coeffs[k] * input[idx];
        }
        out[i] = acc;
    }
    return out;
}

/* ---- Downsample ---- */

QVector<double> ZoomFFT8::downsample(const QVector<double>& input, int D) const
{
    QVector<double> out;
    out.reserve(input.size() / D);
    for (int i = 0; i < input.size(); i += D)
        out.append(input[i]);
    return out;
}

/* ---- Complex demodulation ---- */

QVector<double> ZoomFFT8::demodulate(const QVector<double>& input)
{
    int n = input.size();
    QVector<double> complexOut(n * 2);  // [re, im, re, im, ...]

    for (int i = 0; i < n; ++i) {
        double phase = -2.0 * M_PI * m_config.centerFreq * m_phaseIndex;
        complexOut[i * 2] = input[i] * qCos(phase);      // Real
        complexOut[i * 2 + 1] = input[i] * qSin(phase);  // Imag
        ++m_phaseIndex;
    }
    return complexOut;
}

/* ---- Multi-stage decimation ---- */

QVector<double> ZoomFFT8::decimate(const QVector<double>& input, int stages)
{
    // Split into real/imag
    int n = input.size() / 2;
    QVector<double> re(n), im(n);
    for (int i = 0; i < n; ++i) {
        re[i] = input[i * 2];
        im[i] = input[i * 2 + 1];
    }

    for (int s = 0; s < stages; ++s) {
        re = applyFIR(re, m_lpfCoeffs);
        im = applyFIR(im, m_lpfCoeffs);
        re = downsample(re, m_config.decimationFactor);
        im = downsample(im, m_config.decimationFactor);
    }

    // Interleave back
    int outN = re.size();
    QVector<double> result(outN * 2);
    for (int i = 0; i < outN; ++i) {
        result[i * 2] = re[i];
        result[i * 2 + 1] = im[i];
    }
    return result;
}

/* ---- Bit reverse ---- */

int ZoomFFT8::bitReverse(int x, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

/* ---- In-place radix-2 FFT ---- */

void ZoomFFT8::fft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    if (n <= 1) return;

    int bits = 0;
    while ((1 << bits) < n) ++bits;

    // Bit-reversal permutation
    for (int i = 0; i < n; ++i) {
        int j = bitReverse(i, bits);
        if (j > i) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    // Butterfly stages
    for (int size = 2; size <= n; size <<= 1) {
        int half = size >> 1;
        double angleStep = -2.0 * M_PI / size;

        for (int i = 0; i < n; i += size) {
            for (int j = 0; j < half; ++j) {
                double angle = angleStep * j;
                double wr = qCos(angle);
                double wi = qSin(angle);

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

/* ---- Main analysis ---- */

ZoomFFT8::ZoomResult ZoomFFT8::analyze(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ZoomResult result;
    int n = input.size();
    if (n == 0) return result;

    // Step 1: Complex demodulation (frequency shift to baseband)
    QVector<double> demod = demodulate(input);

    // Step 2: Multi-stage decimation
    int totalDec = 1;
    for (int s = 0; s < m_config.decimationStages; ++s)
        totalDec *= m_config.decimationFactor;

    QVector<double> decimated = decimate(demod, m_config.decimationStages);

    int fftN = m_config.fftSize;
    int decN = decimated.size() / 2;

    // Zero-pad or truncate to FFT size
    QVector<double> re(fftN, 0.0), im(fftN, 0.0);
    int copyN = qMin(decN, fftN);
    for (int i = 0; i < copyN; ++i) {
        re[i] = decimated[i * 2];
        im[i] = decimated[i * 2 + 1];
    }

    // Step 3: FFT
    fft(re, im);

    // Compute magnitude and phase
    result.magnitude.resize(fftN);
    result.phase.resize(fftN);
    result.freqAxis.resize(fftN);

    double totalDecFactor = static_cast<double>(totalDec);
    double freqRes = (m_config.bandwidth) / fftN;
    result.resolution = freqRes;

    double maxMag = 0.0;
    double noiseFloor = 0.0;

    for (int i = 0; i < fftN; ++i) {
        double mag = qSqrt(re[i] * re[i] + im[i] * im[i]) / fftN;
        result.magnitude[i] = mag;
        result.phase[i] = qAtan2(im[i], re[i]);

        // Frequency axis centered on zoom band
        double f = m_config.centerFreq - m_config.bandwidth / 2.0
                   + static_cast<double>(i) * freqRes;
        result.freqAxis[i] = f;

        if (mag > maxMag) maxMag = mag;
        noiseFloor += mag * mag;
    }

    // SNR estimation
    noiseFloor = (noiseFloor - maxMag * maxMag) / qMax(fftN - 1, 1);
    result.snr = (maxMag > 0.0 && noiseFloor > 0.0)
                 ? 10.0 * qLog10(maxMag * maxMag / noiseFloor)
                 : 0.0;

    double elapsed = timer.elapsed();
    m_stats.fftSize = fftN;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit analyzeDone(n, m_config.bandwidth, freqRes, elapsed);

    return result;
}

/* ---- Reset ---- */

void ZoomFFT8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_phaseIndex = 0;
}
