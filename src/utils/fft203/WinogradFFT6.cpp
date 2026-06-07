/**
 * @file WinogradFFT6.cpp
 * @brief WinogradFFT6 实现
 *
 * 实现Winograd FFT：小DFT模块张量积分解、二维矩阵转置。
 */

#include "utils/fft203/WinogradFFT6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WinogradFFT6::WinogradFFT6(QObject *parent) : QObject(parent) { precompute(m_size); }
WinogradFFT6::~WinogradFFT6() = default;

/* ---- Configuration ---- */

void WinogradFFT6::setTransformSize(int n) { m_size = qMax(2, n); precompute(m_size); }

/* ---- Factorize ---- */

QVector<int> WinogradFFT6::factorize(int n) const
{
    QVector<int> factors;
    static const int primes[] = {2, 3, 5, 7};
    for (int p : primes) {
        while (n % p == 0) { factors.append(p); n /= p; }
    }
    if (n > 1) factors.append(n);
    return factors;
}

/* ---- Small Winograd DFT ---- */

QVector<double> WinogradFFT6::smallDFT(const QVector<double>& real,
                                          const QVector<double>& imag, int n)
{
    // Output: interleaved real/imag pairs, length 2*n
    QVector<double> result(2 * n, 0.0);
    for (int k = 0; k < n; ++k) {
        double re = 0.0, im = 0.0;
        for (int j = 0; j < n; ++j) {
            double angle = -2.0 * M_PI * k * j / n;
            re += real[j] * qCos(angle) - imag[j] * qSin(angle);
            im += real[j] * qSin(angle) + imag[j] * qCos(angle);
        }
        result[2 * k] = re;
        result[2 * k + 1] = im;
    }
    return result;
}

/* ---- Tensor product ---- */

void WinogradFFT6::tensorProduct(const QVector<double>& matA, int rowsA, int colsA,
                                    const QVector<double>& matB, int rowsB, int colsB,
                                    QVector<double>& result) const
{
    int rows = rowsA * rowsB;
    int cols = colsA * colsB;
    result.resize(rows * cols);
    for (int i = 0; i < rowsA; ++i)
        for (int j = 0; j < colsA; ++j)
            for (int k = 0; k < rowsB; ++k)
                for (int l = 0; l < colsB; ++l)
                    result[(i * rowsB + k) * cols + (j * colsB + l)] =
                        matA[i * colsA + j] * matB[k * colsB + l];
}

/* ---- Matrix transpose ---- */

void WinogradFFT6::transpose(QVector<double>& matrix, int rows, int cols)
{
    QVector<double> tmp(rows * cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            tmp[j * rows + i] = matrix[i * cols + j];
    matrix = tmp;
}

/* ---- Precompute twiddle factors ---- */

void WinogradFFT6::precompute(int n)
{
    m_twiddleReal.resize(n);
    m_twiddleImag.resize(n);
    for (int k = 0; k < n; ++k) {
        double angle = -2.0 * M_PI * k / n;
        m_twiddleReal[k] = qCos(angle);
        m_twiddleImag[k] = qSin(angle);
    }
}

/* ---- Apply small DFT ---- */

void WinogradFFT6::applySmallDFT(QVector<double>& real, QVector<double>& imag,
                                    int offset, int stride, int n) const
{
    QVector<double> r(n), i(n);
    for (int k = 0; k < n; ++k) { r[k] = real[offset + k * stride]; i[k] = imag[offset + k * stride]; }
    QVector<double> res = smallDFT(r, i, n);
    for (int k = 0; k < n; ++k) {
        real[offset + k * stride] = res[2 * k];
        imag[offset + k * stride] = res[2 * k + 1];
    }
}

/* ---- Combine step ---- */

void WinogradFFT6::combine(QVector<double>& real, QVector<double>& imag, int n1, int n2) const
{
    int n = n1 * n2;
    for (int k1 = 0; k1 < n1; ++k1) {
        for (int k2 = 0; k2 < n2; ++k2) {
            int idx = k1 * n2 + k2;
            int twIdx = k1 * k2;
            double twR = m_twiddleReal[twIdx % m_twiddleReal.size()];
            double twI = m_twiddleImag[twIdx % m_twiddleImag.size()];
            double r = real[idx], im = imag[idx];
            real[idx] = r * twR - im * twI;
            imag[idx] = r * twI + im * twR;
        }
    }
}

/* ---- Forward FFT ---- */

QVector<double> WinogradFFT6::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n != m_size) { m_size = n; precompute(n); }

    QVector<double> real = input;
    QVector<double> imag(n, 0.0);

    QVector<int> factors = factorize(n);
    int stride = n;

    for (int f : factors) {
        int numTransforms = n / f;
        for (int t = 0; t < numTransforms; ++t)
            applySmallDFT(real, imag, t * f, 1, f);
        stride /= f;
    }

    // Apply twiddle and combine
    int acc = 1;
    for (int i = 0; i < factors.size(); ++i) {
        int n1 = acc;
        int n2 = n / acc;
        combine(real, imag, n1, n2);
        acc *= factors[i];
    }

    // Interleave output
    QVector<double> result(2 * n);
    for (int k = 0; k < n; ++k) {
        result[2 * k] = real[k];
        result[2 * k + 1] = imag[k];
    }

    m_stats.totalTransforms++;
    m_stats.transformSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    emit transformCompleted(n, timer.elapsed());
    return result;
}

/* ---- Inverse FFT ---- */

QVector<double> WinogradFFT6::inverse(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int n = spectrum.size() / 2;
    if (n != m_size) { m_size = n; precompute(n); }

    QVector<double> real(n), imag(n);
    for (int k = 0; k < n; ++k) { real[k] = spectrum[2 * k]; imag[k] = spectrum[2 * k + 1]; }

    // Conjugate
    for (auto& v : imag) v = -v;

    QVector<int> factors = factorize(n);
    for (int f : factors) {
        int numTransforms = n / f;
        for (int t = 0; t < numTransforms; ++t)
            applySmallDFT(real, imag, t * f, 1, f);
    }

    // Scale and conjugate back
    QVector<double> result(2 * n);
    for (int k = 0; k < n; ++k) {
        result[2 * k] = real[k] / n;
        result[2 * k + 1] = -imag[k] / n;
    }

    m_stats.totalTransforms++;
    m_stats.transformSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    emit transformCompleted(n, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void WinogradFFT6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
