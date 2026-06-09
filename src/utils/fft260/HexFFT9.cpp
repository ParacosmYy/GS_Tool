/**
 * @file HexFFT9.cpp
 * @brief HexFFT9 实现
 *
 * 实现六角FFT：轴向坐标系与六重旋转对称六角格变换。
 */

#include "utils/fft260/HexFFT9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

HexFFT9::HexFFT9(QObject *parent)
    : QObject(parent) { buildGrid(); }
HexFFT9::~HexFFT9() = default;

/* ---- Configuration ---- */

void HexFFT9::setGridRadius(int radius)
{
    m_radius = qMax(1, radius);
    buildGrid();
}

/* ---- Build hex grid ---- */

void HexFFT9::buildGrid()
{
    m_coords.clear();
    for (int q = -m_radius; q <= m_radius; ++q) {
        int r1 = qMax(-m_radius, -q - m_radius);
        int r2 = qMin(m_radius, -q + m_radius);
        for (int r = r1; r <= r2; ++r) {
            m_coords.append({q, r});
        }
    }
    m_gridSize = m_coords.size();
}

/* ---- Hex distance ---- */

int HexFFT9::hexDistance(const HexCoord& a, const HexCoord& b)
{
    int x1 = a.q, z1 = a.r, y1 = -x1 - z1;
    int x2 = b.q, z2 = b.r, y2 = -x2 - z2;
    return (qAbs(x1 - x2) + qAbs(y1 - y2) + qAbs(z1 - z2)) / 2;
}

/* ---- Axial to cube ---- */

void HexFFT9::axialToCube(int q, int r, int& x, int& y, int& z) const
{
    x = q;
    z = r;
    y = -x - z;
}

/* ---- Ring indices ---- */

QVector<int> HexFFT9::ringIndices(int distance) const
{
    QVector<int> result;
    for (int i = 0; i < m_coords.size(); ++i) {
        if (hexDistance(m_coords[i], {0, 0}) == distance)
            result.append(i);
    }
    return result;
}

/* ---- Bit-reversal ---- */

void HexFFT9::bitReverse(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    int logN = 0;
    for (int tmp = n; tmp > 1; tmp >>= 1) logN++;
    for (int i = 0; i < n; ++i) {
        int rev = 0;
        for (int b = 0; b < logN; ++b)
            if (i & (1 << b)) rev |= (1 << (logN - 1 - b));
        if (i < rev) {
            std::swap(real[i], real[rev]);
            std::swap(imag[i], imag[rev]);
        }
    }
}

/* ---- 1D FFT ---- */

void HexFFT9::fft1D(QVector<double>& real, QVector<double>& imag, bool inverse) const
{
    int n = real.size();
    if (n <= 1) return;
    // Pad to power of 2
    int p = 1;
    while (p < n) p <<= 1;
    if (p != n) {
        real.resize(p, 0.0);
        imag.resize(p, 0.0);
        n = p;
    }

    bitReverse(real, imag);
    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len <<= 1) {
        double angle = sign * 2.0 * M_PI / len;
        double wReal = qCos(angle);
        double wImag = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uReal = real[i + j];
                double uImag = imag[i + j];
                double vReal = curReal * real[i + j + len / 2] - curImag * imag[i + j + len / 2];
                double vImag = curReal * imag[i + j + len / 2] + curImag * real[i + j + len / 2];
                real[i + j] = uReal + vReal;
                imag[i + j] = uImag + vImag;
                real[i + j + len / 2] = uReal - vReal;
                imag[i + j + len / 2] = uImag - vImag;
                double newReal = curReal * wReal - curImag * wImag;
                double newImag = curReal * wImag + curImag * wReal;
                curReal = newReal;
                curImag = newImag;
            }
        }
    }
    if (inverse) {
        for (int i = 0; i < n; ++i) { real[i] /= n; imag[i] /= n; }
    }
}

/* ---- Six-fold symmetry group ---- */

QVector<int> HexFFT9::symmetryGroup(int index) const
{
    QVector<int> group;
    if (index < 0 || index >= m_coords.size()) return group;
    int q = m_coords[index].q;
    int r = m_coords[index].r;
    // Six-fold rotations: (q,r) -> (-r, q+r) -> (-q-r, q) -> (-q,-r) -> (r,-q-r) -> (q+r,-q)
    QVector<HexCoord> rotations = {
        {q, r}, {-r, q + r}, {-q - r, q}, {-q, -r}, {r, -q - r}, {q + r, -q}
    };
    for (const auto& coord : rotations) {
        for (int i = 0; i < m_coords.size(); ++i) {
            if (m_coords[i].q == coord.q && m_coords[i].r == coord.r) {
                if (!group.contains(i)) group.append(i);
                break;
            }
        }
    }
    return group;
}

/* ---- Symmetry optimization ---- */

void HexFFT9::applySymmetryOptimization(QVector<double>& data, bool inverse)
{
    // Compute FFT for unique sectors, replicate via symmetry
    Q_UNUSED(inverse);
    // Group grid points by symmetry equivalence class
    QVector<bool> processed(m_gridSize, false);
    for (int i = 0; i < m_gridSize; ++i) {
        if (processed[i]) continue;
        QVector<int> group = symmetryGroup(i);
        if (group.size() > 1) {
            // Average across symmetry group
            double sum = 0.0;
            for (int idx : group) sum += data[idx];
            double avg = sum / group.size();
            for (int idx : group) data[idx] = avg;
        }
        for (int idx : group)
            if (idx < processed.size()) processed[idx] = true;
    }
}

/* ---- Forward transform ---- */

QVector<double> HexFFT9::forward(const QVector<double>& inputData)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result = inputData;
    if (result.size() != m_gridSize) result.resize(m_gridSize, 0.0);

    // Decompose by rings and apply 1D FFT per ring
    for (int d = 0; d <= m_radius; ++d) {
        QVector<int> ring = ringIndices(d);
        if (ring.isEmpty()) continue;
        QVector<double> real, imag;
        for (int idx : ring) {
            real.append(result[idx]);
            imag.append(0.0);
        }
        fft1D(real, imag, false);
        for (int i = 0; i < ring.size(); ++i)
            result[ring[i]] = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
    }

    // Exploit six-fold rotational symmetry
    applySymmetryOptimization(result, false);

    double elapsed = timer.elapsed();
    m_stats.gridSize = m_gridSize;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformCompleted(m_gridSize, false, elapsed);
    return result;
}

/* ---- Inverse transform ---- */

QVector<double> HexFFT9::inverse(const QVector<double>& spectrumData)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result = spectrumData;
    if (result.size() != m_gridSize) result.resize(m_gridSize, 0.0);

    applySymmetryOptimization(result, true);

    for (int d = 0; d <= m_radius; ++d) {
        QVector<int> ring = ringIndices(d);
        if (ring.isEmpty()) continue;
        QVector<double> real, imag;
        for (int idx : ring) {
            real.append(result[idx]);
            imag.append(0.0);
        }
        fft1D(real, imag, true);
        for (int i = 0; i < ring.size(); ++i)
            result[ring[i]] = real[i];
    }

    double elapsed = timer.elapsed();
    m_stats.numInverseTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformCompleted(m_gridSize, true, elapsed);
    return result;
}

/* ---- Accessors ---- */

QVector<HexFFT9::HexCoord> HexFFT9::gridCoordinates() const { return m_coords; }

/* ---- Reset ---- */

void HexFFT9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
