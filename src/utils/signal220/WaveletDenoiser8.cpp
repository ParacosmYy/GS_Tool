/**
 * @file WaveletDenoiser8.cpp
 * @brief WaveletDenoiser8 实现
 *
 * 实现双树复小波变换去噪：双树滤波、二元收缩、尺度间系数依赖。
 */

#include "utils/signal220/WaveletDenoiser8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WaveletDenoiser8::WaveletDenoiser8(QObject *parent) : QObject(parent)
{
    initFilters();
}

WaveletDenoiser8::~WaveletDenoiser8() = default;

/* ---- Initialize dual-tree Q-shift filters ---- */

void WaveletDenoiser8::initFilters()
{
    // Near-symmetric 10-tap filters for dual-tree CWT
    // Tree A low-pass (approximation)
    m_h1a = {0.0357, -0.0147, -0.0499, 0.1450, 0.5450,
             0.5450, 0.1450, -0.0499, -0.0147, 0.0357};
    // Tree B low-pass (Q-shift, ~pi/4 offset)
    m_h1b = {-0.0147, -0.0499, 0.1450, 0.5450, 0.5450,
              0.1450, -0.0499, -0.0147, 0.0357, 0.0};
    // High-pass derived from low-pass via quadrature
    int len = m_h1a.size();
    m_g1a.resize(len);
    m_g1b.resize(len);
    for (int i = 0; i < len; ++i) {
        m_g1a[i] = (i % 2 == 0 ? 1.0 : -1.0) * m_h1a[len - 1 - i];
        m_g1b[i] = (i % 2 == 0 ? 1.0 : -1.0) * m_h1b[len - 1 - i];
    }
}

/* ---- Set parameters ---- */

void WaveletDenoiser8::setParameters(int levels, double thresholdScale)
{
    m_levels = qMax(1, levels);
    m_thresholdScale = qMax(0.1, thresholdScale);
}

/* ---- Filter and downsample ---- */

QVector<double> WaveletDenoiser8::filterDownsample(
    const QVector<double>& input,
    const QVector<double>& filter) const
{
    int n = input.size();
    int fLen = filter.size();
    QVector<double> output((n + 1) / 2, 0.0);
    int outIdx = 0;
    for (int i = 0; i < n; i += 2) {
        double sum = 0.0;
        for (int j = 0; j < fLen; ++j) {
            int idx = (i + j) % n;  // Circular extension
            sum += input[idx] * filter[j];
        }
        output[outIdx++] = sum;
    }
    return output;
}

/* ---- Upsample and filter ---- */

QVector<double> WaveletDenoiser8::upsampleFilter(
    const QVector<double>& input,
    const QVector<double>& filter,
    int targetLen) const
{
    QVector<double> up(2 * input.size() + 1, 0.0);
    for (int i = 0; i < input.size(); ++i) up[2 * i] = input[i];

    int fLen = filter.size();
    QVector<double> output(targetLen, 0.0);
    for (int i = 0; i < targetLen; ++i) {
        double sum = 0.0;
        for (int j = 0; j < fLen; ++j) {
            int idx = i - j + fLen - 1;
            if (idx >= 0 && idx < up.size())
                sum += up[idx] * filter[j];
        }
        output[i] = sum;
    }
    return output;
}

/* ---- Forward dual-tree CWT ---- */

void WaveletDenoiser8::forwardDTCWT(const QVector<double>& input)
{
    m_detailReal.clear();
    m_detailImag.clear();

    QVector<double> approxA = input;
    QVector<double> approxB = input;

    for (int lev = 0; lev < m_levels; ++lev) {
        // Tree A
        QVector<double> detailA = filterDownsample(approxA, m_g1a);
        approxA = filterDownsample(approxA, m_h1a);

        // Tree B
        QVector<double> detailB = filterDownsample(approxB, m_g1b);
        approxB = filterDownsample(approxB, m_h1b);

        // Complex detail = (detailA + j*detailB) / sqrt(2)
        int len = qMin(detailA.size(), detailB.size());
        QVector<double> re(len), im(len);
        for (int i = 0; i < len; ++i) {
            re[i] = detailA[i] / M_SQRT2;
            im[i] = detailB[i] / M_SQRT2;
        }
        m_detailReal.append(re);
        m_detailImag.append(im);
    }

    m_approxReal = approxA;
    m_approxImag = approxB;
}

/* ---- Bivariate shrinkage ---- */

void WaveletDenoiser8::bivariateShrinkage(double sigma)
{
    double tau = m_thresholdScale * sigma;

    for (int lev = 0; lev < m_detailReal.size(); ++lev) {
        int n = m_detailReal[lev].size();
        // Get parent coefficients for interscale dependency
        int parentLev = qMax(0, lev - 1);

        for (int i = 0; i < n; ++i) {
            double wr = m_detailReal[lev][i];
            double wi = m_detailImag[lev][i];
            double wMag = qSqrt(wr * wr + wi * wi);

            // Parent coefficient magnitude
            int pIdx = qMin(i / 2, m_detailReal[parentLev].size() - 1);
            double pr = m_detailReal[parentLev][pIdx];
            double pi = m_detailImag[parentLev][pIdx];
            double pMag = qSqrt(pr * pr + pi * pi);

            // Bivariate shrinkage: y = max(wMag^2 - tau^2, 0) / (wMag^2 + pMag^2)
            double combined = wMag * wMag + pMag * pMag;
            if (combined < 1e-20) {
                m_detailReal[lev][i] = 0.0;
                m_detailImag[lev][i] = 0.0;
                continue;
            }

            double factor = qMax(wMag * wMag - tau * tau, 0.0) / combined;
            factor = qSqrt(factor);  // Apply to magnitude

            m_detailReal[lev][i] = wr * factor;
            m_detailImag[lev][i] = wi * factor;
        }
    }
}

/* ---- Inverse dual-tree CWT ---- */

QVector<double> WaveletDenoiser8::inverseDTCWT()
{
    QVector<double> approxA = m_approxReal;
    QVector<double> approxB = m_approxImag;

    for (int lev = m_detailReal.size() - 1; lev >= 0; --lev) {
        int detailLen = m_detailReal[lev].size();
        int approxLen = approxA.size();

        // Tree A inverse
        QVector<double> upDetail = upsampleFilter(
            m_detailReal[lev], m_g1a, 2 * detailLen);
        QVector<double> upApprox = upsampleFilter(approxA, m_h1a, 2 * detailLen);
        int outLen = qMin(upDetail.size(), upApprox.size());
        approxA.resize(outLen);
        for (int i = 0; i < outLen; ++i)
            approxA[i] = upApprox[i] + upDetail[i];

        // Tree B inverse
        upDetail = upsampleFilter(m_detailImag[lev], m_g1b, 2 * detailLen);
        upApprox = upsampleFilter(approxB, m_h1b, 2 * detailLen);
        outLen = qMin(upDetail.size(), upApprox.size());
        approxB.resize(outLen);
        for (int i = 0; i < outLen; ++i)
            approxB[i] = upApprox[i] + upDetail[i];
    }

    // Average both trees
    int n = qMin(approxA.size(), approxB.size());
    QVector<double> output(n);
    for (int i = 0; i < n; ++i)
        output[i] = (approxA[i] + approxB[i]) * 0.5;

    return output;
}

/* ---- Estimate noise sigma via MAD ---- */

double WaveletDenoiser8::estimateNoiseSigma(
    const QVector<double>& detailCoeffs) const
{
    if (detailCoeffs.isEmpty()) return 0.0;
    QVector<double> absCoeffs(detailCoeffs.size());
    for (int i = 0; i < detailCoeffs.size(); ++i)
        absCoeffs[i] = qAbs(detailCoeffs[i]);
    std::sort(absCoeffs.begin(), absCoeffs.end());
    double median = absCoeffs[absCoeffs.size() / 2];
    return median / 0.6745;  // MAD to sigma conversion
}

/* ---- Denoise ---- */

QVector<double> WaveletDenoiser8::denoise(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.size() < 16) return input;

    // Forward DTCWT
    forwardDTCWT(input);

    // Estimate noise from finest detail coefficients
    double sigma = 0.0;
    if (!m_detailReal.isEmpty()) {
        // Use real part of finest level for MAD estimation
        sigma = estimateNoiseSigma(m_detailReal[0]);
    }

    // Apply bivariate shrinkage
    bivariateShrinkage(sigma);

    // Inverse DTCWT
    QVector<double> output = inverseDTCWT();

    // Trim to input length
    if (output.size() > input.size())
        output.resize(input.size());

    m_stats.signalLength = input.size();
    m_stats.decompLevels = m_levels;
    m_stats.noiseSigma = sigma;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit denoiseCompleted(m_stats.inputSnr, m_stats.outputSnr, timer.elapsed());
    return output;
}

/* ---- Detail coefficients ---- */

QVector<QVector<double>> WaveletDenoiser8::detailCoefficients() const
{
    return m_detailReal;
}

/* ---- Approximation coefficients ---- */

QVector<double> WaveletDenoiser8::approximationCoefficients() const
{
    return m_approxReal;
}

/* ---- Reset ---- */

void WaveletDenoiser8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_detailReal.clear();
    m_detailImag.clear();
    m_approxReal.clear();
    m_approxImag.clear();
}
