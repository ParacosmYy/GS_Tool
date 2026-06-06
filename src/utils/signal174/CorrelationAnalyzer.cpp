/**
 * @file CorrelationAnalyzer.cpp
 * @brief CorrelationAnalyzer 实现
 *
 * 实现相关性分析：直接/FFT加速互相关与自相关、归一化Pearson系数。
 */

#include "utils/signal174/CorrelationAnalyzer.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

CorrelationAnalyzer::CorrelationAnalyzer(QObject *parent)
    : QObject(parent)
{
}

CorrelationAnalyzer::~CorrelationAnalyzer() = default;

/* ---- Configuration ---- */

void CorrelationAnalyzer::setUseFFT(bool enable) { m_useFFT = enable; }
void CorrelationAnalyzer::setMaxLag(int maxLag) { m_maxLag = maxLag; }

/* ---- Next power of 2 ---- */

int CorrelationAnalyzer::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- In-place radix-2 FFT ---- */

void CorrelationAnalyzer::fft2(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    if (n <= 1) return;

    /* Bit-reverse */
    int bits = 0;
    while ((1 << bits) < n) ++bits;
    for (int i = 0; i < n; ++i) {
        int rev = 0;
        for (int b = 0; b < bits; ++b)
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        if (i < rev) {
            qSwap(real[i], real[rev]);
            qSwap(imag[i], imag[rev]);
        }
    }

    /* Butterfly stages */
    for (int size = 2; size <= n; size *= 2) {
        int half = size / 2;
        double angle = -2.0 * M_PI / size;
        double cs = qCos(angle);
        double sn = qSin(angle);

        for (int i = 0; i < n; i += size) {
            double wr = 1.0, wi = 0.0;
            for (int j = 0; j < half; ++j) {
                double tr = wr * real[i + j + half] - wi * imag[i + j + half];
                double ti = wr * imag[i + j + half] + wi * real[i + j + half];
                real[i + j + half] = real[i + j] - tr;
                imag[i + j + half] = imag[i + j] - ti;
                real[i + j] += tr;
                imag[i + j] += ti;
                double newWr = wr * cs - wi * sn;
                wi = wr * sn + wi * cs;
                wr = newWr;
            }
        }
    }
}

/* ---- In-place radix-2 IFFT ---- */

void CorrelationAnalyzer::ifft2(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    for (int i = 0; i < n; ++i) imag[i] = -imag[i];
    fft2(real, imag);
    for (int i = 0; i < n; ++i) {
        real[i] /= n;
        imag[i] = -imag[i] / n;
    }
}

/* ---- Pearson coefficient ---- */

double CorrelationAnalyzer::pearsonCoeff(const QVector<double>& a,
                                          const QVector<double>& b)
{
    int n = qMin(a.size(), b.size());
    if (n < 2) return 0.0;

    double ma = 0.0, mb = 0.0;
    for (int i = 0; i < n; ++i) { ma += a[i]; mb += b[i]; }
    ma /= n; mb /= n;

    double num = 0.0, da = 0.0, db = 0.0;
    for (int i = 0; i < n; ++i) {
        double va = a[i] - ma, vb = b[i] - mb;
        num += va * vb;
        da += va * va;
        db += vb * vb;
    }
    double denom = qSqrt(da * db);
    return (denom < 1e-15) ? 0.0 : num / denom;
}

/* ---- Find peak in correlation ---- */

QPair<double, double> CorrelationAnalyzer::findPeak(const QVector<double>& lags,
                                                      const QVector<double>& corr)
{
    if (corr.isEmpty()) return {0.0, 0.0};

    double peakVal = corr[0];
    int peakIdx = 0;
    for (int i = 1; i < corr.size(); ++i) {
        if (qFabs(corr[i]) > qFabs(peakVal)) {
            peakVal = corr[i];
            peakIdx = i;
        }
    }
    return {lags[peakIdx], peakVal};
}

/* ---- Direct correlation ---- */

CorrelationAnalyzer::CorrResult CorrelationAnalyzer::directCorrelation(
    const QVector<double>& a, const QVector<double>& b, bool isAuto) const
{
    int n = a.size();
    int maxLag = (m_maxLag > 0) ? qMin(m_maxLag, n - 1) : n - 1;

    CorrResult result;
    result.lags.resize(2 * maxLag + 1);
    result.correlation.resize(2 * maxLag + 1);

    /* Normalize factor */
    double normA = 0.0;
    for (int i = 0; i < n; ++i) normA += a[i] * a[i];

    for (int lag = -maxLag; lag <= maxLag; ++lag) {
        int idx = lag + maxLag;
        result.lags[idx] = lag;

        double sum = 0.0;
        int count = 0;
        for (int i = 0; i < n; ++i) {
            int j = i + lag;
            if (j >= 0 && j < n) {
                sum += a[i] * b[j];
                count++;
            }
        }
        result.correlation[idx] = (count > 0) ? sum / count : 0.0;
    }

    /* Normalize */
    double normB = 0.0;
    if (!isAuto)
        for (int i = 0; i < b.size(); ++i) normB += b[i] * b[i];
    else
        normB = normA;

    double norm = qSqrt(normA * normB);
    if (norm > 1e-15) {
        for (auto& v : result.correlation)
            v /= norm;
    }

    auto peak = findPeak(result.lags, result.correlation);
    result.peakLag = peak.first;
    result.peakValue = peak.second;
    result.normalizedCoeff = result.correlation[maxLag]; /* zero-lag coefficient */

    return result;
}

/* ---- FFT correlation ---- */

CorrelationAnalyzer::CorrResult CorrelationAnalyzer::fftCorrelation(
    const QVector<double>& a, const QVector<double>& b, bool isAuto)
{
    int na = a.size();
    int nb = b.size();
    int n = nextPow2(na + nb - 1);

    QVector<double> realA(n, 0.0), imagA(n, 0.0);
    QVector<double> realB(n, 0.0), imagB(n, 0.0);

    for (int i = 0; i < na; ++i) realA[i] = a[i];
    for (int i = 0; i < nb; ++i) realB[i] = b[i];

    /* FFT both signals */
    fft2(realA, imagA);
    fft2(realB, imagB);

    /* Cross-multiply: A * conj(B) */
    for (int i = 0; i < n; ++i) {
        double r = realA[i] * realB[i] + imagA[i] * imagB[i];
        double im = imagA[i] * realB[i] - realA[i] * imagB[i];
        realA[i] = r;
        imagA[i] = im;
    }

    /* IFFT */
    ifft2(realA, imagA);

    /* Build result */
    int maxLag = (m_maxLag > 0) ? qMin(m_maxLag, na - 1) : na - 1;
    CorrResult result;
    result.lags.resize(2 * maxLag + 1);
    result.correlation.resize(2 * maxLag + 1);

    for (int lag = -maxLag; lag <= maxLag; ++lag) {
        int idx = lag + maxLag;
        result.lags[idx] = lag;
        int srcIdx = (lag >= 0) ? lag : n + lag;
        result.correlation[idx] = realA[srcIdx] / na;
    }

    auto peak = findPeak(result.lags, result.correlation);
    result.peakLag = peak.first;
    result.peakValue = peak.second;
    result.normalizedCoeff = pearsonCoeff(a, b);

    return result;
}

/* ---- Auto-correlation ---- */

CorrelationAnalyzer::CorrResult CorrelationAnalyzer::autoCorrelation(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    CorrResult result = m_useFFT
        ? fftCorrelation(signal, signal, true)
        : directCorrelation(signal, signal, true);

    m_stats.totalComputations++;
    m_stats.lastLength = signal.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
        ? m_timeSum / m_stats.totalComputations : 0.0;

    emit computationCompleted(AutoCorrelation, result.peakLag, result.normalizedCoeff);
    return result;
}

/* ---- Cross-correlation ---- */

CorrelationAnalyzer::CorrResult CorrelationAnalyzer::crossCorrelation(
    const QVector<double>& signalA, const QVector<double>& signalB)
{
    QElapsedTimer timer;
    timer.start();

    CorrResult result = m_useFFT
        ? fftCorrelation(signalA, signalB, false)
        : directCorrelation(signalA, signalB, false);

    m_stats.totalComputations++;
    m_stats.lastLength = qMax(signalA.size(), signalB.size());
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
        ? m_timeSum / m_stats.totalComputations : 0.0;

    emit computationCompleted(CrossCorrelation, result.peakLag, result.normalizedCoeff);
    return result;
}

/* ---- Statistics ---- */

void CorrelationAnalyzer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
