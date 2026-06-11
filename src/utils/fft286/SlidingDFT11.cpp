/**
 * @file SlidingDFT11.cpp
 * @brief SlidingDFT11 实现
 *
 * 实现滑动DFT：加窗递归更新与频谱泄漏补偿的连续频率监测。
 */

#include "utils/fft286/SlidingDFT11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SlidingDFT11::SlidingDFT11(QObject *parent)
    : QObject(parent)
{
    setConfig(DFTConfig{});
}

SlidingDFT11::~SlidingDFT11() = default;

/* ---- Configuration ---- */

void SlidingDFT11::setConfig(const DFTConfig& cfg)
{
    m_config = cfg;
    m_config.fftSize = qBound(16, cfg.fftSize, 8192);
    m_config.hopSize = qBound(1, cfg.hopSize, m_config.fftSize);

    int N = m_config.fftSize;
    m_circularBuf.resize(N, 0.0);
    m_re.resize(N, 0.0);
    m_im.resize(N, 0.0);
    m_prevRe.resize(N, 0.0);
    m_prevIm.resize(N, 0.0);

    designWindow();
}

/* ---- Design Hann window ---- */

void SlidingDFT11::designWindow()
{
    int N = m_config.fftSize;
    m_window.resize(N);
    double sum = 0.0;
    for (int i = 0; i < N; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / N));
        m_window[i] = w;
        sum += w;
    }
    m_config.windowCorrection = N / sum;
}

/* ---- Initialize specific bins ---- */

void SlidingDFT11::initBins(const QVector<int>& bins)
{
    m_activeBins = bins;
    // Ensure bins in range
    for (auto& b : m_activeBins)
        b = qBound(0, b, m_config.fftSize / 2);
}

/* ---- Twiddle factor ---- */

void SlidingDFT11::twiddle(int k, double& wr, double& wi) const
{
    double angle = -2.0 * M_PI * k / m_config.fftSize;
    wr = qCos(angle);
    wi = qSin(angle);
}

/* ---- Recursive DFT update ---- */

void SlidingDFT11::recursiveUpdate(double xNew, double xOld)
{
    int N = m_config.fftSize;
    int numBins = m_activeBins.isEmpty() ? N : m_activeBins.size();
    const QVector<int>& bins = m_activeBins;

    for (int i = 0; i < numBins; ++i) {
        int k = m_activeBins.isEmpty() ? i : bins[i];
        double wr, wi;
        twiddle(k, wr, wi);

        // Recursive update: X_k[n] = (X_k[n-1] - x[n-N] + x[n]) * W_N^k
        double diff = xNew - xOld;
        double oldRe = m_re[k];
        double oldIm = m_im[k];

        m_re[k] = oldRe + diff;
        m_im[k] = oldIm;

        // Apply twiddle rotation
        double rotRe = m_re[k] * wr - m_im[k] * wi;
        double rotIm = m_re[k] * wi + m_im[k] * wr;
        m_re[k] = rotRe;
        m_im[k] = rotIm;
    }
}

/* ---- Apply spectral leakage compensation ---- */

void SlidingDFT11::applyLeakageCompensation()
{
    int N = m_config.fftSize;
    int numBins = m_activeBins.isEmpty() ? N : m_activeBins.size();
    const QVector<int>& bins = m_activeBins;

    for (int i = 0; i < numBins; ++i) {
        int k = m_activeBins.isEmpty() ? i : bins[i];
        // Compensation: interpolate from neighbors to reduce leakage
        double prevMag = qSqrt(m_prevRe[k] * m_prevRe[k] + m_prevIm[k] * m_prevIm[k]);
        double curMag = qSqrt(m_re[k] * m_re[k] + m_im[k] * m_im[k]);

        if (curMag > 0.0 && prevMag > 0.0) {
            // Apply correction based on phase continuity
            double curPhase = qAtan2(m_im[k], m_re[k]);
            double prevPhase = qAtan2(m_prevIm[k], m_prevRe[k]);
            double phaseDiff = curPhase - prevPhase;
            if (phaseDiff > M_PI) phaseDiff -= 2.0 * M_PI;
            if (phaseDiff < -M_PI) phaseDiff += 2.0 * M_PI;

            // Dampen if phase discontinuity detected (leakage indicator)
            double leakageFactor = qExp(-qAbs(phaseDiff) * 0.5);
            m_re[k] *= (0.7 + 0.3 * leakageFactor);
            m_im[k] *= (0.7 + 0.3 * leakageFactor);
        }
    }
}

/* ---- Push single sample ---- */

void SlidingDFT11::pushSample(double sample)
{
    int N = m_config.fftSize;
    int oldIdx = m_writeIdx;

    // Save previous state for leakage compensation
    m_prevRe = m_re;
    m_prevIm = m_im;

    double oldSample = m_circularBuf[oldIdx];
    double windowedNew = m_config.applyWindow ? sample * m_window[oldIdx] : sample;
    double windowedOld = m_config.applyWindow ? oldSample * m_window[oldIdx] : oldSample;

    m_circularBuf[oldIdx] = sample;
    m_writeIdx = (m_writeIdx + 1) % N;
    m_sampleCount++;

    recursiveUpdate(windowedNew, windowedOld);
    applyLeakageCompensation();
}

/* ---- Process block ---- */

QVector<SlidingDFT11::SpectrumFrame> SlidingDFT11::processBlock(const QVector<double>& block)
{
    QElapsedTimer timer;
    timer.start();

    QVector<SpectrumFrame> frames;
    int hop = m_config.hopSize;

    for (int i = 0; i < block.size(); i += hop) {
        for (int h = 0; h < hop && (i + h) < block.size(); ++h)
            pushSample(block[i + h]);

        if ((m_sampleCount % hop) == 0) {
            SpectrumFrame frame = currentSpectrum();
            frame.timestamp = static_cast<double>(m_sampleCount);
            frames.append(frame);

            double elapsed = timer.elapsed();
            m_stats.totalOps++;
            m_timeSum += elapsed;
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
            emit frameReady(frames.size() - 1, elapsed);
        }
    }
    return frames;
}

/* ---- Current spectrum ---- */

SlidingDFT11::SpectrumFrame SlidingDFT11::currentSpectrum() const
{
    SpectrumFrame frame;
    int numBins = m_activeBins.isEmpty() ? m_config.fftSize / 2 + 1 : m_activeBins.size();
    frame.magnitude.resize(numBins);
    frame.phase.resize(numBins);

    for (int i = 0; i < numBins; ++i) {
        int k = m_activeBins.isEmpty() ? i : m_activeBins[i];
        double re = m_re[k] * m_config.windowCorrection;
        double im = m_im[k] * m_config.windowCorrection;
        frame.magnitude[i] = qSqrt(re * re + im * im) / m_config.fftSize;
        frame.phase[i] = qAtan2(im, re);
    }
    frame.timestamp = static_cast<double>(m_sampleCount);
    return frame;
}

/* ---- Reset window ---- */

void SlidingDFT11::resetWindow()
{
    m_circularBuf.fill(0.0);
    m_re.fill(0.0);
    m_im.fill(0.0);
    m_prevRe.fill(0.0);
    m_prevIm.fill(0.0);
    m_writeIdx = 0;
    m_sampleCount = 0;
}

/* ---- Reset statistics ---- */

void SlidingDFT11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    resetWindow();
}
