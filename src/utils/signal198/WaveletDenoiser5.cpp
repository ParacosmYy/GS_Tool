/**
 * @file WaveletDenoiser5.cpp
 * @brief WaveletDenoiser5 实现
 *
 * 实现小波去噪：DTCWT双树复小波、邻域相关阈值、自适应噪声估计。
 */

#include "utils/signal198/WaveletDenoiser5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WaveletDenoiser5::WaveletDenoiser5(QObject *parent) : QObject(parent) {}
WaveletDenoiser5::~WaveletDenoiser5() = default;

/* ---- Configuration ---- */

void WaveletDenoiser5::setLevels(int levels) { m_levels = qMax(1, levels); }
void WaveletDenoiser5::setThreshold(Threshold method) { m_threshold = method; }
void WaveletDenoiser5::setWaveletLength(int len) { m_waveletLen = qMax(4, len); }

/* ---- DTCWT filters (near-symmetric Q-shift) ---- */

const double* WaveletDenoiser5::filterLoA()
{
    // Tree A lowpass: approximate 10-tap near-symmetric
    static const double h[] = {0.0358, -0.0114, -0.0551, 0.2790, 0.9055,
                               0.2790, -0.0551, -0.0114, 0.0358, 0.0};
    return h;
}

const double* WaveletDenoiser5::filterHiA()
{
    static const double h[] = {0.0, -0.0358, -0.0114, 0.0551, 0.2790,
                               -0.9055, 0.2790, -0.0551, -0.0114, 0.0358};
    return h;
}

const double* WaveletDenoiser5::filterLoB()
{
    static const double h[] = {-0.0114, 0.0358, 0.0, -0.0551, 0.2790,
                               0.9055, 0.2790, -0.0551, 0.0, 0.0358};
    return h;
}

const double* WaveletDenoiser5::filterHiB()
{
    static const double h[] = {-0.0358, -0.0114, 0.0551, 0.2790, -0.9055,
                               -0.2790, -0.0551, 0.0, 0.0358, 0.0114};
    return h;
}

int WaveletDenoiser5::filterLen() { return 10; }

/* ---- Convolve with periodic extension ---- */

QVector<double> WaveletDenoiser5::convolve(const QVector<double>& x,
                                             const double* h, int hLen)
{
    int n = x.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double s = 0.0;
        for (int k = 0; k < hLen; ++k) {
            int idx = (i - k + n) % n;
            s += h[k] * x[idx];
        }
        y[i] = s;
    }
    return y;
}

/* ---- Downsample ---- */

QVector<double> WaveletDenoiser5::downsample(const QVector<double>& x)
{
    int n = (x.size() + 1) / 2;
    QVector<double> y(n);
    for (int i = 0; i < n; ++i) y[i] = x[2 * i];
    return y;
}

/* ---- Upsample ---- */

QVector<double> WaveletDenoiser5::upsample(const QVector<double>& x)
{
    int n = x.size();
    QVector<double> y(2 * n, 0.0);
    for (int i = 0; i < n; ++i) y[2 * i] = x[i];
    return y;
}

/* ---- Universal threshold ---- */

double WaveletDenoiser5::universalThreshold(const QVector<double>& coeffs, double sigma)
{
    return sigma * qSqrt(2.0 * qLn(qMax(1.0, static_cast<double>(coeffs.size()))));
}

/* ---- BayesShrink threshold ---- */

double WaveletDenoiser5::bayesThreshold(const QVector<double>& coeffs, double sigma)
{
    double sumSq = 0.0;
    for (double c : coeffs) sumSq += c * c;
    double var = sumSq / qMax(1, coeffs.size());
    double sigSq = sigma * sigma;
    if (var <= sigSq) return universalThreshold(coeffs, sigma);
    return sigSq / qSqrt(qMax(var - sigSq, 1e-30));
}

/* ---- Neighbor-dependent threshold ---- */

QVector<QVector<double>> WaveletDenoiser5::neighborThreshold(
    const QVector<QVector<double>>& realTree,
    const QVector<QVector<double>>& imagTree,
    double sigma)
{
    int levels = realTree.size();
    QVector<QVector<double>> result(levels);
    for (int lev = 0; lev < levels; ++lev) {
        int n = realTree[lev].size();
        result[lev].resize(n);
        double t = sigma * qSqrt(2.0 * qLn(qMax(1.0, static_cast<double>(n))));

        for (int i = 0; i < n; ++i) {
            // Compute complex magnitude
            double re = realTree[lev][i];
            double im = (i < imagTree[lev].size()) ? imagTree[lev][i] : 0.0;
            double mag = qSqrt(re * re + im * im);

            // Neighbor context: average magnitude of neighbors
            double neighborMag = 0.0;
            int count = 0;
            for (int d = -2; d <= 2; ++d) {
                if (d == 0) continue;
                int ni = i + d;
                if (ni >= 0 && ni < n) {
                    double nr = realTree[lev][ni];
                    double ni2 = (ni < imagTree[lev].size()) ? imagTree[lev][ni] : 0.0;
                    neighborMag += qSqrt(nr * nr + ni2 * ni2);
                    count++;
                }
            }
            neighborMag = (count > 0) ? neighborMag / count : 0.0;

            // Adaptive threshold: lower if neighbors are significant
            double adaptT = t;
            if (neighborMag > t * 0.5) adaptT *= 0.7;

            // Soft threshold
            if (mag > adaptT && mag > 1e-15) {
                double factor = (mag - adaptT) / mag;
                result[lev][i] = re * factor;
            } else {
                result[lev][i] = 0.0;
            }
        }
    }
    return result;
}

/* ---- Forward DTCWT ---- */

QVector<QVector<QVector<double>>> WaveletDenoiser5::dtcwtForward(
    const QVector<double>& signal) const
{
    int fLen = filterLen();
    QVector<QVector<QVector<double>>> coeffs(m_levels);

    QVector<double> aA = signal;
    QVector<double> aB = signal;

    for (int lev = 0; lev < m_levels; ++lev) {
        // Tree A
        auto loA = convolve(aA, filterLoA(), fLen);
        auto hiA = convolve(aA, filterHiA(), fLen);
        // Tree B
        auto loB = convolve(aB, filterLoB(), fLen);
        auto hiB = convolve(aB, filterHiB(), fLen);

        // Downsample detail coefficients
        auto dA = downsample(hiA);
        auto dB = downsample(hiB);

        // Store: [real_detail, imag_detail] per level
        coeffs[lev] = {dA, dB};

        // Continue with approximation
        aA = downsample(loA);
        aB = downsample(loB);
    }

    // Store final approximation
    coeffs.append({aA, aB});
    return coeffs;
}

/* ---- Inverse DTCWT ---- */

QVector<double> WaveletDenoiser5::dtcwtInverse(
    const QVector<QVector<QVector<double>>>& coeffs) const
{
    if (coeffs.isEmpty()) return {};

    int fLen = filterLen();
    // Start from final approximation
    auto aA = coeffs.last()[0];
    auto aB = coeffs.last().size() > 1 ? coeffs.last()[1] : coeffs.last()[0];

    for (int lev = coeffs.size() - 2; lev >= 0; --lev) {
        auto dA = coeffs[lev][0];
        auto dB = coeffs[lev].size() > 1 ? coeffs[lev][1] : QVector<double>(dA.size(), 0.0);

        // Upsample
        auto upAA = upsample(aA);
        auto upDA = upsample(dA);
        auto upAB = upsample(aB);
        auto upDB = upsample(dB);

        // Inverse filter (transposed convolution)
        int targetLen = upAA.size() + fLen - 1;
        QVector<double> newAA(targetLen, 0.0);
        for (int i = 0; i < targetLen; ++i) {
            double sA = 0.0, sD = 0.0;
            for (int k = 0; k < fLen; ++k) {
                int idx = i - k;
                if (idx >= 0 && idx < upAA.size()) {
                    sA += filterLoA()[k] * upAA[idx];
                    sD += filterHiA()[k] * upDA[idx];
                }
            }
            newAA[i] = sA + sD;
        }

        QVector<double> newAB(targetLen, 0.0);
        for (int i = 0; i < targetLen; ++i) {
            double sA = 0.0, sD = 0.0;
            for (int k = 0; k < fLen; ++k) {
                int idx = i - k;
                if (idx >= 0 && idx < upAB.size()) {
                    sA += filterLoB()[k] * upAB[idx];
                    sD += filterHiB()[k] * upDB[idx];
                }
            }
            newAB[i] = sA + sD;
        }

        aA = newAA;
        aB = newAB;
    }

    // Average trees
    int n = qMin(aA.size(), aB.size());
    QVector<double> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = 0.5 * (aA[i] + aB[i]);
    return result;
}

/* ---- Estimate noise ---- */

double WaveletDenoiser5::estimateNoise(const QVector<double>& signal) const
{
    if (signal.size() < 2) return 0.0;
    // MAD (Median Absolute Deviation) of finest-level detail
    QVector<double> diffs(signal.size() - 1);
    for (int i = 0; i < diffs.size(); ++i)
        diffs[i] = qAbs(signal[i + 1] - signal[i]);

    QVector<double> sorted = diffs;
    std::sort(sorted.begin(), sorted.end());
    double median = sorted[sorted.size() / 2];

    // MAD estimator for Gaussian noise: sigma = MAD / 0.6745
    return median / 0.6745;
}

/* ---- Denoise ---- */

QVector<double> WaveletDenoiser5::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 4) return signal;

    double sigma = estimateNoise(signal);

    // Forward DTCWT
    auto coeffs = dtcwtForward(signal);

    // Threshold detail coefficients
    int detailLevels = coeffs.size() - 1; // last is approximation
    QVector<QVector<double>> realTree(detailLevels);
    QVector<QVector<double>> imagTree(detailLevels);

    for (int lev = 0; lev < detailLevels; ++lev) {
        realTree[lev] = coeffs[lev][0];
        imagTree[lev] = (coeffs[lev].size() > 1) ? coeffs[lev][1]
                     : QVector<double>(coeffs[lev][0].size(), 0.0);
    }

    // Apply threshold
    QVector<QVector<QVector<double>>> thrCoeffs(coeffs.size());
    if (m_threshold == NeighborDependent) {
        auto thrReal = neighborThreshold(realTree, imagTree, sigma);
        for (int lev = 0; lev < detailLevels; ++lev)
            thrCoeffs[lev] = {thrReal[lev], imagTree[lev]};
    } else {
        for (int lev = 0; lev < detailLevels; ++lev) {
            double t = (m_threshold == BayesShrink)
                       ? bayesThreshold(realTree[lev], sigma)
                       : universalThreshold(realTree[lev], sigma);
            QVector<double> thr(realTree[lev].size());
            for (int i = 0; i < thr.size(); ++i) {
                double c = realTree[lev][i];
                thr[i] = (qAbs(c) > t) ? c - t * (c > 0 ? 1.0 : -1.0) : 0.0;
            }
            thrCoeffs[lev] = {thr, imagTree[lev]};
        }
    }
    thrCoeffs[detailLevels] = coeffs[detailLevels]; // keep approximation

    m_lastCoeffs = thrCoeffs;

    // Inverse DTCWT
    auto result = dtcwtInverse(thrCoeffs);

    // Resize to match input
    if (result.size() > n) result.resize(n);
    while (result.size() < n) result.append(0.0);

    m_stats.totalDenoise++;
    m_stats.signalLength = n;
    m_stats.levels = m_levels;
    m_stats.noiseEstimate = sigma;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDenoise;

    emit denoiseCompleted(n, sigma, timer.elapsed());
    return result;
}

/* ---- Last coefficients ---- */

QVector<QVector<QVector<double>>> WaveletDenoiser5::lastCoefficients() const
{
    return m_lastCoeffs;
}

/* ---- Reset ---- */

void WaveletDenoiser5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_lastCoeffs.clear();
}
