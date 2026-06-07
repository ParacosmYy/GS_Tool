/**
 * @file PhaseCorrelator2.cpp
 * @brief PhaseCorrelator2 实现
 *
 * 实现相位相关：归一化互功率谱、亚像素峰值插值、二维图像配准。
 */

#include "utils/dsp199/PhaseCorrelator2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

PhaseCorrelator2::PhaseCorrelator2(QObject *parent) : QObject(parent) {}
PhaseCorrelator2::~PhaseCorrelator2() = default;

/* ---- Configuration ---- */

void PhaseCorrelator2::setSubpixelInterpolation(bool e) { m_subpixel = e; }
void PhaseCorrelator2::setWindowSize(int sz) { m_windowSize = qMax(16, sz); }

/* ---- DFT ---- */

void PhaseCorrelator2::dft(const QVector<double>& inRe, const QVector<double>& inIm,
                            QVector<double>& outRe, QVector<double>& outIm, bool inverse) const
{
    int n = inRe.size();
    double sign = inverse ? 1.0 : -1.0;
    double scale = inverse ? 1.0 / n : 1.0;

    for (int k = 0; k < n; ++k) {
        double re = 0.0, im = 0.0;
        for (int j = 0; j < n; ++j) {
            double angle = sign * 2.0 * M_PI * j * k / n;
            double cR = qCos(angle), cI = qSin(angle);
            re += inRe[j] * cR - inIm[j] * cI;
            im += inRe[j] * cI + inIm[j] * cR;
        }
        outRe[k] = re * scale;
        outIm[k] = im * scale;
    }
}

/* ---- Windows ---- */

QVector<double> PhaseCorrelator2::applyHanning(const QVector<double>& signal) const
{
    int n = signal.size();
    QVector<double> out(n);
    for (int i = 0; i < n; ++i)
        out[i] = signal[i] * (0.5 - 0.5 * qCos(2.0 * M_PI * i / (n - 1)));
    return out;
}

QVector<double> PhaseCorrelator2::applyHamming(const QVector<double>& signal) const
{
    int n = signal.size();
    QVector<double> out(n);
    for (int i = 0; i < n; ++i)
        out[i] = signal[i] * (0.54 - 0.46 * qCos(2.0 * M_PI * i / (n - 1)));
    return out;
}

/* ---- Cross-power spectrum ---- */

void PhaseCorrelator2::crossPowerSpectrum(
    const QVector<double>& re1, const QVector<double>& im1,
    const QVector<double>& re2, const QVector<double>& im2,
    QVector<double>& outRe, QVector<double>& outIm) const
{
    int n = re1.size();
    outRe.resize(n); outIm.resize(n);

    for (int i = 0; i < n; ++i) {
        // Cross product: F1 * conj(F2)
        double crR = re1[i] * re2[i] + im1[i] * im2[i];
        double crI = im1[i] * re2[i] - re1[i] * im2[i];
        double mag = qSqrt(crR * crR + crI * crI);
        if (mag > 1e-12) {
            outRe[i] = crR / mag;
            outIm[i] = crI / mag;
        } else {
            outRe[i] = 0.0; outIm[i] = 0.0;
        }
    }
}

/* ---- Find peak ---- */

int PhaseCorrelator2::findPeak(const QVector<double>& data) const
{
    int best = 0;
    double bestVal = data[0];
    for (int i = 1; i < data.size(); ++i)
        if (data[i] > bestVal) { bestVal = data[i]; best = i; }
    return best;
}

/* ---- Interpolate peak ---- */

PhaseCorrelator2::ShiftResult PhaseCorrelator2::interpolatePeak(
    const QVector<double>& correlation) const
{
    int n = correlation.size();
    if (n == 0) return {0.0, 0.0, 0.0, 0.0};

    int peak = findPeak(correlation);
    double peakVal = correlation[peak];
    ShiftResult result;
    result.peakValue = peakVal;

    if (!m_subpixel || n < 3) {
        result.dx = static_cast<double>(peak);
        result.confidence = peakVal;
        return result;
    }

    // Gaussian sub-pixel interpolation using 3-point fit
    int left = (peak - 1 + n) % n;
    int right = (peak + 1) % n;
    double yL = correlation[left];
    double yR = correlation[right];
    double denom = 2.0 * (2.0 * peakVal - yL - yR);

    if (qAbs(denom) > 1e-12) {
        double offset = (yR - yL) / denom;
        result.dx = static_cast<double>(peak) + offset;
    } else {
        result.dx = static_cast<double>(peak);
    }

    // Wrap around for circular shift
    if (result.dx > n / 2.0) result.dx -= n;
    result.confidence = peakVal;
    return result;
}

/* ---- 1D correlate ---- */

double PhaseCorrelator2::correlate1D(const QVector<double>& signal1,
                                      const QVector<double>& signal2)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMax(signal1.size(), signal2.size());

    // Pad to equal length
    QVector<double> s1(n, 0.0), s2(n, 0.0);
    for (int i = 0; i < signal1.size(); ++i) s1[i] = signal1[i];
    for (int i = 0; i < signal2.size(); ++i) s2[i] = signal2[i];

    // Window to reduce leakage
    s1 = applyHanning(s1);
    s2 = applyHanning(s2);

    // Forward DFT
    QVector<double> f1Re(n, 0.0), f1Im(n, 0.0), f2Re(n, 0.0), f2Im(n, 0.0);
    dft(s1, QVector<double>(n, 0.0), f1Re, f1Im, false);
    dft(s2, QVector<double>(n, 0.0), f2Re, f2Im, false);

    // Cross-power spectrum
    QVector<double> cpRe(n), cpIm(n);
    crossPowerSpectrum(f1Re, f1Im, f2Re, f2Im, cpRe, cpIm);

    // Inverse DFT
    QVector<double> corrRe(n), corrIm(n);
    dft(cpRe, cpIm, corrRe, corrIm, true);

    // Extract magnitudes (real part of correlation)
    QVector<double> corrMag(n);
    for (int i = 0; i < n; ++i)
        corrMag[i] = qSqrt(corrRe[i] * corrRe[i] + corrIm[i] * corrIm[i]);

    auto result = interpolatePeak(corrMag);

    m_stats.totalCorrelations++;
    m_stats.lastSize = n;
    m_stats.lastPeakValue = result.peakValue;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCorrelations;

    emit correlationCompleted(result.dx, result.peakValue, timer.elapsed());
    return result.dx;
}

/* ---- 2D correlate ---- */

PhaseCorrelator2::ShiftResult PhaseCorrelator2::correlate2D(
    const QVector<QVector<double>>& img1, const QVector<QVector<double>>& img2)
{
    QElapsedTimer timer;
    timer.start();

    int rows = qMax(img1.size(), img2.size());
    int cols = (rows > 0) ? qMax(img1[0].size(), img2[0].size()) : 0;

    ShiftResult result;

    // Row-wise correlation for vertical shift
    QVector<double> col1(rows, 0.0), col2(rows, 0.0);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double v1 = (r < img1.size() && c < img1[0].size()) ? img1[r][c] : 0.0;
            double v2 = (r < img2.size() && c < img2[0].size()) ? img2[r][c] : 0.0;
            col1[r] += v1; col2[r] += v2;
        }
        col1[r] /= cols; col2[r] /= cols;
    }
    result.dy = correlate1D(col1, col2);

    // Column-wise correlation for horizontal shift
    QVector<double> row1(cols, 0.0), row2(cols, 0.0);
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) {
            double v1 = (r < img1.size() && c < img1[0].size()) ? img1[r][c] : 0.0;
            double v2 = (r < img2.size() && c < img2[0].size()) ? img2[r][c] : 0.0;
            row1[c] += v1; row2[c] += v2;
        }
        row1[c] /= rows; row2[c] /= rows;
    }
    result.dx = correlate1D(row1, row2);
    result.confidence = m_stats.lastPeakValue;

    m_timeSum += timer.elapsed();

    emit correlationCompleted(result.dx, result.confidence, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void PhaseCorrelator2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
