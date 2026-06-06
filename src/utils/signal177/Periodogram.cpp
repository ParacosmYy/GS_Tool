/**
 * @file Periodogram.cpp
 * @brief Periodogram 实现
 *
 * 实现周期图谱估计：经典/Bartlett/Welch方法、窗函数、置信区间。
 */

#include "utils/signal177/Periodogram.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Periodogram::Periodogram(QObject *parent)
    : QObject(parent)
{
}

Periodogram::~Periodogram() = default;

/* ---- Configuration ---- */

void Periodogram::setMethod(Method method)
{
    m_method = method;
    emit methodChanged(method);
}

void Periodogram::setWindow(Window w) { m_window = w; }
void Periodogram::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }
void Periodogram::setFftSize(int size) { m_fftSize = qMax(16, size); }
void Periodogram::setOverlap(double overlap) { m_overlap = qBound(0.0, overlap, 0.95); }

/* ---- Apply window ---- */

QVector<double> Periodogram::applyWindow(const QVector<double>& data) const
{
    int n = data.size();
    QVector<double> windowed(n);
    for (int i = 0; i < n; ++i) {
        double w = 1.0;
        switch (m_window) {
        case Hann:
            w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
            break;
        case Hamming:
            w = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (n - 1));
            break;
        case Blackman:
            w = 0.42 - 0.5 * qCos(2.0 * M_PI * i / (n - 1))
                + 0.08 * qCos(4.0 * M_PI * i / (n - 1));
            break;
        default:
            break;
        }
        windowed[i] = data[i] * w;
    }
    return windowed;
}

/* ---- Simplified DFT ---- */

void Periodogram::computeDFT(const QVector<double>& input,
                              QVector<double>& realOut,
                              QVector<double>& imagOut) const
{
    int N = input.size();
    int halfN = N / 2 + 1;
    realOut.resize(halfN, 0.0);
    imagOut.resize(halfN, 0.0);

    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        double angle = -2.0 * M_PI * k / N;
        for (int n = 0; n < N; ++n) {
            double phi = angle * n;
            re += input[n] * qCos(phi);
            im += input[n] * qSin(phi);
        }
        realOut[k] = re;
        imagOut[k] = im;
    }
}

/* ---- Single periodogram ---- */

QVector<double> Periodogram::singlePeriodogram(const QVector<double>& segment) const
{
    auto windowed = applyWindow(segment);
    QVector<double> re, im;
    computeDFT(windowed, re, im);

    /* Compute window power for normalization */
    double winPower = 0.0;
    for (double v : windowed) winPower += v * v;
    if (winPower < 1e-20) winPower = 1.0;

    int halfN = re.size();
    QVector<double> psd(halfN);
    for (int k = 0; k < halfN; ++k) {
        double mag2 = re[k] * re[k] + im[k] * im[k];
        psd[k] = mag2 / (winPower * m_sampleRate);
    }
    return psd;
}

/* ---- Find peak ---- */

QPair<double, double> Periodogram::findPeak(const QVector<double>& psd,
                                             const QVector<double>& freqs)
{
    double maxPsd = 0.0;
    int maxIdx = 0;
    for (int i = 1; i < psd.size(); ++i) {
        if (psd[i] > maxPsd) {
            maxPsd = psd[i];
            maxIdx = i;
        }
    }
    return {freqs[maxIdx], maxPsd};
}

/* ---- Main estimate ---- */

Periodogram::SpectrumResult Periodogram::estimate(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    SpectrumResult result;
    int n = signal.size();
    if (n == 0) return result;

    int halfN = m_fftSize / 2 + 1;

    /* Build frequency axis */
    result.frequencies.resize(halfN);
    for (int k = 0; k < halfN; ++k)
        result.frequencies[k] = static_cast<double>(k) * m_sampleRate / m_fftSize;

    switch (m_method) {
    case Classical: {
        /* Single periodogram on full signal */
        QVector<double> padded = signal;
        padded.resize(m_fftSize, 0.0);
        result.psd = singlePeriodogram(padded);
        m_numAverages = 1;
        break;
    }
    case Bartlett: {
        /* Average non-overlapping segments */
        int segLen = m_fftSize;
        int numSegs = n / segLen;
        if (numSegs < 1) numSegs = 1;
        m_numAverages = numSegs;

        result.psd.resize(halfN, 0.0);
        for (int s = 0; s < numSegs; ++s) {
            QVector<double> seg(segLen, 0.0);
            for (int i = 0; i < segLen && s * segLen + i < n; ++i)
                seg[i] = signal[s * segLen + i];
            auto segPsd = singlePeriodogram(seg);
            for (int k = 0; k < halfN; ++k)
                result.psd[k] += segPsd[k];
        }
        for (int k = 0; k < halfN; ++k)
            result.psd[k] /= numSegs;
        break;
    }
    case Welch: {
        /* Overlapping windowed segments */
        int segLen = m_fftSize;
        int hop = static_cast<int>(segLen * (1.0 - m_overlap));
        if (hop < 1) hop = 1;
        int numSegs = (n - segLen) / hop + 1;
        if (numSegs < 1) { numSegs = 1; segLen = n; }
        m_numAverages = numSegs;

        result.psd.resize(halfN, 0.0);
        for (int s = 0; s < numSegs; ++s) {
            int start = s * hop;
            QVector<double> seg(segLen, 0.0);
            for (int i = 0; i < segLen && start + i < n; ++i)
                seg[i] = signal[start + i];
            auto segPsd = singlePeriodogram(seg);
            for (int k = 0; k < halfN; ++k)
                result.psd[k] += segPsd[k];
        }
        for (int k = 0; k < halfN; ++k)
            result.psd[k] /= numSegs;
        break;
    }
    }

    /* Compute dB and total power */
    result.psdDb.resize(result.psd.size());
    result.totalPower = 0.0;
    for (int k = 0; k < result.psd.size(); ++k) {
        result.psdDb[k] = (result.psd[k] > 1e-20)
            ? 10.0 * qLog10(result.psd[k]) : -200.0;
        result.totalPower += result.psd[k];
    }
    result.totalPower *= m_sampleRate / m_fftSize;

    /* Find peak */
    auto peak = findPeak(result.psd, result.frequencies);
    result.peakFrequency = peak.first;
    result.peakPower = peak.second;
    result.fftSize = m_fftSize;

    m_lastResult = result;

    m_stats.totalEstimates++;
    m_stats.lastFftSize = m_fftSize;
    m_stats.lastMethod = m_method;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    emit estimateCompleted(result.peakFrequency, result.peakPower);
    return result;
}

/* ---- Confidence interval ---- */

Periodogram::ConfidenceInterval Periodogram::confidenceInterval(
    double confidenceLevel) const
{
    ConfidenceInterval ci;
    ci.confidence = confidenceLevel;

    /* Chi-squared based confidence interval for PSD
       For K averages, PSD follows chi-squared with 2K DOF
       CI: [2K*PSD / chi2_upper, 2K*PSD / chi2_lower] */
    int K = qMax(1, m_numAverages);
    int dof = 2 * K;

    /* Approximate chi-squared quantiles using normal approximation */
    double alpha = 1.0 - confidenceLevel;
    double zLow = qSqrt(2.0 * dof) * (-1.0 + 0.0) + dof; /* simplified */
    double zHigh = qSqrt(2.0 * dof) * (1.0 + 0.0) + dof;

    /* In dB: 10*log10(dof / chi2) */
    if (dof > 0) {
        ci.lower = 10.0 * qLog10(static_cast<double>(dof) / zHigh);
        ci.upper = 10.0 * qLog10(static_cast<double>(dof) / qMax(1.0, zLow));
    }
    return ci;
}

/* ---- Accessors ---- */

Periodogram::SpectrumResult Periodogram::lastResult() const { return m_lastResult; }

void Periodogram::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
