/**
 * @file HexFFT4.cpp
 * @brief HexFFT4 实现
 *
 * 实现六角FFT：非正交基变换、六角-矩形网格转换、轴对称频谱分析。
 */

#include "utils/fft184/HexFFT4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HexFFT4::HexFFT4(QObject *parent) : QObject(parent) {}
HexFFT4::~HexFFT4() = default;

/* ---- Configuration ---- */

void HexFFT4::setHexLayout(HexLayout layout) { m_layout = layout; }
void HexFFT4::setHexSize(int size) { m_hexSize = qMax(1, size); }

/* ---- Helpers ---- */

int HexFFT4::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/* ---- Hex coordinate utilities ---- */

int HexFFT4::hexDistance(int q1, int r1, int q2, int r2) const
{
    // Convert axial to cube coordinates
    int s1 = -q1 - r1;
    int s2 = -q2 - r2;
    return (qAbs(q1 - q2) + qAbs(r1 - r2) + qAbs(s1 - s2)) / 2;
}

QVector<QPair<int, int>> HexFFT4::hexNeighbors() const
{
    return {{1, 0}, {1, -1}, {0, -1}, {-1, 0}, {-1, 1}, {0, 1}};
}

QPair<int, int> HexFFT4::hexRound(double q, double r) const
{
    double s = -q - r;
    int rq = qRound(q), rr = qRound(r), rs = qRound(s);
    double dq = qAbs(rq - q), dr = qAbs(rr - r), ds = qAbs(rs - s);
    if (dq > dr && dq > ds) rq = -rr - rs;
    else if (dr > ds) rr = -rq - rs;
    return {rq, rr};
}

/* ---- Hex <-> Rect conversion ---- */

QVector<QVector<double>> HexFFT4::hexToRect(
    const QVector<QVector<double>>& hexData) const
{
    int hexR = hexData.size();
    if (hexR == 0) return {};

    int size = m_hexSize;
    int diam = 2 * size + 1;
    QVector<QVector<double>> rect(diam, QVector<double>(diam, 0.0));

    for (int q = -size; q <= size; ++q) {
        int r1 = qMax(-size, -q - size);
        int r2 = qMin(size, -q + size);
        for (int r = r1; r <= r2; ++r) {
            // Axial to offset coordinates
            int row = r + (q > 0 ? q / 2 : (q - 1) / 2) + size;
            int col = q + size;
            int hexIdx = (q + size) * (size + 1) + r;
            if (row >= 0 && row < diam && col >= 0 && col < diam &&
                hexIdx >= 0 && hexIdx < hexR) {
                int colIdx = r - r1;
                if (colIdx < hexData[hexIdx].size())
                    rect[row][col] = hexData[hexIdx][colIdx];
            }
        }
    }
    return rect;
}

QVector<QVector<double>> HexFFT4::rectToHex(
    const QVector<QVector<double>>& rectData) const
{
    int rows = rectData.size();
    if (rows == 0) return {};

    int size = m_hexSize;
    QVector<QVector<double>> hexData;
    for (int q = -size; q <= size; ++q) {
        int r1 = qMax(-size, -q - size);
        int r2 = qMin(size, -q + size);
        QVector<double> ring;
        for (int r = r1; r <= r2; ++r) {
            int row = r + size;
            int col = q + size;
            if (row >= 0 && row < rows && col >= 0 && col < rectData[row].size())
                ring.append(rectData[row][col]);
            else
                ring.append(0.0);
        }
        hexData.append(ring);
    }
    return hexData;
}

/* ---- Compute twiddles ---- */

void HexFFT4::computeHexTwiddles(int N, QVector<double>& cosT,
                                    QVector<double>& sinT) const
{
    cosT.resize(N);
    sinT.resize(N);
    // Non-orthogonal basis: 60-degree angle between hex axes
    double alpha = (m_layout == HexLayout::PointyTop) ? M_PI / 3.0 : M_PI / 6.0;
    for (int k = 0; k < N; ++k) {
        // Composite twiddle for hex basis vectors
        double angle = -2.0 * M_PI * k / N + alpha * k / N;
        cosT[k] = qCos(angle);
        sinT[k] = qSin(angle);
    }
}

/* ---- 1D FFT ---- */

void HexFFT4::fft1D(QVector<double>& re, QVector<double>& im, bool inverse) const
{
    int N = re.size();
    if (N <= 1) return;
    int log2N = 0;
    while ((1 << log2N) < N) ++log2N;

    // Bit-reversal
    for (int i = 0; i < N; ++i) {
        int j = 0;
        for (int b = 0; b < log2N; ++b) j = (j << 1) | ((i >> b) & 1);
        if (j > i) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }

    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= N; len *= 2) {
        double ang = sign * 2.0 * M_PI / len;
        double wR = qCos(ang), wI = qSin(ang);
        for (int i = 0; i < N; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = i + j + len / 2;
                double tR = cR * re[o] - cI * im[o];
                double tI = cR * im[o] + cI * re[o];
                re[o] = re[e] - tR; im[o] = im[e] - tI;
                re[e] += tR; im[e] += tI;
                double nR = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = nR;
            }
        }
    }
    if (inverse) for (int i = 0; i < N; ++i) { re[i] /= N; im[i] /= N; }
}

/* ---- Basis transform ---- */

void HexFFT4::basisTransform(QVector<double>& re, QVector<double>& im,
                               int N, bool inverse) const
{
    double alpha = (m_layout == HexLayout::PointyTop) ? M_PI / 3.0 : M_PI / 6.0;
    double sign = inverse ? 1.0 : -1.0;

    for (int k = 0; k < N; ++k) {
        double angle = sign * alpha * k / N;
        double cs = qCos(angle), sn = qSin(angle);
        double r = re[k], i = im[k];
        re[k] = cs * r - sn * i;
        im[k] = sn * r + cs * i;
    }
}

/* ---- Forward transform ---- */

void HexFFT4::transform(const QVector<QVector<double>>& hexData,
                          QVector<QVector<double>>& outRe,
                          QVector<QVector<double>>& outIm)
{
    QElapsedTimer timer;
    timer.start();

    // Convert hex to rectangular grid
    auto rectData = hexToRect(hexData);
    int rows = rectData.size();
    if (rows == 0) { outRe.clear(); outIm.clear(); return; }
    int cols = rectData[0].size();

    rows = nextPow2(rows);
    cols = nextPow2(cols);

    outRe.assign(rows, QVector<double>(cols, 0.0));
    outIm.assign(rows, QVector<double>(cols, 0.0));

    for (int r = 0; r < qMin(rows, rectData.size()); ++r)
        for (int c = 0; c < qMin(cols, rectData[r].size()); ++c)
            outRe[r][c] = rectData[r][c];

    // Row-wise FFT + basis transform
    for (int r = 0; r < rows; ++r) {
        fft1D(outRe[r], outIm[r], false);
        basisTransform(outRe[r], outIm[r], cols, false);
    }

    // Column-wise FFT
    QVector<double> colRe(rows), colIm(rows);
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) { colRe[r] = outRe[r][c]; colIm[r] = outIm[r][c]; }
        fft1D(colRe, colIm, false);
        basisTransform(colRe, colIm, rows, false);
        for (int r = 0; r < rows; ++r) { outRe[r][c] = colRe[r]; outIm[r][c] = colIm[r]; }
    }

    m_stats.totalTransforms++;
    m_stats.gridSize = rows;
    m_stats.hexSize = m_hexSize;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(rows, m_hexSize);
}

/* ---- Inverse transform ---- */

void HexFFT4::inverseTransform(const QVector<QVector<double>>& inRe,
                                  const QVector<QVector<double>>& inIm,
                                  QVector<QVector<double>>& hexData)
{
    int rows = inRe.size();
    if (rows == 0) { hexData.clear(); return; }
    int cols = inRe[0].size();

    QVector<QVector<double>> re = inRe, im = inIm;

    // Inverse column FFT
    QVector<double> colRe(rows), colIm(rows);
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) { colRe[r] = re[r][c]; colIm[r] = im[r][c]; }
        basisTransform(colRe, colIm, rows, true);
        fft1D(colRe, colIm, true);
        for (int r = 0; r < rows; ++r) { re[r][c] = colRe[r]; im[r][c] = colIm[r]; }
    }

    // Inverse row FFT
    for (int r = 0; r < rows; ++r) {
        basisTransform(re[r], im[r], cols, true);
        fft1D(re[r], im[r], true);
    }

    // Convert back to hex
    auto rectData = re;
    hexData = rectToHex(rectData);
}

/* ---- Reset ---- */

void HexFFT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
