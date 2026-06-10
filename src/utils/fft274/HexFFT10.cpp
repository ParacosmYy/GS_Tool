/**
 * @file HexFFT10.cpp
 * @brief HexFFT10 实现
 *
 * 实现六角FFT：六角采样定理与三轴频率分解二维六角网格变换。
 */

#include "utils/fft274/HexFFT10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HexFFT10::HexFFT10(QObject *parent)
    : QObject(parent) {}

HexFFT10::~HexFFT10() = default;

/* ---- Configuration ---- */

void HexFFT10::setGridSize(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    m_gridSize = qBound(4, p, 4096);
}

/* ---- Bit-reverse an index ---- */

int HexFFT10::bitReverse(int x, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

/* ---- 1D FFT (Cooley-Tukey decimation-in-time) ---- */

void HexFFT10::fft1D(QVector<double>& real, QVector<double>& imag, bool inverse)
{
    int n = real.size();
    if (n <= 1) return;

    int bits = 0;
    { int tmp = n; while (tmp > 1) { tmp >>= 1; bits++; } }

    // Bit-reversal permutation
    for (int i = 0; i < n; ++i) {
        int j = bitReverse(i, bits);
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    // Butterfly stages
    double dir = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len <<= 1) {
        double angle = dir * 2.0 * M_PI / len;
        double wr = qCos(angle);
        double wi = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curWr = 1.0, curWi = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tr = curWr * real[v] - curWi * imag[v];
                double ti = curWr * imag[v] + curWi * real[v];
                real[v] = real[u] - tr;
                imag[v] = imag[u] - ti;
                real[u] += tr;
                imag[u] += ti;
                double newWr = curWr * wr - curWi * wi;
                curWi = curWr * wi + curWi * wr;
                curWr = newWr;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < n; ++i) {
            real[i] /= n;
            imag[i] /= n;
        }
    }
}

/* ---- Hex twiddle for 3-axis decomposition ---- */

void HexFFT10::hexTwiddle(int k1, int k2, double& wr, double& wi) const
{
    // Hexagonal sampling uses 60-degree rotation basis
    double angle = 2.0 * M_PI * (k1 + 0.5 * k2) / m_gridSize;
    wr = qCos(angle);
    wi = qSin(angle);
}

/* ---- 3-axis frequency decomposition ---- */

void HexFFT10::decompose3Axis(QVector<QVector<double>>& real,
                               QVector<QVector<double>>& imag, bool inverse)
{
    int n = m_gridSize;

    // Axis 1: Row-wise FFT
    for (int i = 0; i < n; ++i)
        fft1D(real[i], imag[i], inverse);

    // Axis 2: Column-wise FFT with hex twiddle
    for (int k1 = 0; k1 < n; ++k1) {
        QVector<double> colR(n), colI(n);
        for (int k2 = 0; k2 < n; ++k2) {
            colR[k2] = real[k2][k1];
            colI[k2] = imag[k2][k1];
        }
        fft1D(colR, colI, inverse);

        // Apply hex twiddle correction
        for (int k2 = 0; k2 < n; ++k2) {
            double wr, wi;
            hexTwiddle(k1, k2, wr, wi);
            double rr = colR[k2] * wr - colI[k2] * wi;
            double ii = colR[k2] * wi + colI[k2] * wr;
            real[k2][k1] = rr;
            imag[k2][k1] = ii;
        }
    }

    // Axis 3: Diagonal (60-degree) FFT via sheared coordinates
    for (int k1 = 0; k1 < n; ++k1) {
        QVector<double> diagR(n), diagI(n);
        for (int k2 = 0; k2 < n; ++k2) {
            int src = (k1 + k2) % n;
            diagR[k2] = real[src][k1];
            diagI[k2] = imag[src][k1];
        }
        fft1D(diagR, diagI, inverse);
        for (int k2 = 0; k2 < n; ++k2) {
            int dst = (k1 + k2) % n;
            real[dst][k1] = diagR[k2];
            imag[dst][k1] = diagI[k2];
        }
    }
}

/* ---- Forward hex FFT ---- */

QVector<QVector<double>> HexFFT10::forward(const QVector<QVector<double>>& hexGrid)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_gridSize;
    QVector<QVector<double>> real(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> imag(n, QVector<double>(n, 0.0));

    for (int i = 0; i < qMin(n, hexGrid.size()); ++i)
        for (int j = 0; j < qMin(n, hexGrid[i].size()); ++j)
            real[i][j] = hexGrid[i][j];

    decompose3Axis(real, imag, false);

    // Pack real+imag into output: interleaved magnitude
    QVector<QVector<double>> spectrum(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            spectrum[i][j] = qSqrt(real[i][j] * real[i][j] + imag[i][j] * imag[i][j]);

    double elapsed = timer.elapsed();
    m_stats.gridSize = n;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(n, m_stats.numTransforms, elapsed);

    return spectrum;
}

/* ---- Inverse hex FFT ---- */

QVector<QVector<double>> HexFFT10::inverse(const QVector<QVector<double>>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_gridSize;
    QVector<QVector<double>> real(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> imag(n, QVector<double>(n, 0.0));

    for (int i = 0; i < qMin(n, spectrum.size()); ++i)
        for (int j = 0; j < qMin(n, spectrum[i].size()); ++j)
            real[i][j] = spectrum[i][j];

    decompose3Axis(real, imag, true);

    double elapsed = timer.elapsed();
    m_stats.gridSize = n;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(n, m_stats.numTransforms, elapsed);

    return real;
}

/* ---- Rect to Hex conversion (offset sampling) ---- */

QVector<QVector<double>> HexFFT10::rectToHex(const QVector<QVector<double>>& rect) const
{
    int n = qMin(m_gridSize, rect.size());
    QVector<QVector<double>> hex(n, QVector<double>(n * 2, 0.0));
    for (int row = 0; row < n; ++row) {
        int offset = (row % 2) * 1;  // Odd rows shifted by half
        for (int col = 0; col < qMin(n, rect[row].size()); ++col) {
            hex[row][col + offset] = rect[row][col];
        }
    }
    return hex;
}

/* ---- Hex to Rect conversion ---- */

QVector<QVector<double>> HexFFT10::hexToRect(const QVector<QVector<double>>& hex) const
{
    int n = qMin(m_gridSize, hex.size());
    QVector<QVector<double>> rect(n, QVector<double>(n, 0.0));
    for (int row = 0; row < n; ++row) {
        int offset = (row % 2) * 1;
        for (int col = 0; col < n; ++col) {
            int srcCol = col + offset;
            if (srcCol < hex[row].size())
                rect[row][col] = hex[row][srcCol];
        }
    }
    return rect;
}

/* ---- Reset ---- */

void HexFFT10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
