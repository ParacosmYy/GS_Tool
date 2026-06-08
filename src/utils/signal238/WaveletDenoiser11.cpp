/**
 * @file WaveletDenoiser11.cpp
 * @brief WaveletDenoiser11 实现
 *
 * 实现小波去噪器：双树复小波变换与跨尺度依赖双变量收缩。
 */

#include "utils/signal238/WaveletDenoiser11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WaveletDenoiser11::WaveletDenoiser11(QObject *parent) : QObject(parent) {}
WaveletDenoiser11::~WaveletDenoiser11() = default;

/* ---- Configuration ---- */

void WaveletDenoiser11::setDecompositionLevels(int levels) { m_levels = qMax(0, levels); }
void WaveletDenoiser11::setNoiseThreshold(double threshold) { m_noiseThreshold = qMax(0.1, threshold); }
void WaveletDenoiser11::setWaveletType(int type) { m_waveletType = qBound(0, type, 2); }

/* ---- Auto-select levels ---- */

int WaveletDenoiser11::autoLevels(int signalLen) const
{
    int lev = 0;
    while ((1 << lev) < signalLen && lev < 10) lev++;
    return qMax(1, lev - 2);
}

/* ---- Wavelet filter coefficients ---- */

QVector<double> WaveletDenoiser11::lowPassFilter() const
{
    if (m_waveletType == 0) {
        // Haar
        return {0.7071067811865476, 0.7071067811865476};
    } else if (m_waveletType == 1) {
        // DB2 (Daubechies 2)
        return {0.4829629131445341, 0.8365163037378079, 0.2241438680420134, -0.1294095225512603};
    } else {
        // DB4 (Daubechies 4)
        return {0.2303778133088964, 0.7148465705529154, 0.6308807679398587, -0.0279837694168599,
                -0.1870348117190931, 0.0308413818355607, 0.0328830116668852, -0.0105974017850690};
    }
}

QVector<double> WaveletDenoiser11::highPassFilter() const
{
    QVector<double> lp = lowPassFilter();
    int n = lp.size();
    QVector<double> hp(n);
    for (int i = 0; i < n; ++i)
        hp[i] = (i % 2 == 0 ? 1 : -1) * lp[n - 1 - i];
    return hp;
}

/* ---- Single-level decomposition ---- */

void WaveletDenoiser11::decomposeOneLevel(const QVector<double>& input,
                                            QVector<double>& approx,
                                            QVector<double>& detail) const
{
    QVector<double> lp = lowPassFilter();
    QVector<double> hp = highPassFilter();
    int n = input.size();
    int fLen = lp.size();
    int outLen = (n + fLen - 1) / 2;

    approx.resize(outLen);
    detail.resize(outLen);

    for (int i = 0; i < outLen; ++i) {
        double a = 0.0, d = 0.0;
        for (int k = 0; k < fLen; ++k) {
            int idx = 2 * i + k;
            if (idx < n) {
                a += lp[k] * input[idx];
                d += hp[k] * input[idx];
            }
        }
        approx[i] = a;
        detail[i] = d;
    }
}

/* ---- Single-level reconstruction ---- */

QVector<double> WaveletDenoiser11::reconstructOneLevel(const QVector<double>& approx,
                                                         const QVector<double>& detail) const
{
    QVector<double> lp = lowPassFilter();
    QVector<double> hp = highPassFilter();
    int fLen = lp.size();
    int n = approx.size();
    int outLen = 2 * n;

    QVector<double> result(outLen, 0.0);

    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < fLen; ++k) {
            int idx = 2 * i + k;
            if (idx < outLen) {
                result[idx] += lp[k] * approx[i] + hp[k] * detail[i];
            }
        }
    }
    return result;
}

/* ---- DTCWT forward ---- */

void WaveletDenoiser11::dtcwtForward(const QVector<double>& signal)
{
    int levels = (m_levels > 0) ? m_levels : autoLevels(signal.size());
    levels = qMin(levels, 10);
    m_detailCoeffs.clear();
    m_detailCoeffs.resize(levels);

    QVector<double> current = signal;
    for (int lev = 0; lev < levels; ++lev) {
        QVector<double> approx, detail;
        decomposeOneLevel(current, approx, detail);
        m_detailCoeffs[lev] = detail;
        current = approx;
    }
    m_approxCoeffs = current;
}

/* ---- DTCWT inverse ---- */

QVector<double> WaveletDenoiser11::dtcwtInverse()
{
    int levels = m_detailCoeffs.size();
    QVector<double> current = m_approxCoeffs;

    for (int lev = levels - 1; lev >= 0; --lev) {
        current = reconstructOneLevel(current, m_detailCoeffs[lev]);
    }
    return current;
}

/* ---- Estimate noise std via MAD ---- */

double WaveletDenoiser11::estimateNoiseStd(const QVector<double>& detailCoeffs) const
{
    if (detailCoeffs.isEmpty()) return 0.0;
    QVector<double> absCoeffs;
    absCoeffs.reserve(detailCoeffs.size());
    for (double c : detailCoeffs)
        absCoeffs.append(qAbs(c));

    std::sort(absCoeffs.begin(), absCoeffs.end());
    double median = absCoeffs[absCoeffs.size() / 2];

    // MAD estimator: sigma = median / 0.6745
    return median / 0.6745;
}

/* ---- Bivariate shrinkage with interscale dependency ---- */

void WaveletDenoiser11::bivariateShrinkage(QVector<double>& childDetail,
                                             const QVector<double>& parentDetail,
                                             double noiseStd)
{
    int n = childDetail.size();
    if (n == 0) return;

    // Compute signal variance from child coefficients
    double sigVar = 0.0;
    for (int i = 0; i < n; ++i)
        sigVar += childDetail[i] * childDetail[i];
    sigVar /= n;
    sigVar = qMax(0.0, sigVar - noiseStd * noiseStd);

    if (sigVar < 1e-15) {
        childDetail.fill(0.0);
        return;
    }

    // Bivariate shrinkage: threshold T = sqrt(3) * noiseStd^2 / sqrt(sigVar)
    double T = qSqrt(3.0) * noiseStd * noiseStd / qSqrt(sigVar);
    T *= m_noiseThreshold / 3.0;  // adjust with user threshold

    for (int i = 0; i < n; ++i) {
        // Interscale dependency: parent coefficient influence
        double parentMag = 0.0;
        int parentIdx = qMin(i, parentDetail.size() - 1);
        if (parentIdx >= 0) parentMag = qAbs(parentDetail[parentIdx]);

        // Combined magnitude
        double combinedMag = qSqrt(childDetail[i] * childDetail[i] + parentMag * parentMag);

        if (combinedMag < T) {
            childDetail[i] = 0.0;
        } else {
            // Soft thresholding with bivariate model
            double factor = qMax(0.0, 1.0 - T * T / (combinedMag * combinedMag + 1e-15));
            childDetail[i] *= factor;
        }
    }
}

/* ---- Compute SNR ---- */

double WaveletDenoiser11::computeSNR(const QVector<double>& signal, const QVector<double>& noise)
{
    double sigPow = 0.0, noisePow = 0.0;
    int n = qMin(signal.size(), noise.size());
    for (int i = 0; i < n; ++i) {
        sigPow += signal[i] * signal[i];
        noisePow += noise[i] * noise[i];
    }
    if (noisePow < 1e-15) return 100.0;
    return 10.0 * qLn(sigPow / noisePow) / qLn(10.0);
}

/* ---- Denoise ---- */

WaveletDenoiser11::DenoiseResult WaveletDenoiser11::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();
    DenoiseResult result;

    int n = signal.size();
    if (n < 4) {
        result.signal = signal;
        return result;
    }

    // Forward DTCWT
    dtcwtForward(signal);

    int levels = m_detailCoeffs.size();

    // Estimate noise from finest level
    double noiseStd = estimateNoiseStd(m_detailCoeffs[0]);
    result.noiseEstimate = noiseStd;

    // Apply bivariate shrinkage level by level (coarse to fine)
    for (int lev = 0; lev < levels; ++lev) {
        // Parent coefficients for interscale dependency
        QVector<double> parent;
        if (lev + 1 < levels)
            parent = m_detailCoeffs[lev + 1];
        else
            parent = m_approxCoeffs;

        // Scale noise estimate for each level
        double levelNoise = noiseStd;

        bivariateShrinkage(m_detailCoeffs[lev], parent, levelNoise);
    }

    // Inverse DTCWT
    result.signal = dtcwtInverse();

    // Trim to original length
    if (result.signal.size() > n)
        result.signal.resize(n);

    result.levelsUsed = levels;

    // Estimate SNR improvement
    QVector<double> residual(n);
    for (int i = 0; i < n && i < result.signal.size(); ++i)
        residual[i] = signal[i] - result.signal[i];
    result.outputSNR = computeSNR(result.signal, residual);
    result.inputSNR = 0.0;  // unknown without ground truth

    m_stats.signalLength = n;
    m_stats.decompLevels = levels;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    double snrImprovement = result.outputSNR - result.inputSNR;
    m_stats.avgNoiseReductionDb = (m_stats.avgNoiseReductionDb * (m_stats.totalOps - 1) + snrImprovement) / m_stats.totalOps;

    emit denoiseCompleted(n, snrImprovement, timer.elapsed());
    return result;
}

/* ---- Detail coefficients accessor ---- */

QVector<QVector<double>> WaveletDenoiser11::detailCoefficients() const { return m_detailCoeffs; }

/* ---- Reset ---- */

void WaveletDenoiser11::resetStatistics()
{
    m_approxCoeffs.clear();
    m_detailCoeffs.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
