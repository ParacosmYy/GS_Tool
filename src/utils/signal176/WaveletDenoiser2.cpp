/**
 * @file WaveletDenoiser2.cpp
 * @brief WaveletDenoiser2 实现
 *
 * 实现小波去噪：多级分解/重构、软/硬阈值、多种小波基。
 */

#include "utils/signal176/WaveletDenoiser2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WaveletDenoiser2::WaveletDenoiser2(QObject *parent)
    : QObject(parent)
{
}

WaveletDenoiser2::~WaveletDenoiser2() = default;

/* ---- Configuration ---- */

void WaveletDenoiser2::setWaveletType(WaveletType type) { m_wavelet = type; }
void WaveletDenoiser2::setDecompositionLevels(int levels) { m_levels = qMax(1, levels); }
void WaveletDenoiser2::setThresholdType(ThresholdType type) { m_threshType = type; }
void WaveletDenoiser2::setThreshold(double threshold) { m_threshold = qMax(0.0, threshold); }

/* ---- Get analysis filters ---- */

void WaveletDenoiser2::getFilters(QVector<double>& lowDec,
                                   QVector<double>& highDec) const
{
    switch (m_wavelet) {
    case Haar:
        lowDec = {0.7071067811865476, 0.7071067811865476};
        highDec = {0.7071067811865476, -0.7071067811865476};
        break;
    case Db2:
        lowDec = {-0.1294095225512603, 0.2241438680420134,
                   0.8365163037378077, 0.4829629131445341};
        highDec = {-0.4829629131445341, 0.8365163037378077,
                   -0.2241438680420134, -0.1294095225512603};
        break;
    case Db4:
        lowDec = {-0.010597401784997278, 0.03288301166698296,
                   0.030841381835986965, -0.18703481171888145,
                   -0.02798376941698385, 0.6308807679295904,
                   0.7148465705525415, 0.23037781330889648};
        highDec = {-0.23037781330889648, 0.7148465705525415,
                   -0.6308807679295904, -0.02798376941698385,
                    0.18703481171888145, 0.030841381835986965,
                   -0.03288301166698296, -0.010597401784997278};
        break;
    case Sym4:
        lowDec = {0.022405651182, -0.047648438950, -0.042839423146,
                   0.380921059360, 0.764407862070, 0.430938988530,
                  -0.087977812890, -0.040095754210};
        highDec = {0.040095754210, 0.087977812890, -0.430938988530,
                   0.764407862070, -0.380921059360, -0.042839423146,
                   0.047648438950, -0.022405651182};
        break;
    }
}

/* ---- Get synthesis filters ---- */

void WaveletDenoiser2::getReconFilters(QVector<double>& lowRec,
                                        QVector<double>& highRec) const
{
    QVector<double> ld, hd;
    getFilters(ld, hd);
    int n = ld.size();
    lowRec.resize(n);
    highRec.resize(n);
    /* Reconstruction filters = time-reversed analysis filters */
    for (int i = 0; i < n; ++i) {
        lowRec[i] = hd[n - 1 - i];
        highRec[i] = ld[n - 1 - i];
    }
}

/* ---- Single-level DWT ---- */

void WaveletDenoiser2::dwtStep(const QVector<double>& input,
                                QVector<double>& approx,
                                QVector<double>& detail) const
{
    QVector<double> ld, hd;
    getFilters(ld, hd);
    int n = input.size();
    int fLen = ld.size();
    int outLen = (n + fLen - 1) / 2; /* Approximate half-length */

    approx.resize(outLen);
    detail.resize(outLen);

    for (int k = 0; k < outLen; ++k) {
        double aSum = 0.0, dSum = 0.0;
        for (int m = 0; m < fLen; ++m) {
            int idx = 2 * k + m;
            /* Periodic extension */
            idx = idx % n;
            aSum += input[idx] * ld[m];
            dSum += input[idx] * hd[m];
        }
        approx[k] = aSum;
        detail[k] = dSum;
    }
}

/* ---- Single-level inverse DWT ---- */

QVector<double> WaveletDenoiser2::idwtStep(const QVector<double>& approx,
                                             const QVector<double>& detail) const
{
    QVector<double> lr, hr;
    getReconFilters(lr, hr);
    int fLen = lr.size();
    int n = approx.size();
    int outLen = 2 * n;

    QVector<double> output(outLen, 0.0);

    for (int k = 0; k < n; ++k) {
        for (int m = 0; m < fLen; ++m) {
            int idx = 2 * k + m;
            if (idx < outLen) {
                output[idx] += approx[k] * lr[m];
                output[idx] += detail[k] * hr[m];
            }
        }
    }
    return output;
}

/* ---- Apply threshold ---- */

double WaveletDenoiser2::applyThreshold(double value, double thresh) const
{
    if (m_threshType == Soft) {
        if (value > thresh) return value - thresh;
        if (value < -thresh) return value + thresh;
        return 0.0;
    }
    /* Hard thresholding */
    return (qAbs(value) > thresh) ? value : 0.0;
}

/* ---- Multi-level decomposition ---- */

void WaveletDenoiser2::decompose(const QVector<double>& signal,
                                  QVector<QVector<double>>& detailCoeffs,
                                  QVector<double>& approxCoeffs) const
{
    detailCoeffs.clear();
    QVector<double> current = signal;

    for (int level = 0; level < m_levels; ++level) {
        QVector<double> a, d;
        dwtStep(current, a, d);
        detailCoeffs.prepend(d); /* Store highest frequency first */
        current = a;
        if (current.size() < 4) break; /* Too short to continue */
    }
    approxCoeffs = current;
}

/* ---- Multi-level reconstruction ---- */

QVector<double> WaveletDenoiser2::reconstruct(
    const QVector<QVector<double>>& detailCoeffs,
    const QVector<double>& approxCoeffs) const
{
    QVector<double> current = approxCoeffs;

    for (int level = detailCoeffs.size() - 1; level >= 0; --level) {
        /* Pad if sizes mismatch */
        int n = current.size();
        if (detailCoeffs[level].size() != n) {
            int maxN = qMax(n, detailCoeffs[level].size());
            current.resize(maxN, 0.0);
            QVector<double> paddedDetail = detailCoeffs[level];
            paddedDetail.resize(maxN, 0.0);
            current = idwtStep(current, paddedDetail);
        } else {
            current = idwtStep(current, detailCoeffs[level]);
        }
    }
    return current;
}

/* ---- Noise estimation (MAD) ---- */

double WaveletDenoiser2::estimateNoiseStd(const QVector<double>& detailCoeffs) const
{
    if (detailCoeffs.isEmpty()) return 0.0;
    /* Median Absolute Deviation */
    QVector<double> sorted = detailCoeffs;
    for (auto& v : sorted) v = qAbs(v);
    std::sort(sorted.begin(), sorted.end());
    int n = sorted.size();
    double median = (n % 2 == 0) ?
        (sorted[n / 2 - 1] + sorted[n / 2]) * 0.5 : sorted[n / 2];
    return median / 0.6745; /* Scale for Gaussian */
}

/* ---- Universal threshold (VisuShrink) ---- */

double WaveletDenoiser2::universalThreshold(int n, double sigma) const
{
    return sigma * qSqrt(2.0 * qLn(static_cast<double>(n)));
}

/* ---- Main denoise ---- */

QVector<double> WaveletDenoiser2::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 4) return signal;

    /* Decompose */
    QVector<QVector<double>> detailCoeffs;
    QVector<double> approxCoeffs;
    decompose(signal, detailCoeffs, approxCoeffs);

    /* Estimate noise from finest detail coefficients */
    double sigma = estimateNoiseStd(detailCoeffs[0]);
    double thresh = m_threshold;
    if (thresh <= 0.0)
        thresh = universalThreshold(n, sigma);

    /* Apply threshold to detail coefficients */
    for (auto& level : detailCoeffs) {
        for (auto& coeff : level)
            coeff = applyThreshold(coeff, thresh);
    }

    /* Reconstruct */
    QVector<double> result = reconstruct(detailCoeffs, approxCoeffs);

    /* Trim to original length */
    if (result.size() > n)
        result = result.mid(0, n);

    /* Estimate SNR improvement */
    double signalPower = 0.0, noisePower = 0.0;
    for (int i = 0; i < qMin(n, result.size()); ++i) {
        signalPower += result[i] * result[i];
        double diff = signal[i] - result[i];
        noisePower += diff * diff;
    }
    double snr = (noisePower > 0) ? 10.0 * qLog10(signalPower / noisePower) : 100.0;

    m_stats.totalDenoise++;
    m_stats.lastLevels = qMin(m_levels, detailCoeffs.size());
    m_stats.lastThreshold = thresh;
    m_stats.outputSnr = snr;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDenoise;

    emit denoiseCompleted(m_stats.lastLevels, thresh, snr);
    return result;
}

/* ---- Get wavelet coefficients ---- */

QVector<double> WaveletDenoiser2::waveletCoeffs() const
{
    QVector<double> ld, hd;
    getFilters(ld, hd);
    return ld;
}

/* ---- Statistics ---- */

void WaveletDenoiser2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
