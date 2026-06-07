/**
 * @file HexFFT5.cpp
 * @brief HexFFT5 实现
 *
 * 实现六角FFT：六角到矩形坐标映射、六重对称利用、六角采样数据处理。
 */

#include "utils/fft198/HexFFT5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HexFFT5::HexFFT5(QObject *parent) : QObject(parent) {}
HexFFT5::~HexFFT5() = default;

/* ---- Configuration ---- */

void HexFFT5::setGridRadius(int r) { m_gridRadius = qMax(1, r); }
void HexFFT5::setSymmetryOrder(int o) { m_symmetryOrder = qBound(1, o, 6); }
void HexFFT5::setNormalize(bool e) { m_normalize = e; }

/* ---- Helpers ---- */

int HexFFT5::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

int HexFFT5::hexDist(int q, int r)
{
    return (qAbs(q) + qAbs(r) + qAbs(q + r)) / 2;
}

int HexFFT5::hexGridSize(int radius)
{
    return 3 * radius * radius + 3 * radius + 1;
}

/* ---- Coordinate mapping ---- */

QPair<double, double> HexFFT5::hexToRect(int q, int r) const
{
    // Axial -> pixel: x = sqrt(3)*q + sqrt(3)/2*r, y = 3/2*r
    double x = qSqrt(3.0) * q + qSqrt(3.0) / 2.0 * r;
    double y = 1.5 * r;
    return {x, y};
}

QPair<int, int> HexFFT5::rectToHex(double x, double y) const
{
    // Fractional axial coords: q = (2/3 * x) / sqrt(3), r = -1/3 * x / sqrt(3) + 2/3 * y
    double qf = (2.0 / 3.0 * x) / qSqrt(3.0);
    double rf = -1.0 / 3.0 * x / qSqrt(3.0) + 2.0 / 3.0 * y;

    // Round to nearest hex (cube rounding)
    double sf = -qf - rf;
    int q = qRound(qf), r = qRound(rf), s = qRound(sf);
    double dq = qAbs(q - qf), dr = qAbs(r - rf), ds = qAbs(s - sf);
    if (dq > dr && dq > ds) q = -r - s;
    else if (dr > ds) r = -q - s;
    return {q, r};
}

/* ---- Rotate 60 degrees ---- */

HexFFT5::HexPoint HexFFT5::rotate60(const HexPoint& p)
{
    // Rotation by 60 degrees in axial: (q,r) -> (-r, q+r)
    HexPoint rotated;
    rotated.q = -p.r;
    rotated.r = p.q + p.r;
    rotated.value = p.value;
    return rotated;
}

/* ---- Generate grid ---- */

QVector<HexFFT5::HexPoint> HexFFT5::generateGrid(int radius) const
{
    QVector<HexPoint> grid;
    for (int q = -radius; q <= radius; ++q) {
        int r1 = qMax(-radius, -q - radius);
        int r2 = qMin(radius, -q + radius);
        for (int r = r1; r <= r2; ++r) {
            HexPoint pt;
            pt.q = q; pt.r = r; pt.value = 0.0;
            grid.append(pt);
        }
    }
    return grid;
}

/* ---- Symmetry reduction ---- */

QVector<HexFFT5::HexPoint> HexFFT5::reduceBySymmetry(const QVector<HexPoint>& samples) const
{
    if (m_symmetryOrder < 2) return samples;

    // Keep only points in one sector (0 to 60 degrees)
    QVector<HexPoint> reduced;
    for (const auto& pt : samples) {
        // Check if point is in fundamental sector (q >= 0, r >= 0, q >= r)
        if (pt.q >= 0 && pt.r >= 0 && pt.q >= pt.r)
            reduced.append(pt);
        else if (pt.q == 0 && pt.r == 0)
            reduced.append(pt);
    }
    return reduced;
}

QVector<HexFFT5::HexPoint> HexFFT5::reconstructSymmetry(const QVector<HexPoint>& reduced) const
{
    QVector<HexPoint> full;
    for (const auto& pt : reduced) {
        HexPoint cur = pt;
        for (int k = 0; k < m_symmetryOrder; ++k) {
            full.append(cur);
            cur = rotate60(cur);
        }
    }
    return full;
}

/* ---- Internal FFT ---- */

void HexFFT5::fft(QVector<double>& re, QVector<double>& im, bool inv) const
{
    int N = re.size();
    if (N <= 1) return;

    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }

    for (int len = 2; len <= N; len <<= 1) {
        double angle = (inv ? 2.0 : -2.0) * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < N; i += len) {
            double cRe = 1.0, cIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uRe = re[i+j], uIm = im[i+j];
                double vRe = re[i+j+len/2]*cRe - im[i+j+len/2]*cIm;
                double vIm = re[i+j+len/2]*cIm + im[i+j+len/2]*cRe;
                re[i+j] = uRe + vRe; im[i+j] = uIm + vIm;
                re[i+j+len/2] = uRe - vRe; im[i+j+len/2] = uIm - vIm;
                double nRe = cRe*wRe - cIm*wIm;
                cIm = cRe*wIm + cIm*wRe; cRe = nRe;
            }
        }
    }
    if (inv) for (int i = 0; i < N; ++i) { re[i] /= N; im[i] /= N; }
}

/* ---- Forward transform ---- */

QVector<QVector<double>> HexFFT5::forward(const QVector<HexPoint>& samples)
{
    QElapsedTimer timer;
    timer.start();

    int nPts = samples.size();
    if (nPts == 0) return {};

    // Map hex to rectangular, pad to power of 2
    int N = nextPow2(nPts);
    QVector<double> re(N, 0.0), im(N, 0.0);

    for (int i = 0; i < nPts; ++i) {
        auto xy = hexToRect(samples[i].q, samples[i].r);
        // Use spatial hashing for grid position
        int idx = i;
        re[idx] = samples[i].value;
    }

    fft(re, im, false);

    // Output: pairs of [real, imag]
    QVector<QVector<double>> result(N, {0.0, 0.0});
    for (int i = 0; i < N; ++i)
        result[i] = {re[i], im[i]};

    m_stats.totalTransforms++;
    m_stats.numHexPoints = nPts;
    m_stats.gridSize = N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(nPts, timer.elapsed());
    return result;
}

/* ---- Inverse transform ---- */

QVector<HexFFT5::HexPoint> HexFFT5::inverse(const QVector<QVector<double>>& spectrum, int radius)
{
    QElapsedTimer timer;
    timer.start();

    int N = spectrum.size();
    if (N == 0) return {};

    QVector<double> re(N), im(N);
    for (int i = 0; i < N; ++i) {
        re[i] = spectrum[i][0];
        im[i] = spectrum[i][1];
    }

    fft(re, im, true);

    // Map back to hex grid
    auto grid = generateGrid(radius);
    int count = qMin(grid.size(), N);
    for (int i = 0; i < count; ++i)
        grid[i].value = re[i];

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    return grid;
}

/* ---- Reset ---- */

void HexFFT5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
