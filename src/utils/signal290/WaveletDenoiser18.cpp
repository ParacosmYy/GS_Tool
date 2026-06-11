/**
 * @file WaveletDenoiser18.cpp
 * @brief WaveletDenoiser18 实现
 *
 * 实现小波去噪：贝叶斯收缩与后验中值估计的层自适应阈值。
 */

#include "utils/signal290/WaveletDenoiser18.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WaveletDenoiser18::WaveletDenoiser18(QObject *parent)
    : QObject(parent) {}

WaveletDenoiser18::~WaveletDenoiser18() = default;

/* ---- Configuration ---- */

void WaveletDenoiser18::setParams(const Params& p)
{
    m_params = p;
    m_params.levels = qBound(1, m_params.levels, 20);
}

/* ---- Wavelet filters ---- */

QVector<double> WaveletDenoiser18::lowPassFilter(Wavelet w) const
{
    switch (w) {
    case Wavelet::Haar:
        return {1.0 / qSqrt(2.0), 1.0 / qSqrt(2.0)};
    case Wavelet::Daubechies4:
        return {0.4829629131, 0.8365163037, 0.2241438680, -0.1294095226};
    case Wavelet::Daubechies8: {
        double s = qSqrt(2.0);
        return {0.2303778133/s, 0.7148465706/s, 0.6308807679/s, -0.0279837694/s,
                -0.1870348117/s, 0.0308413818/s, 0.0328830117/s, -0.0105974018/s};
    }
    case Wavelet::Symlet6: {
        double s = qSqrt(2.0);
        return {0.015404109/s, 0.018315582/s, -0.073442501/s, -0.261819700/s,
                0.562286030/s, 0.795670400/s, 0.212092750/s, -0.136524500/s,
                -0.016394100/s, 0.028417800/s, 0.001567200/s, -0.001444600/s};
    }
    }
    return {1.0 / qSqrt(2.0), 1.0 / qSqrt(2.0)};
}

QVector<double> WaveletDenoiser18::highPassFilter(Wavelet w) const
{
    QVector<double> lo = lowPassFilter(w);
    int n = lo.size();
    QVector<double> hi(n);
    for (int i = 0; i < n; ++i)
        hi[i] = (i % 2 == 0) ? lo[n - 1 - i] : -lo[n - 1 - i];
    return hi;
}

/* ---- Single-level DWT ---- */

void WaveletDenoiser18::dwtLevel(const QVector<double>& signal,
                                   const QVector<double>& lo, const QVector<double>& hi,
                                   QVector<double>& approx, QVector<double>& detail) const
{
    int n = signal.size();
    int fLen = lo.size();
    int outLen = (n + fLen - 1) / 2;

    approx.resize(outLen);
    detail.resize(outLen);

    for (int k = 0; k < outLen; ++k) {
        double aSum = 0.0, dSum = 0.0;
        for (int j = 0; j < fLen; ++j) {
            int idx = 2 * k + j;
            double s = (idx < n) ? signal[idx] : 0.0;
            aSum += lo[j] * s;
            dSum += hi[j] * s;
        }
        approx[k] = aSum;
        detail[k] = dSum;
    }
}

/* ---- Single-level IDWT ---- */

QVector<double> WaveletDenoiser18::idwtLevel(const QVector<double>& approx,
                                               const QVector<double>& detail,
                                               const QVector<double>& lo, const QVector<double>& hi,
                                               int targetLen) const
{
    int fLen = lo.size();
    int n = approx.size();
    int outLen = 2 * n + fLen - 1;
    QVector<double> out(outLen, 0.0);

    for (int k = 0; k < n; ++k) {
        for (int j = 0; j < fLen; ++j) {
            int idx = 2 * k + j;
            out[idx] += lo[j] * approx[k] + hi[j] * detail[k];
        }
    }

    // Trim to target length
    QVector<double> result(targetLen, 0.0);
    for (int i = 0; i < targetLen && i < outLen; ++i) result[i] = out[i];
    return result;
}

/* ---- Estimate noise via MAD ---- */

double WaveletDenoiser18::estimateNoiseMAD(const QVector<double>& coeffs) const
{
    if (coeffs.isEmpty()) return 0.0;
    QVector<double> absCoeffs(coeffs.size());
    for (int i = 0; i < coeffs.size(); ++i) absCoeffs[i] = qAbs(coeffs[i]);
    std::sort(absCoeffs.begin(), absCoeffs.end());
    double median = absCoeffs[absCoeffs.size() / 2];
    return median / 0.6745;  // MAD to sigma conversion
}

/* ---- Estimate prior variance ---- */

double WaveletDenoiser18::estimatePriorVariance(const QVector<double>& coeffs,
                                                  double sigma) const
{
    double sum2 = 0.0;
    for (double c : coeffs) sum2 += c * c;
    double mean2 = sum2 / qMax(coeffs.size(), 1);
    return qMax(mean2 - sigma * sigma, 1e-15);
}

/* ---- Bayesian shrinkage: posterior median ---- */

double WaveletDenoiser18::bayesianShrink(double coeff, double sigma,
                                           double sigmaPrior) const
{
    // Posterior median under Laplace + Gaussian model
    double sigma2 = sigma * sigma;
    double tau = sigmaPrior;
    double absC = qAbs(coeff);

    // Threshold
    double thresh = sigma2 / tau;
    if (absC < thresh) return 0.0;

    // Soft threshold with Bayesian correction
    double sign = (coeff >= 0) ? 1.0 : -1.0;
    double shrunk = absC - thresh;

    // Posterior variance weighting
    double posteriorVar = 1.0 / (1.0 / sigma2 + 1.0 / (tau * tau));
    double weight = posteriorVar / sigma2;

    return sign * shrunk * qMin(weight, 1.0);
}

/* ---- Level-dependent threshold ---- */

double WaveletDenoiser18::levelThreshold(int level, int maxLevel, double sigma) const
{
    // Level-adaptive: higher levels get smaller threshold (coarser scales)
    double baseFactor = qSqrt(2.0 * qLn(qMax(maxLevel * 4.0, 2.0)));
    double levelFactor = 1.0 + 0.3 * (maxLevel - level);
    return sigma * baseFactor * levelFactor;
}

/* ---- Main denoise ---- */

WaveletDenoiser18::DenoiseResult WaveletDenoiser18::denoise(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    DenoiseResult result;
    int n = input.size();
    if (n < 4) {
        result.denoised = input;
        return result;
    }

    QVector<double> lo = lowPassFilter(m_params.wavelet);
    QVector<double> hi = highPassFilter(m_params.wavelet);

    // Determine decomposition levels
    int maxLevels = m_params.levels;
    int maxPossible = 0;
    int tmp = n;
    while (tmp >= 4) { tmp /= 2; maxPossible++; }
    int levels = qMin(maxLevels, maxPossible);
    result.levelsUsed = levels;

    // Forward DWT
    QVector<QVector<double>> details(levels);
    QVector<double> current = input;
    QVector<QVector<double>> approximations(levels);

    for (int lev = 0; lev < levels; ++lev) {
        QVector<double> approx, detail;
        dwtLevel(current, lo, hi, approx, detail);
        details[lev] = detail;
        approximations[lev] = current;
        current = approx;
    }

    // Estimate noise from finest-level detail coefficients
    double sigma = m_params.noiseEstimate;
    if (sigma <= 0.0) sigma = estimateNoiseMAD(details[0]);
    result.estimatedNoise = sigma;

    // Apply Bayesian shrinkage level by level
    result.thresholds.resize(levels);
    for (int lev = 0; lev < levels; ++lev) {
        double thresh = levelThreshold(lev, levels, sigma);
        result.thresholds[lev] = thresh;
        double sigmaPrior = qSqrt(estimatePriorVariance(details[lev], sigma));

        for (int i = 0; i < details[lev].size(); ++i) {
            if (m_params.posteriorMedian) {
                details[lev][i] = bayesianShrink(details[lev][i], sigma, sigmaPrior);
            } else {
                // Soft thresholding fallback
                double absC = qAbs(details[lev][i]);
                details[lev][i] = (absC > thresh)
                    ? ((details[lev][i] > 0 ? 1 : -1) * (absC - thresh)) : 0.0;
            }
        }
    }
    result.detailCoeffs = details;

    // Inverse DWT: reconstruct
    QVector<double> finalApprox = current;
    for (int lev = levels - 1; lev >= 0; --lev) {
        int targetLen = approximations[lev].size();
        finalApprox = idwtLevel(finalApprox, details[lev], lo, hi, targetLen);
    }

    result.denoised = finalApprox;

    double elapsed = timer.elapsed();
    m_stats.inputLength = n;
    m_stats.levelsUsed = levels;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit denoiseDone(n, levels, sigma, elapsed);
    return result;
}

/* ---- Reset ---- */

void WaveletDenoiser18::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
