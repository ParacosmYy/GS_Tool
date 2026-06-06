/**
 * @file PhaseCorrelator.cpp
 * @brief PhaseCorrelator 实现
 *
 * 实现相位相关亚像素对齐：频域互功率谱、高斯亚像素精化、Hann窗、2D FFT。
 */

#include "utils/dsp183/PhaseCorrelator.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PhaseCorrelator::PhaseCorrelator(QObject *parent) : QObject(parent) {}
PhaseCorrelator::~PhaseCorrelator() = default;

/* ---- Configuration ---- */

void PhaseCorrelator::setSubpixelRefinement(bool enabled) { m_subpixel = enabled; }
void PhaseCorrelator::setUseWindow(bool enabled) { m_useWindow = enabled; }

/* ---- Helpers ---- */

int PhaseCorrelator::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

int PhaseCorrelator::log2Int(int n) const
{
    int log = 0;
    while ((1 << log) < n) ++log;
    return log;
}

/* ---- 1D FFT (Cooley-Tukey radix-2) ---- */

void PhaseCorrelator::fft1D(QVector<double>& re, QVector<double>& im, bool inverse) const
{
    int N = re.size();
    if (N <= 1) return;

    // Bit-reversal permutation
    int log2N = log2Int(N);
    for (int i = 0; i < N; ++i) {
        int j = 0;
        for (int b = 0; b < log2N; ++b) j = (j << 1) | ((i >> b) & 1);
        if (j > i) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= N; len *= 2) {
        double angle = sign * 2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < N; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int even = i + j;
                int odd = i + j + len / 2;
                double tRe = curRe * re[odd] - curIm * im[odd];
                double tIm = curRe * im[odd] + curIm * re[odd];
                re[odd] = re[even] - tRe;
                im[odd] = im[even] - tIm;
                re[even] += tRe;
                im[even] += tIm;
                double nRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = nRe;
            }
        }
    }
    if (inverse) {
        for (int i = 0; i < N; ++i) { re[i] /= N; im[i] /= N; }
    }
}

/* ---- 2D FFT ---- */

void PhaseCorrelator::fft2D(QVector<QVector<double>>& re,
                              QVector<QVector<double>>& im, bool inverse) const
{
    int rows = re.size();
    if (rows == 0) return;
    int cols = re[0].size();

    // Row-wise FFT
    for (int r = 0; r < rows; ++r) fft1D(re[r], im[r], inverse);

    // Column-wise FFT
    QVector<double> colRe(rows), colIm(rows);
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) { colRe[r] = re[r][c]; colIm[r] = im[r][c]; }
        fft1D(colRe, colIm, inverse);
        for (int r = 0; r < rows; ++r) { re[r][c] = colRe[r]; im[r][c] = colIm[r]; }
    }
}

/* ---- Hann window ---- */

void PhaseCorrelator::applyHannWindow(QVector<QVector<double>>& img) const
{
    int rows = img.size();
    if (rows == 0) return;
    int cols = img[0].size();
    for (int r = 0; r < rows; ++r) {
        double wy = 0.5 * (1.0 - qCos(2.0 * M_PI * r / (rows - 1)));
        for (int c = 0; c < cols; ++c) {
            double wx = 0.5 * (1.0 - qCos(2.0 * M_PI * c / (cols - 1)));
            img[r][c] *= wx * wy;
        }
    }
}

/* ---- Cross-power spectrum ---- */

QVector<QVector<double>> PhaseCorrelator::crossPowerSpectrum(
    const QVector<QVector<double>>& reA, const QVector<QVector<double>>& imA,
    const QVector<QVector<double>>& reB, const QVector<QVector<double>>& imB) const
{
    int rows = reA.size();
    if (rows == 0) return {};
    int cols = reA[0].size();

    QVector<QVector<double>> result(rows, QVector<double>(cols, 0.0));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // Cross product: A * conj(B)
            double crRe = reA[r][c] * reB[r][c] + imA[r][c] * imB[r][c];
            double crIm = imA[r][c] * reB[r][c] - reA[r][c] * imB[r][c];
            double mag = qSqrt(crRe * crRe + crIm * crIm);
            if (mag > 1e-12) {
                // Normalize: R = A * conj(B) / |A * conj(B)|
                result[r][c] = qAtan2(crIm, crRe);
            }
        }
    }
    return result;
}

/* ---- Find peak ---- */

QPair<int, int> PhaseCorrelator::findPeak(const QVector<QVector<double>>& corr) const
{
    int rows = corr.size();
    if (rows == 0) return {0, 0};
    int cols = corr[0].size();

    int px = 0, py = 0;
    double maxVal = -1e18;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (corr[r][c] > maxVal) { maxVal = corr[r][c]; px = c; py = r; }
        }
    }
    return {px, py};
}

/* ---- Gaussian refinement ---- */

QPair<double, double> PhaseCorrelator::gaussianRefinement(
    const QVector<QVector<double>>& corr, int peakX, int peakY) const
{
    int rows = corr.size();
    if (rows == 0) return {0.0, 0.0};
    int cols = corr[0].size();

    auto safeVal = [&](int r, int c) -> double {
        if (r < 0 || r >= rows || c < 0 || c >= cols) return 0.0;
        return corr[r][c];
    };

    double vp = corr[peakY][peakX];
    double vxl = safeVal(peakY, peakX - 1);
    double vxr = safeVal(peakY, peakX + 1);
    double vyu = safeVal(peakY - 1, peakX);
    double vyd = safeVal(peakY + 1, peakX);

    double dx = 0.0, dy = 0.0;
    double denomX = 2.0 * vp - vxl - vxr;
    if (qAbs(denomX) > 1e-12) dx = (vxl - vxr) / (2.0 * denomX);
    double denomY = 2.0 * vp - vyu - vyd;
    if (qAbs(denomY) > 1e-12) dy = (vyu - vyd) / (2.0 * denomY);

    return {dx, dy};
}

/* ---- 1D alignment ---- */

double PhaseCorrelator::align1D(const QVector<double>& ref,
                                  const QVector<double>& moving) const
{
    int N = qMax(ref.size(), moving.size());
    N = nextPow2(N);

    QVector<double> reA(N, 0.0), imA(N, 0.0), reB(N, 0.0), imB(N, 0.0);
    for (int i = 0; i < ref.size(); ++i) reA[i] = ref[i];
    for (int i = 0; i < moving.size(); ++i) reB[i] = moving[i];

    fft1D(reA, imA, false);
    fft1D(reB, imB, false);

    // Cross-power spectrum
    QVector<double> crossRe(N), crossIm(N);
    for (int i = 0; i < N; ++i) {
        double crRe = reA[i] * reB[i] + imA[i] * imB[i];
        double crIm = imA[i] * reB[i] - reA[i] * imB[i];
        double mag = qSqrt(crRe * crRe + crIm * crIm);
        if (mag > 1e-12) { crossRe[i] = crRe / mag; crossIm[i] = crIm / mag; }
        else { crossRe[i] = 0.0; crossIm[i] = 0.0; }
    }

    fft1D(crossRe, crossIm, true);

    int peak = 0;
    double maxVal = -1e18;
    for (int i = 0; i < N; ++i) {
        if (crossRe[i] > maxVal) { maxVal = crossRe[i]; peak = i; }
    }

    // Wrap around for negative shifts
    double shift = (peak > N / 2) ? peak - N : peak;
    return shift;
}

/* ---- 2D alignment ---- */

PhaseCorrelator::AlignResult PhaseCorrelator::align(
    const QVector<QVector<double>>& ref,
    const QVector<QVector<double>>& moving) const
{
    QElapsedTimer timer;
    timer.start();

    AlignResult result;
    int rows = qMax(ref.size(), moving.size());
    int cols = 0;
    if (ref.size() > 0) cols = qMax(cols, ref[0].size());
    if (moving.size() > 0) cols = qMax(cols, moving[0].size());
    rows = nextPow2(rows);
    cols = nextPow2(cols);

    QVector<QVector<double>> reA(rows, QVector<double>(cols, 0.0));
    QVector<QVector<double>> imA(rows, QVector<double>(cols, 0.0));
    QVector<QVector<double>> reB(rows, QVector<double>(cols, 0.0));
    QVector<QVector<double>> imB(rows, QVector<double>(cols, 0.0));

    for (int r = 0; r < ref.size() && r < rows; ++r)
        for (int c = 0; c < ref[r].size() && c < cols; ++c)
            reA[r][c] = ref[r][c];
    for (int r = 0; r < moving.size() && r < rows; ++r)
        for (int c = 0; c < moving[r].size() && c < cols; ++c)
            reB[r][c] = moving[r][c];

    if (m_useWindow) {
        applyHannWindow(reA);
        applyHannWindow(reB);
    }

    fft2D(reA, imA, false);
    fft2D(reB, imB, false);

    // Cross-power spectrum and inverse FFT
    QVector<QVector<double>> crossRe(rows, QVector<double>(cols, 0.0));
    QVector<QVector<double>> crossIm(rows, QVector<double>(cols, 0.0));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double crRe = reA[r][c] * reB[r][c] + imA[r][c] * imB[r][c];
            double crIm = imA[r][c] * reB[r][c] - reA[r][c] * imB[r][c];
            double mag = qSqrt(crRe * crRe + crIm * crIm);
            if (mag > 1e-12) { crossRe[r][c] = crRe / mag; crossIm[r][c] = crIm / mag; }
        }
    }

    fft2D(crossRe, crossIm, true);

    auto [px, py] = findPeak(crossRe);
    result.peakValue = crossRe[py][px];

    double offX = (px > cols / 2) ? px - cols : px;
    double offY = (py > rows / 2) ? py - rows : py;

    if (m_subpixel) {
        auto [dx, dy] = gaussianRefinement(crossRe, px, py);
        offX += dx;
        offY += dy;
    }

    result.offsetX = offX;
    result.offsetY = offY;
    result.confidence = qBound(0.0, result.peakValue * 10.0, 1.0);

    m_stats.totalAlignments++;
    m_stats.imageWidth = cols;
    m_stats.imageHeight = rows;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAlignments;

    emit alignmentCompleted(offX, offY, result.confidence);
    return result;
}

/* ---- Reset ---- */

void PhaseCorrelator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
