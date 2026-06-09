/**
 * @file Correlator6.cpp
 * @brief Correlator6 实现
 *
 * 实现互相关器：重叠保留频域法与归一化互相关模式匹配。
 */

#include "utils/signal239/Correlator6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Correlator6::Correlator6(QObject *parent) : QObject(parent) {}
Correlator6::~Correlator6() = default;

/* ---- Configuration ---- */

void Correlator6::setBlockSize(int size) { m_blockSize = qMax(64, size); }

/* ---- Next power of 2 ---- */

int Correlator6::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Radix-2 FFT ---- */

void Correlator6::fft(QVector<double>& re, QVector<double>& im, bool inverse)
{
    int n = re.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * M_PI / len * (inverse ? 1.0 : -1.0);
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uRe = re[i + j], uIm = im[i + j];
                double vRe = re[i + j + len / 2] * curRe - im[i + j + len / 2] * curIm;
                double vIm = re[i + j + len / 2] * curIm + im[i + j + len / 2] * curRe;
                re[i + j] = uRe + vRe; im[i + j] = uIm + vIm;
                re[i + j + len / 2] = uRe - vRe; im[i + j + len / 2] = uIm - vIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }
    if (inverse) for (int i = 0; i < n; ++i) { re[i] /= n; im[i] /= n; }
}

/* ---- Overlap-save block ---- */

QVector<double> Correlator6::overlapSaveBlock(const QVector<double>& sigBlock,
                                               const QVector<double>& patRe,
                                               const QVector<double>& patIm,
                                               int fftSize) const
{
    // Zero-pad signal block to fftSize
    QVector<double> sigRe(fftSize, 0.0), sigIm(fftSize, 0.0);
    for (int i = 0; i < sigBlock.size() && i < fftSize; ++i) sigRe[i] = sigBlock[i];

    fft(sigRe, sigIm, false);

    // Complex multiply: conj(pattern) * signal
    QVector<double> outRe(fftSize, 0.0), outIm(fftSize, 0.0);
    for (int i = 0; i < fftSize; ++i) {
        outRe[i] = patRe[i] * sigRe[i] + patIm[i] * sigIm[i];
        outIm[i] = patRe[i] * sigIm[i] - patIm[i] * sigRe[i];
    }

    fft(outRe, outIm, true);
    return outRe;
}

/* ---- Cross-correlate ---- */

QVector<double> Correlator6::correlate(const QVector<double>& signal,
                                        const QVector<double>& pattern)
{
    QElapsedTimer timer;
    timer.start();

    int sigLen = signal.size();
    int patLen = pattern.size();
    if (sigLen == 0 || patLen == 0) return {};

    int fftSize = nextPow2(qMax(m_blockSize, patLen * 2));

    // Precompute conjugated pattern FFT
    QVector<double> patRe(fftSize, 0.0), patIm(fftSize, 0.0);
    // Reverse pattern for correlation (not convolution)
    for (int i = 0; i < patLen; ++i) patRe[i] = pattern[patLen - 1 - i];
    fft(patRe, patIm, false);

    int resultLen = sigLen - patLen + 1;
    if (resultLen <= 0) return {};
    QVector<double> result(resultLen, 0.0);

    int overlap = patLen - 1;
    int validPerBlock = fftSize - overlap;
    int numBlocks = 0;
    int outPos = 0;

    for (int start = -overlap; start < sigLen && outPos < resultLen; start += validPerBlock) {
        QVector<double> block(fftSize, 0.0);
        for (int i = 0; i < fftSize; ++i) {
            int idx = start + i;
            if (idx >= 0 && idx < sigLen) block[i] = signal[idx];
        }

        QVector<double> xcorr = overlapSaveBlock(block, patRe, patIm, fftSize);

        // Extract valid samples (skip overlap samples)
        for (int i = overlap; i < fftSize && outPos < resultLen; ++i) {
            result[outPos++] = xcorr[i];
        }
        numBlocks++;
    }

    m_stats.signalLength = sigLen;
    m_stats.patternLength = patLen;
    m_stats.fftSize = fftSize;
    m_stats.numBlocks = numBlocks;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    // Find peak lag
    double peakVal = -std::numeric_limits<double>::max();
    int peakLag = 0;
    for (int i = 0; i < result.size(); ++i) {
        if (result[i] > peakVal) { peakVal = result[i]; peakLag = i; }
    }
    emit correlationCompleted(result.size(), peakLag, timer.elapsed());
    return result;
}

/* ---- Local statistics ---- */

void Correlator6::localStats(const QVector<double>& sig, int start, int len,
                              double& mean, double& std) const
{
    mean = 0.0;
    double sum2 = 0.0;
    for (int i = start; i < start + len && i < sig.size(); ++i) {
        mean += sig[i];
        sum2 += sig[i] * sig[i];
    }
    mean /= len;
    std = qSqrt(qMax(0.0, sum2 / len - mean * mean));
}

/* ---- NCC at lag ---- */

double Correlator6::nccAtLag(const QVector<double>& sig, const QVector<double>& pat, int lag) const
{
    int patLen = pat.size();
    double sigMean, sigStd, patMean, patStd;
    localStats(sig, lag, patLen, sigMean, sigStd);
    localStats(pat, 0, patLen, patMean, patStd);

    if (sigStd < 1e-15 || patStd < 1e-15) return 0.0;

    double ncc = 0.0;
    for (int i = 0; i < patLen; ++i)
        ncc += (sig[lag + i] - sigMean) * (pat[i] - patMean);
    return ncc / (patLen * sigStd * patStd);
}

/* ---- Parabolic interpolation ---- */

double Correlator6::interpolatePeak(const QVector<double>& corr, int idx) const
{
    if (idx <= 0 || idx >= corr.size() - 1) return static_cast<double>(idx);
    double y1 = corr[idx - 1], y2 = corr[idx], y3 = corr[idx + 1];
    double denom = 2.0 * (2.0 * y2 - y1 - y3);
    if (qAbs(denom) < 1e-30) return static_cast<double>(idx);
    return static_cast<double>(idx) + (y1 - y3) / denom;
}

/* ---- Normalized cross-correlation ---- */

QVector<double> Correlator6::normalizedCorrelate(const QVector<double>& signal,
                                                  const QVector<double>& pattern)
{
    QVector<double> raw = correlate(signal, pattern);
    int patLen = pattern.size();
    int resultLen = signal.size() - patLen + 1;
    if (resultLen <= 0) return {};

    QVector<double> ncc(resultLen);
    for (int lag = 0; lag < resultLen; ++lag)
        ncc[lag] = nccAtLag(signal, pattern, lag);
    return ncc;
}

/* ---- Find peak ---- */

Correlator6::PeakResult Correlator6::findPeak(const QVector<double>& signal,
                                               const QVector<double>& pattern)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> raw = correlate(signal, pattern);
    QVector<double> ncc = normalizedCorrelate(signal, pattern);

    PeakResult result;
    double bestVal = -std::numeric_limits<double>::max();
    for (int i = 0; i < raw.size(); ++i) {
        if (raw[i] > bestVal) {
            bestVal = raw[i];
            result.lag = i;
            result.value = raw[i];
        }
    }
    result.normalizedValue = (result.lag < ncc.size()) ? ncc[result.lag] : 0.0;
    result.confidence = qAbs(result.normalizedValue);

    emit peakFound(result.lag, result.value, result.confidence);
    return result;
}

/* ---- Find all peaks ---- */

QVector<Correlator6::PeakResult> Correlator6::findPeaks(const QVector<double>& signal,
                                                         const QVector<double>& pattern,
                                                         double threshold) const
{
    int patLen = pattern.size();
    int resultLen = signal.size() - patLen + 1;
    if (resultLen <= 0) return {};

    QVector<PeakResult> peaks;
    for (int lag = 0; lag < resultLen; ++lag) {
        double ncc = nccAtLag(signal, pattern, lag);
        if (ncc >= threshold) {
            PeakResult p;
            p.lag = lag;
            p.normalizedValue = ncc;
            p.confidence = qAbs(ncc);
            peaks.append(p);
        }
    }
    return peaks;
}

/* ---- Reset ---- */

void Correlator6::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
