/**
 * @file SlidingDFT5.cpp
 * @brief SlidingDFT5 实现
 *
 * 实现滑动DFT：递推更新公式、频点稳定性监控、Goertzel稀疏谱回退、窗函数补偿。
 */

#include "utils/fft190/SlidingDFT5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SlidingDFT5::SlidingDFT5(QObject *parent) : QObject(parent)
{
    setBlockSize(m_blockSize);
}

SlidingDFT5::~SlidingDFT5() = default;

/* ---- Configuration ---- */

void SlidingDFT5::setBlockSize(int N)
{
    m_blockSize = qMax(8, N);
    int halfN = m_blockSize / 2 + 1;
    m_real.resize(halfN, 0.0);
    m_imag.resize(halfN, 0.0);
    m_inputBuf.resize(m_blockSize, 0.0);
    m_bufPos = 0;
    computeTwiddles();
}

void SlidingDFT5::setSampleRate(double sr) { m_sampleRate = qMax(1.0, sr); }
void SlidingDFT5::setStabilityThreshold(double t) { m_stabThreshold = qMax(1e-12, t); }

void SlidingDFT5::setTargetBins(const QVector<int>& bins)
{
    m_targetBins = bins;
    std::sort(m_targetBins.begin(), m_targetBins.end());
    m_targetBins.erase(std::unique(m_targetBins.begin(), m_targetBins.end()),
                       m_targetBins.end());
}

/* ---- Precompute twiddle factors ---- */

void SlidingDFT5::computeTwiddles()
{
    int halfN = m_blockSize / 2 + 1;
    m_cosW.resize(halfN);
    m_sinW.resize(halfN);
    for (int k = 0; k < halfN; ++k) {
        double angle = 2.0 * M_PI * k / m_blockSize;
        m_cosW[k] = qCos(angle);
        m_sinW[k] = qSin(angle);
    }
}

/* ---- Full recomputation to prevent numerical drift ---- */

void SlidingDFT5::recomputeFull()
{
    int halfN = m_blockSize / 2 + 1;
    m_real.fill(0.0);
    m_imag.fill(0.0);

    for (int n = 0; n < m_blockSize; ++n) {
        double x = m_inputBuf[(m_bufPos + n) % m_blockSize];
        for (int k = 0; k < halfN; ++k) {
            double angle = -2.0 * M_PI * k * n / m_blockSize;
            m_real[k] += x * qCos(angle);
            m_imag[k] += x * qSin(angle);
        }
    }
}

/* ---- Update with single sample ---- */

void SlidingDFT5::updateSample(double sample)
{
    QElapsedTimer timer;
    timer.start();

    // Oldest sample leaving the window
    double oldest = m_inputBuf[m_bufPos];

    // Store new sample
    m_inputBuf[m_bufPos] = sample;
    m_bufPos = (m_bufPos + 1) % m_blockSize;

    // Recursive update: F_new[k] = (F_old[k] - oldest + sample) * exp(j*2*pi*k/N)
    int halfN = m_blockSize / 2 + 1;
    for (int k = 0; k < halfN; ++k) {
        double re = m_real[k] - oldest + sample;
        double im = m_imag[k];
        // Multiply by twiddle: (re + j*im) * (cosW + j*sinW)
        m_real[k] = re * m_cosW[k] - im * m_sinW[k];
        m_imag[k] = re * m_sinW[k] + im * m_cosW[k];
    }

    m_stats.totalUpdates++;
    m_stats.numBins = halfN;

    // Periodically recompute to prevent drift (every blockSize updates)
    if (m_stats.totalUpdates % m_blockSize == 0)
        recomputeFull();

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalUpdates > 0)
        ? m_timeSum / m_stats.totalUpdates : 0.0;

    emit spectrumUpdated(halfN, timer.elapsed());
}

/* ---- Magnitude spectrum ---- */

QVector<double> SlidingDFT5::magnitudes() const
{
    int halfN = m_real.size();
    QVector<double> mag(halfN);
    for (int k = 0; k < halfN; ++k)
        mag[k] = qSqrt(m_real[k] * m_real[k] + m_imag[k] * m_imag[k]);
    return mag;
}

/* ---- Phase spectrum ---- */

QVector<double> SlidingDFT5::phases() const
{
    int halfN = m_real.size();
    QVector<double> phase(halfN);
    for (int k = 0; k < halfN; ++k)
        phase[k] = qAtan2(m_imag[k], m_real[k]);
    return phase;
}

/* ---- Bin frequency ---- */

double SlidingDFT5::binFrequency(int bin) const
{
    return static_cast<double>(bin) * m_sampleRate / m_blockSize;
}

/* ---- Check stability ---- */

QVector<bool> SlidingDFT5::checkStability() const
{
    int halfN = m_real.size();
    QVector<bool> stable(halfN, true);
    for (int k = 0; k < halfN; ++k) {
        double mag = qSqrt(m_real[k] * m_real[k] + m_imag[k] * m_imag[k]);
        if (mag > 1e10 || qIsNaN(mag)) stable[k] = false;
    }

    int unstable = 0;
    for (bool s : stable) if (!s) unstable++;
    m_stats.unstableBins = unstable;
    return stable;
}

/* ---- Goertzel fallback for sparse spectra ---- */

QVector<QPair<int, double>> SlidingDFT5::goertzelSpectrum(
    const QVector<double>& block, const QVector<int>& bins) const
{
    QVector<QPair<int, double>> result;
    int N = block.size();

    for (int k : bins) {
        if (k < 0 || k >= N) continue;
        double w = 2.0 * M_PI * k / N;
        double coeff = 2.0 * qCos(w);
        double s0 = 0.0, s1 = 0.0, s2 = 0.0;

        for (int n = 0; n < N; ++n) {
            s0 = block[n] + coeff * s1 - s2;
            s2 = s1;
            s1 = s0;
        }

        double mag = qSqrt(s1 * s1 + s2 * s2 - coeff * s1 * s2);
        result.append({k, mag});
    }
    return result;
}

/* ---- Reset ---- */

void SlidingDFT5::reset()
{
    m_real.fill(0.0);
    m_imag.fill(0.0);
    m_inputBuf.fill(0.0);
    m_bufPos = 0;
}

/* ---- Reset statistics ---- */

void SlidingDFT5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    reset();
}
