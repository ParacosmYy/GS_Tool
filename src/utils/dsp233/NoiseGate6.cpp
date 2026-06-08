/**
 * @file NoiseGate6.cpp
 * @brief NoiseGate6 实现
 *
 * 实现噪声门：最小统计谱底估计与心理声学掩蔽阈值门控。
 */

#include "utils/dsp233/NoiseGate6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

NoiseGate6::NoiseGate6(QObject *parent) : QObject(parent) {}
NoiseGate6::~NoiseGate6() = default;

/* ---- Configure ---- */

bool NoiseGate6::configure(int frameSize, int fftSize)
{
    if (frameSize < 16) return false;
    m_frameSize = frameSize;
    m_fftSize = (fftSize > 0) ? fftSize : frameSize;
    // Round up to next power of 2
    int n = 1;
    while (n < m_fftSize) n <<= 1;
    m_fftSize = n;
    m_halfFft = m_fftSize / 2;

    m_psd.resize(m_halfFft, 0.0);
    m_noiseEstimate.resize(m_halfFft, 0.0);
    m_psdMin.resize(m_halfFft, 1e10);
    m_psdMinHist.resize(m_halfFft * m_minHistLen, 1e10);
    m_maskThreshold.resize(m_halfFft, 0.0);

    precompute();
    initSpreadingFunction();

    m_stats.frameSize = m_frameSize;
    m_stats.fftSize = m_fftSize;
    return true;
}

/* ---- Precompute window and twiddles ---- */

void NoiseGate6::precompute()
{
    int n = m_fftSize;
    m_window.resize(m_frameSize);
    for (int i = 0; i < m_frameSize; ++i)
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_frameSize));

    m_twReal.resize(n);
    m_twImag.resize(n);
    for (int k = 0; k < n; ++k) {
        double angle = -2.0 * M_PI * k / n;
        m_twReal[k] = qCos(angle);
        m_twImag[k] = qSin(angle);
    }
}

/* ---- Spreading function (simplified Barker model) ---- */

void NoiseGate6::initSpreadingFunction()
{
    m_spreadFn.resize(m_halfFft, 0.0);
    int spreadWidth = qMax(1, m_halfFft / 32);
    for (int i = 0; i < m_halfFft; ++i) {
        int dist = (i < spreadWidth) ? i : spreadWidth;
        m_spreadFn[i] = qExp(-dist / (spreadWidth * 0.5));
    }
}

/* ---- Apply Hann window ---- */

void NoiseGate6::applyWindow(QVector<double>& frame) const
{
    int n = qMin(frame.size(), m_window.size());
    for (int i = 0; i < n; ++i)
        frame[i] *= m_window[i];
}

/* ---- In-place radix-2 FFT ---- */

void NoiseGate6::fft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    // Bit-reverse permutation
    int bits = 0;
    while ((1 << bits) < n) bits++;
    for (int i = 0; i < n; ++i) {
        int j = 0;
        for (int b = 0; b < bits; ++b)
            if (i & (1 << b)) j |= (1 << (bits - 1 - b));
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
    // Butterfly stages
    for (int len = 2; len <= n; len <<= 1) {
        int half = len >> 1;
        int step = n / len;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < half; ++j) {
                int twIdx = j * step;
                double tR = m_twReal[twIdx] * re[i + j + half]
                            - m_twImag[twIdx] * im[i + j + half];
                double tI = m_twReal[twIdx] * im[i + j + half]
                            + m_twImag[twIdx] * re[i + j + half];
                re[i + j + half] = re[i + j] - tR;
                im[i + j + half] = im[i + j] - tI;
                re[i + j] += tR;
                im[i + j] += tI;
            }
        }
    }
}

/* ---- In-place IFFT ---- */

void NoiseGate6::ifft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    for (int i = 0; i < n; ++i) im[i] = -im[i];
    fft(re, im);
    for (int i = 0; i < n; ++i) {
        re[i] /= n;
        im[i] = -im[i] / n;
    }
}

/* ---- Update minimum statistics ---- */

void NoiseGate6::updateMinStatistics(const QVector<double>& psd)
{
    int bins = m_halfFft;
    // Update running minimum
    for (int b = 0; b < bins; ++b) {
        m_psdMin[b] = qMin(m_psdMin[b], psd[b]);
    }

    // Store in circular history buffer
    int offset = m_minHistIdx * bins;
    for (int b = 0; b < bins; ++b)
        m_psdMinHist[offset + b] = m_psdMin[b];

    m_minHistIdx = (m_minHistIdx + 1) % m_minHistLen;

    // Compute noise estimate as minimum over history
    for (int b = 0; b < bins; ++b) {
        double minVal = 1e18;
        for (int h = 0; h < m_minHistLen; ++h)
            minVal = qMin(minVal, m_psdMinHist[h * bins + b]);
        // Bias correction factor for minimum statistics
        m_noiseEstimate[b] = minVal * 1.5;
        // Reset running min periodically
        m_psdMin[b] = psd[b];
    }
}

/* ---- Compute psychoacoustic masking threshold ---- */

void NoiseGate6::computeMaskingThreshold(const QVector<double>& psd)
{
    int bins = m_halfFft;
    // Simplified spreading: convolve PSD with spreading function
    QVector<double> spread(bins, 0.0);
    int spreadLen = m_spreadFn.size();
    for (int b = 0; b < bins; ++b) {
        double sum = 0.0;
        for (int s = 0; s < spreadLen; ++s) {
            int idx = b + s;
            if (idx < bins) sum += psd[idx] * m_spreadFn[s];
        }
        spread[b] = sum;
    }

    // Masking threshold = spread energy with offset (simplified)
    double offset = qLn(1e6); // ~60 dB offset
    for (int b = 0; b < bins; ++b) {
        double mask = spread[b] * qExp(-offset);
        // Combine with absolute threshold of hearing
        m_maskThreshold[b] = qMax(mask, m_absThreshold);
    }

    // Average for stats
    double avgMask = 0.0;
    for (int b = 0; b < bins; ++b) avgMask += m_maskThreshold[b];
    m_stats.avgMaskThreshold = avgMask / bins;
}

/* ---- Process single frame ---- */

QVector<double> NoiseGate6::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_fftSize;
    QVector<double> re(n, 0.0), im(n, 0.0);

    // Window and zero-pad
    int len = qMin(frame.size(), m_frameSize);
    for (int i = 0; i < len; ++i)
        re[i] = frame[i] * m_window[i];

    // Forward FFT
    fft(re, im);

    // Compute PSD
    for (int b = 0; b < m_halfFft; ++b)
        m_psd[b] = re[b] * re[b] + im[b] * im[b];

    // Update noise floor estimate via minimum statistics
    updateMinStatistics(m_psd);

    // Compute masking thresholds
    computeMaskingThreshold(m_psd);

    // Gating decision: compare PSD to max(noiseEstimate, maskThreshold)
    int gated = 0;
    double noiseFloorSum = 0.0;
    for (int b = 0; b < m_halfFft; ++b) {
        double threshold = qMax(m_noiseEstimate[b], m_maskThreshold[b]);
        noiseFloorSum += m_noiseEstimate[b];
        if (m_psd[b] < threshold) {
            // Attenuate by factor 0.01 (-40 dB)
            re[b] *= 0.01;
            im[b] *= 0.01;
            if (b > 0 && b < m_halfFft) {
                re[n - b] *= 0.01;
                im[n - b] *= 0.01;
            }
            gated++;
        }
    }

    m_stats.noiseFloorDb = 10.0 * qLn(noiseFloorSum / m_halfFft + 1e-30) / qLn(10.0);

    // Inverse FFT
    ifft(re, im);

    // Extract real part, apply window again for overlap-add synthesis
    QVector<double> output(m_frameSize);
    for (int i = 0; i < m_frameSize; ++i)
        output[i] = re[i] * m_window[i];

    m_stats.framesProcessed++;
    m_stats.framesGated += (gated > m_halfFft / 2) ? 1 : 0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    double snr = 0.0;
    if (noiseFloorSum > 0) {
        double sigSum = 0.0;
        for (int b = 0; b < m_halfFft; ++b) sigSum += m_psd[b];
        snr = 10.0 * qLn((sigSum / noiseFloorSum) + 1e-30) / qLn(10.0);
    }

    emit frameProcessed(m_stats.framesProcessed, snr, gated > m_halfFft / 2);
    return output;
}

/* ---- Process buffer ---- */

QVector<double> NoiseGate6::processBuffer(const QVector<double>& buffer)
{
    QElapsedTimer timer;
    timer.start();

    int total = buffer.size();
    int hop = m_frameSize / 2;
    QVector<double> output(total, 0.0);
    QVector<double> frame(m_frameSize);
    int frames = 0;
    int gatedCount = 0;

    for (int pos = 0; pos + m_frameSize <= total; pos += hop) {
        for (int i = 0; i < m_frameSize; ++i)
            frame[i] = buffer[pos + i];

        QVector<double> gated = process(frame);
        for (int i = 0; i < m_frameSize && pos + i < total; ++i)
            output[pos + i] += gated[i];
        frames++;
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalOps);

    emit bufferCompleted(frames, gatedCount, timer.elapsed());
    return output;
}

/* ---- Accessors ---- */

QVector<double> NoiseGate6::noiseFloor() const { return m_noiseEstimate; }
QVector<double> NoiseGate6::maskingThresholds() const { return m_maskThreshold; }

/* ---- Reset ---- */

void NoiseGate6::resetStatistics()
{
    m_psd.clear();
    m_noiseEstimate.clear();
    m_psdMin.clear();
    m_psdMinHist.clear();
    m_maskThreshold.clear();
    m_spreadFn.clear();
    m_window.clear();
    m_twReal.clear();
    m_twImag.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
