/**
 * @file Periodogram5.cpp
 * @brief Periodogram5 实现
 *
 * 实现周期图：Bartlett平均周期图与Welch重叠分段法方差降低。
 */

#include "utils/signal235/Periodogram5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Periodogram5::Periodogram5(QObject *parent) : QObject(parent) {}
Periodogram5::~Periodogram5() = default;

/* ---- Configure ---- */

bool Periodogram5::configure(int segLen, int overlap, int fftSize, Method method)
{
    if (segLen < 16 || overlap < 0 || overlap >= segLen) return false;

    m_segLen = segLen;
    m_overlap = overlap;
    m_method = method;

    // Round FFT size to next power of 2
    int n = 1;
    while (n < fftSize) n <<= 1;
    m_fftSize = qMax(n, segLen);
    m_halfFft = m_fftSize / 2;

    precompute();

    m_stats.segmentLength = segLen;
    m_stats.overlap = overlap;
    m_stats.fftSize = m_fftSize;
    m_stats.method = static_cast<int>(method);
    return true;
}

/* ---- Precompute ---- */

void Periodogram5::precompute()
{
    // Hann window
    m_window.resize(m_segLen);
    for (int i = 0; i < m_segLen; ++i)
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_segLen));

    // Twiddle factors
    int n = m_fftSize;
    m_twReal.resize(n);
    m_twImag.resize(n);
    for (int k = 0; k < n; ++k) {
        double angle = -2.0 * M_PI * k / n;
        m_twReal[k] = qCos(angle);
        m_twImag[k] = qSin(angle);
    }
}

/* ---- Apply window ---- */

void Periodogram5::applyWindow(QVector<double>& seg) const
{
    int n = qMin(seg.size(), m_window.size());
    for (int i = 0; i < n; ++i)
        seg[i] *= m_window[i];
}

/* ---- In-place radix-2 FFT ---- */

void Periodogram5::fft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
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

/* ---- Estimate PSD ---- */

QVector<double> Periodogram5::estimate(const QVector<double>& signal, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    int totalLen = signal.size();
    m_stats.signalLength = totalLen;

    // Determine segment hop
    int hop = (m_method == Welch) ? (m_segLen - m_overlap) : m_segLen;

    // Compute number of segments
    int numSeg = 0;
    if (totalLen >= m_segLen) {
        numSeg = (totalLen - m_segLen) / hop + 1;
    }
    m_stats.numSegments = numSeg;

    // Window normalization factor
    double winNorm = 0.0;
    for (int i = 0; i < m_segLen; ++i) winNorm += m_window[i] * m_window[i];
    winNorm *= sampleRate;  // Normalize for PSD

    // Accumulate modified periodograms
    QVector<double> avgPsd(m_halfFft, 0.0);

    for (int seg = 0; seg < numSeg; ++seg) {
        int start = seg * hop;

        // Extract and window segment
        QVector<double> re(m_fftSize, 0.0), im(m_fftSize, 0.0);
        for (int i = 0; i < m_segLen; ++i)
            re[i] = signal[start + i] * m_window[i];

        // FFT
        fft(re, im);

        // Compute modified periodogram (one-sided)
        for (int k = 0; k < m_halfFft; ++k) {
            double power = (re[k] * re[k] + im[k] * im[k]) / winNorm;
            avgPsd[k] += power;
        }
    }

    // Average over segments
    if (numSeg > 0) {
        for (int k = 0; k < m_halfFft; ++k)
            avgPsd[k] /= numSeg;
    }

    // Double the power for one-sided PSD (except DC and Nyquist)
    for (int k = 1; k < m_halfFft - 1; ++k)
        avgPsd[k] *= 2.0;

    m_psd = avgPsd;
    m_stats.freqResolution = sampleRate / m_fftSize;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit estimateCompleted(numSeg, m_stats.freqResolution, timer.elapsed());
    return m_psd;
}

/* ---- Frequency axis ---- */

QVector<double> Periodogram5::frequencyAxis(double sampleRate) const
{
    QVector<double> freq(m_halfFft);
    for (int k = 0; k < m_halfFft; ++k)
        freq[k] = k * sampleRate / m_fftSize;
    return freq;
}

/* ---- Get PSD ---- */

QVector<double> Periodogram5::psd() const { return m_psd; }

/* ---- Reset ---- */

void Periodogram5::resetStatistics()
{
    m_psd.clear();
    m_window.clear();
    m_twReal.clear();
    m_twImag.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
