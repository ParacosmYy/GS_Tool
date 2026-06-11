/**
 * @file Correlator10.cpp
 * @brief Correlator10 实现
 *
 * 实现相关器：重叠保留块处理与归一化互相关及统计显著性检验实现信号匹配。
 */

#include "utils/signal295/Correlator10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Correlator10::Correlator10(QObject *parent)
    : QObject(parent) {}

Correlator10::~Correlator10() = default;

/* ---- Configuration ---- */

void Correlator10::setBlockSize(int size) { m_blockSize = qBound(64, size, 65536); }
void Correlator10::setSignificanceLevel(double alpha) { m_alpha = qBound(0.001, alpha, 0.5); }

/* ---- Statistical helpers ---- */

double Correlator10::mean(const QVector<double>& v) const
{
    if (v.isEmpty()) return 0.0;
    double s = 0.0;
    for (double x : v) s += x;
    return s / v.size();
}

double Correlator10::stddev(const QVector<double>& v) const
{
    if (v.size() < 2) return 0.0;
    double m = mean(v);
    double s = 0.0;
    for (double x : v) s += (x - m) * (x - m);
    return qSqrt(s / (v.size() - 1));
}

/* ---- Log-gamma (Stirling + Lanczos) ---- */

double Correlator10::gammaLn(double x) const
{
    // Lanczos approximation
    static const double c[7] = {
        0.99999999999980993, 676.5203681218851, -1259.1392167224028,
        771.32342877765313, -176.61502916214059, 12.507343278686905,
        -0.13857109526572012
    };
    if (x < 0.5)
        return M_LNPI - qLn(qSin(M_PI * x)) - gammaLn(1.0 - x);

    x -= 1.0;
    double a = c[0];
    for (int i = 1; i < 7; ++i)
        a += c[i] / (x + i);
    double t = x + 5.5;
    return 0.5 * qLn(2.0 * M_PI) + (x + 0.5) * qLn(t) - t + qLn(a);
}

/* ---- Regularized incomplete beta ---- */

double Correlator10::betaIncomplete(double a, double b, double x) const
{
    if (x <= 0.0) return 0.0;
    if (x >= 1.0) return 1.0;

    double lnB = gammaLn(a) + gammaLn(b) - gammaLn(a + b);
    double front = qExp(qLn(x) * a + qLn(1.0 - x) * b - lnB);

    // Continued fraction (Lentz's method)
    double aa = 1.0, bb = 1.0;
    double f = 1.0;
    for (int i = 0; i < 200; ++i) {
        double d;
        if (i % 2 == 0) {
            double m = (i / 2);
            d = -(a + m) * (a + b + m) * x / ((a + 2.0 * m) * (a + 2.0 * m + 1.0));
        } else {
            double m = (i + 1) / 2;
            d = m * (b - m) * x / ((a + 2.0 * m - 1.0) * (a + 2.0 * m));
        }
        bb = 1.0 + d / bb;
        if (qAbs(bb) < 1e-30) bb = 1e-30;
        aa = 1.0 + d / aa;
        if (qAbs(aa) < 1e-30) aa = 1e-30;
        f *= aa / bb;
        if (qAbs(aa / bb - 1.0) < 1e-10) break;
    }
    return front * (f - 1.0);
}

/* ---- t-distribution p-value ---- */

double Correlator10::tDistPValue(double t, int df) const
{
    double x = df / (df + t * t);
    double p = betaIncomplete(df * 0.5, 0.5, x);
    return qBound(0.0, p, 1.0);
}

/* ---- p-value for correlation ---- */

double Correlator10::computePValue(double r, int n) const
{
    if (n < 3) return 1.0;
    double r2 = qBound(-1.0, r, 1.0);
    double t = r2 * qSqrt((n - 2) / (1.0 - r2 * r2 + 1e-30));
    return tDistPValue(t, n - 2);
}

/* ---- Overlap-save block convolution ---- */

QVector<double> Correlator10::overlapSaveConvolve(
    const QVector<double>& signal,
    const QVector<double>& kernel) const
{
    int kLen = kernel.size();
    int sLen = signal.size();
    int bSize = qMax(m_blockSize, kLen * 2);
    int validLen = bSize - kLen + 1;

    QVector<double> result(sLen + kLen - 1, 0.0);
    QVector<double> overlap(kLen - 1, 0.0);

    int outPos = 0;
    for (int start = -(kLen - 1); start < sLen; start += validLen) {
        // Build block with overlap from previous
        QVector<double> block(bSize, 0.0);
        for (int i = 0; i < kLen - 1; ++i)
            block[i] = overlap[i];

        int readStart = qMax(0, start);
        int readLen = qMin(validLen, sLen - readStart);
        for (int i = 0; i < readLen; ++i)
            block[kLen - 1 + i] = signal[readStart + i];

        // Direct convolution of block with kernel
        for (int i = kLen - 1; i < bSize; ++i) {
            double s = 0.0;
            for (int j = 0; j < kLen; ++j)
                s += block[i - j] * kernel[j];
            int resIdx = outPos + (i - kLen + 1);
            if (resIdx >= 0 && resIdx < result.size())
                result[resIdx] = s;
        }

        // Save overlap for next block
        for (int i = 0; i < kLen - 1; ++i) {
            int srcIdx = bSize - kLen + 1 + i;
            overlap[i] = (srcIdx < bSize) ? block[srcIdx] : 0.0;
        }
        outPos += validLen;
    }

    return result;
}

/* ---- Main correlate ---- */

Correlator10::CorrelResult Correlator10::correlate(
    const QVector<double>& signal,
    const QVector<double>& pattern) const
{
    QElapsedTimer timer;
    timer.start();

    CorrelResult result;
    int sLen = signal.size();
    int pLen = pattern.size();
    if (sLen == 0 || pLen == 0) return result;

    // Normalize both signals
    double sigMean = mean(signal);
    double sigStd = stddev(signal);
    double patMean = mean(pattern);
    double patStd = stddev(pattern);

    QVector<double> normSig(sLen), normPat(pLen);
    for (int i = 0; i < sLen; ++i)
        normSig[i] = (sigStd > 1e-15) ? (signal[i] - sigMean) / sigStd : 0.0;
    for (int i = 0; i < pLen; ++i)
        normPat[i] = (patStd > 1e-15) ? (pattern[i] - patMean) / patStd : 0.0;

    // Reverse pattern for cross-correlation via convolution
    QVector<double> revPat(pLen);
    for (int i = 0; i < pLen; ++i)
        revPat[i] = normPat[pLen - 1 - i];

    // Convolve via overlap-save
    auto rawCorr = overlapSaveConvolve(normSig, revPat);

    // Normalize by pattern length
    int corrLen = sLen + pLen - 1;
    result.correlation.resize(corrLen);
    for (int i = 0; i < qMin(corrLen, rawCorr.size()); ++i)
        result.correlation[i] = rawCorr[i] / pLen;

    // Find peak
    result.peakValue = -1e300;
    for (int i = 0; i < corrLen; ++i) {
        if (result.correlation[i] > result.peakValue) {
            result.peakValue = result.correlation[i];
            result.peakLag = i - pLen + 1;
        }
    }
    result.peakValue = qBound(-1.0, result.peakValue, 1.0);

    // Statistical significance
    int n = qMin(sLen, pLen);
    result.zScore = result.peakValue * qSqrt(n - 2) / qSqrt(1.0 - result.peakValue * result.peakValue + 1e-30);
    result.significance = computePValue(result.peakValue, n);

    double elapsed = timer.elapsed();
    m_stats.blockSize = m_blockSize;
    m_stats.totalCorrelations++;
    m_peakSum += result.peakValue;
    m_stats.avgPeakCorrelation = m_peakSum / m_stats.totalCorrelations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCorrelations;

    emit correlationDone(sLen, pLen, result.peakValue, elapsed);
    return result;
}

/* ---- Correlate at specific lag range ---- */

QVector<double> Correlator10::correlateLagRange(const QVector<double>& signal,
                                                   const QVector<double>& pattern,
                                                   int minLag, int maxLag) const
{
    double sigMean = mean(signal);
    double sigStd = stddev(signal);
    double patMean = mean(pattern);
    double patStd = stddev(pattern);

    QVector<double> result;
    result.reserve(maxLag - minLag + 1);

    int pLen = pattern.size();
    for (int lag = minLag; lag <= maxLag; ++lag) {
        double sum = 0.0;
        int count = 0;
        for (int i = 0; i < pLen; ++i) {
            int si = i + lag;
            if (si >= 0 && si < signal.size()) {
                double sv = (sigStd > 1e-15) ? (signal[si] - sigMean) / sigStd : 0.0;
                double pv = (patStd > 1e-15) ? (pattern[i] - patMean) / patStd : 0.0;
                sum += sv * pv;
                count++;
            }
        }
        result.append((count > 0) ? sum / count : 0.0);
    }
    return result;
}

/* ---- Reset ---- */

void Correlator10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_peakSum = 0.0;
}
