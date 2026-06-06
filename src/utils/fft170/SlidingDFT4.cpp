/**
 * @file SlidingDFT4.cpp
 * @brief SlidingDFT4 实现
 *
 * 实现滑动DFT：递推更新Bin、环形缓冲区、实时频谱提取。
 */

#include "utils/fft170/SlidingDFT4.h"

#include <QElapsedTimer>
#include <QtMath>

SlidingDFT4::SlidingDFT4(int N, QObject *parent)
    : QObject(parent), m_N(qMax(4, N))
{
    initBins();
}

SlidingDFT4::~SlidingDFT4() = default;

void SlidingDFT4::initBins()
{
    m_bins.resize(m_N);
    m_buffer.resize(m_N);
    std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
    m_bufIdx = 0;
    m_sampleCount = 0;

    /* Precompute twiddle factors: coeff_k = e^{-j*2pi*k/N} */
    for (int k = 0; k < m_N; ++k) {
        double angle = -2.0 * M_PI * k / m_N;
        m_bins[k].coeffReal = qCos(angle);
        m_bins[k].coeffImag = qSin(angle);
        m_bins[k].real = 0.0;
        m_bins[k].imag = 0.0;
    }
}

void SlidingDFT4::setTransformSize(int N)
{
    m_N = qMax(4, N);
    initBins();
    m_stats.transformSize = m_N;
}

void SlidingDFT4::pushSample(double sample)
{
    QElapsedTimer timer;
    timer.start();

    /* Oldest sample in circular buffer */
    double oldest = m_buffer[m_bufIdx];

    /* Insert new sample */
    m_buffer[m_bufIdx] = sample;
    m_bufIdx = (m_bufIdx + 1) % m_N;
    m_sampleCount++;

    /* Recursive update: X_k[n] = coeff_k * (X_k[n-1] + x[n] - x[n-N]) */
    double diff = sample - oldest;
    for (int k = 0; k < m_N; ++k) {
        double newReal = diff + m_bins[k].real;
        double newImag = m_bins[k].imag;

        /* Complex multiply by coeff */
        double cr = m_bins[k].coeffReal;
        double ci = m_bins[k].coeffImag;
        m_bins[k].real = newReal * cr - newImag * ci;
        m_bins[k].imag = newReal * ci + newImag * cr;
    }

    m_stats.totalSamples++;
    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalUpdates > 0)
        ? m_timeSum / m_stats.totalUpdates : 0.0;

    emit sampleProcessed(m_sampleCount);
}

void SlidingDFT4::processBatch(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    for (double s : samples)
    {
        double oldest = m_buffer[m_bufIdx];
        m_buffer[m_bufIdx] = s;
        m_bufIdx = (m_bufIdx + 1) % m_N;
        m_sampleCount++;

        double diff = s - oldest;
        for (int k = 0; k < m_N; ++k) {
            double nr = diff + m_bins[k].real;
            double ni = m_bins[k].imag;
            double cr = m_bins[k].coeffReal;
            double ci = m_bins[k].coeffImag;
            m_bins[k].real = nr * cr - ni * ci;
            m_bins[k].imag = nr * ci + ni * cr;
        }
    }

    m_stats.totalSamples += samples.size();
    m_stats.totalUpdates += samples.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalUpdates > 0)
        ? m_timeSum / m_stats.totalUpdates : 0.0;

    emit batchCompleted(samples.size());
}

QVector<double> SlidingDFT4::magnitudes() const
{
    QVector<double> mag(m_N);
    for (int k = 0; k < m_N; ++k)
        mag[k] = qSqrt(m_bins[k].real * m_bins[k].real +
                        m_bins[k].imag * m_bins[k].imag);
    return mag;
}

QVector<double> SlidingDFT4::phases() const
{
    QVector<double> ph(m_N);
    for (int k = 0; k < m_N; ++k)
        ph[k] = qAtan2(m_bins[k].imag, m_bins[k].real);
    return ph;
}

double SlidingDFT4::binMagnitude(int k) const
{
    if (k < 0 || k >= m_N) return 0.0;
    return qSqrt(m_bins[k].real * m_bins[k].real + m_bins[k].imag * m_bins[k].imag);
}

double SlidingDFT4::binPhase(int k) const
{
    if (k < 0 || k >= m_N) return 0.0;
    return qAtan2(m_bins[k].imag, m_bins[k].real);
}

void SlidingDFT4::reset()
{
    for (int k = 0; k < m_N; ++k) {
        m_bins[k].real = 0.0;
        m_bins[k].imag = 0.0;
    }
    std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
    m_bufIdx = 0;
    m_sampleCount = 0;
}

void SlidingDFT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
