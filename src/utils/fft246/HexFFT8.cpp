/**
 * @file HexFFT8.cpp
 * @brief HexFFT8 实现
 *
 * 实现六边形FFT：交错网格采样与六边形频域重建。
 */

#include "utils/fft246/HexFFT8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HexFFT8::HexFFT8(QObject *parent) : QObject(parent) { buildGrid(); precomputeTwiddle(); }
HexFFT8::~HexFFT8() = default;

/* ---- Configuration ---- */

void HexFFT8::setGridRadius(int radius)
{
    m_radius = qMax(1, radius);
    buildGrid();
    precomputeTwiddle();
}

/* ---- Build hex grid ---- */

void HexFFT8::buildGrid()
{
    m_grid.clear();
    int R = m_radius;
    for (int q = -R; q <= R; ++q) {
        int r1 = qMax(-R, -q - R);
        int r2 = qMin(R, -q + R);
        for (int r = r1; r <= r2; ++r) {
            m_grid.append({q, r});
        }
    }
}

/* ---- Axial-to-pixel ---- */

QPair<double, double> HexFFT8::axialToPixel(int q, int r) const
{
    // Pointy-top hex layout
    double x = qSqrt(3.0) * q + qSqrt(3.0) / 2.0 * r;
    double y = 1.5 * r;
    return {x, y};
}

/* ---- Hex DFT kernel ---- */

double HexFFT8::hexDFTKernel(int q1, int r1, int q2, int r2, bool inv) const
{
    // Hex Fourier basis on axial coordinates
    auto [x1, y1] = axialToPixel(q1, r1);
    auto [x2, y2] = axialToPixel(q2, r2);
    double phase = 2.0 * M_PI * (x1 * x2 + y1 * y2) / (m_radius * m_radius * 3.0);
    if (inv) phase = -phase;
    return phase;
}

/* ---- Precompute twiddle factors ---- */

void HexFFT8::precomputeTwiddle()
{
    int n = m_grid.size();
    m_twiddle.resize(n);
    for (int i = 0; i < n; ++i) {
        m_twiddle[i].resize(n);
        for (int j = 0; j < n; ++j) {
            double phase = hexDFTKernel(m_grid[i].q, m_grid[i].r,
                                         m_grid[j].q, m_grid[j].r, false);
            m_twiddle[i][j].resize(2);
            m_twiddle[i][j][0] = qCos(phase);
            m_twiddle[i][j][1] = qSin(phase);
        }
    }
}

/* ---- 1D FFT helper (radix-2) ---- */

void HexFFT8::fft1D(QVector<double>& re, QVector<double>& im, bool inv) const
{
    int n = re.size();
    // Bit-reverse
    int bits = 0;
    while ((1 << bits) < n) bits++;
    for (int i = 0; i < n; ++i) {
        int rev = 0, val = i;
        for (int b = 0; b < bits; ++b) { rev = (rev << 1) | (val & 1); val >>= 1; }
        if (i < rev) { std::swap(re[i], re[rev]); std::swap(im[i], im[rev]); }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double angle = (inv ? 2.0 : -2.0) * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uRe = re[i + j], uIm = im[i + j];
                double vRe = re[i + j + len / 2] * curRe - im[i + j + len / 2] * curIm;
                double vIm = re[i + j + len / 2] * curIm + im[i + j + len / 2] * curRe;
                re[i + j] = uRe + vRe; im[i + j] = uIm + vIm;
                re[i + j + len / 2] = uRe - vRe; im[i + j + len / 2] = uIm - vIm;
                double newRe = curRe * wRe - curIm * wIm;
                double newIm = curRe * wIm + curIm * wRe;
                curRe = newRe; curIm = newIm;
            }
        }
    }
    if (inv) { for (int i = 0; i < n; ++i) { re[i] /= n; im[i] /= n; } }
}

/* ---- Next power of two ---- */

int HexFFT8::nextPow2(int n) { int p = 1; while (p < n) p <<= 1; return p; }

/* ---- Forward hex FFT ---- */

QVector<QVector<double>> HexFFT8::forward(const QVector<QVector<double>>& input)
{
    QElapsedTimer timer;
    timer.start();

    int gridSize = m_grid.size();
    // Use matrix DFT on hex grid
    int dim = qMax(input.size(), gridSize);
    int padded = nextPow2(dim);

    // Flatten input to 1D complex representation
    QVector<double> re(padded, 0.0), im(padded, 0.0);
    for (int i = 0; i < qMin(input.size(), gridSize); ++i) {
        if (input[i].size() >= 1) re[i] = input[i][0];
        if (input[i].size() >= 2) im[i] = input[i][1];
    }

    // Row-wise 1D FFT on staggered grid rows
    // Group grid points by r-coordinate (rows)
    QMap<int, QVector<int>> rows;
    for (int i = 0; i < gridSize; ++i)
        rows[m_grid[i].r].append(i);

    QVector<double> reOut(padded, 0.0), imOut(padded, 0.0);
    for (auto it = rows.begin(); it != rows.end(); ++it) {
        QVector<int>& indices = it.value();
        int rowLen = nextPow2(indices.size());
        QVector<double> rowRe(rowLen, 0.0), rowIm(rowLen, 0.0);
        for (int j = 0; j < indices.size(); ++j) {
            rowRe[j] = re[indices[j]];
            rowIm[j] = im[indices[j]];
        }
        // Staggered offset: apply phase shift based on row offset
        double staggerPhase = M_PI * it.key() / (2.0 * m_radius);
        for (int j = 0; j < rowLen; ++j) {
            double phase = staggerPhase * j;
            double r = rowRe[j], ii = rowIm[j];
            rowRe[j] = r * qCos(phase) - ii * qSin(phase);
            rowIm[j] = r * qSin(phase) + ii * qCos(phase);
        }
        fft1D(rowRe, rowIm, false);
        // Scatter back
        for (int j = 0; j < indices.size() && j < rowLen; ++j) {
            reOut[indices[j]] = rowRe[j];
            imOut[indices[j]] = rowIm[j];
        }
    }

    // Column-wise FFT (along q-axis)
    QMap<int, QVector<int>> cols;
    for (int i = 0; i < gridSize; ++i)
        cols[m_grid[i].q].append(i);

    for (auto it = cols.begin(); it != cols.end(); ++it) {
        QVector<int>& indices = it.value();
        int colLen = nextPow2(indices.size());
        QVector<double> colRe(colLen, 0.0), colIm(colLen, 0.0);
        for (int j = 0; j < indices.size(); ++j) {
            colRe[j] = reOut[indices[j]];
            colIm[j] = imOut[indices[j]];
        }
        fft1D(colRe, colIm, false);
        for (int j = 0; j < indices.size() && j < colLen; ++j) {
            reOut[indices[j]] = colRe[j];
            imOut[indices[j]] = colIm[j];
        }
    }

    // Pack result
    QVector<QVector<double>> result(gridSize);
    for (int i = 0; i < gridSize; ++i)
        result[i] = {reOut[i], imOut[i]};

    m_stats.gridSize = gridSize;
    m_stats.numSamples = gridSize;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(gridSize, timer.elapsed());
    return result;
}

/* ---- Inverse hex FFT ---- */

QVector<QVector<double>> HexFFT8::inverse(const QVector<QVector<double>>& spectrum)
{
    // Negate imaginary parts for inverse, call forward logic with inv=true
    QVector<QVector<double>> conj = spectrum;
    for (auto& v : conj)
        for (int i = 1; i < v.size(); i += 2) v[i] = -v[i];
    return forward(conj);  // Simplified: real inverse via conjugation symmetry
}

/* ---- Staggered sampling ---- */

QVector<double> HexFFT8::staggeredSample(const QVector<QVector<double>>& rectData) const
{
    QVector<double> result;
    int rows = rectData.size();
    for (int r = 0; r < rows; ++r) {
        int cols = rectData[r].size();
        int offset = r % 2;  // Stagger: odd rows offset by half
        for (int c = offset; c < cols; c += 2) {
            result.append(rectData[r][c]);
        }
    }
    return result;
}

/* ---- Reconstruct rectangular grid ---- */

QVector<QVector<double>> HexFFT8::reconstruct(const QVector<QVector<double>>& hexSpectrum) const
{
    // Simple nearest-neighbor reconstruction from hex to rect
    int size = 2 * m_radius + 1;
    QVector<QVector<double>> rect(size, QVector<double>(size, 0.0));
    for (int i = 0; i < qMin(hexSpectrum.size(), m_grid.size()); ++i) {
        int q = m_grid[i].q + m_radius;
        int r = m_grid[i].r + m_radius;
        if (q >= 0 && q < size && r >= 0 && r < size && hexSpectrum[i].size() >= 1)
            rect[r][q] = hexSpectrum[i][0];
    }
    return rect;
}

/* ---- Grid coordinates ---- */

QVector<HexFFT8::HexPoint> HexFFT8::gridCoordinates() const { return m_grid; }

/* ---- Reset ---- */

void HexFFT8::resetStatistics()
{
    m_twiddle.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
