/**
 * @file DCT8.cpp
 * @brief DCT8 实现
 *
 * 实现离散余弦变换：II型快速计算、FFT重索引与偶扩展。
 */

#include "utils/fft239/DCT8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DCT8::DCT8(QObject *parent) : QObject(parent) {}
DCT8::~DCT8() = default;

/* ---- Configure ---- */

bool DCT8::configure(int N)
{
    if (N < 2) return false;
    m_N = N;
    m_stats.transformSize = N;
    computeTables();
    return true;
}

/* ---- Compute cosine table and scaling ---- */

void DCT8::computeTables()
{
    m_cosTable.resize(m_N * m_N);
    m_scale.resize(m_N);

    for (int k = 0; k < m_N; ++k) {
        // Orthonormal scaling: sqrt(2/N) for k>0, sqrt(1/N) for k=0
        m_scale[k] = (k == 0) ? qSqrt(1.0 / m_N) : qSqrt(2.0 / m_N);
        for (int n = 0; n < m_N; ++n)
            m_cosTable[k * m_N + n] = qCos(M_PI * (2 * n + 1) * k / (2.0 * m_N));
    }
}

/* ---- Bit reversal ---- */

void DCT8::bitReverse(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    int log2n = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; log2n++; }
    for (int i = 0; i < n; ++i) {
        int rev = 0, val = i;
        for (int b = 0; b < log2n; ++b) { rev = (rev << 1) | (val & 1); val >>= 1; }
        if (rev > i) {
            std::swap(re[i], re[rev]);
            std::swap(im[i], im[rev]);
        }
    }
}

/* ---- Radix-2 FFT butterfly ---- */

void DCT8::fftButterfly(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    bitReverse(re, im);
    for (int len = 2; len <= n; len *= 2) {
        int half = len / 2;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < half; ++j) {
                double angle = -2.0 * M_PI * j / len;
                double cosA = qCos(angle), sinA = qSin(angle);
                double tRe = cosA * re[i + j + half] - sinA * im[i + j + half];
                double tIm = sinA * re[i + j + half] + cosA * im[i + j + half];
                re[i + j + half] = re[i + j] - tRe;
                im[i + j + half] = im[i + j] - tIm;
                re[i + j] += tRe;
                im[i + j] += tIm;
            }
        }
    }
}

/* ---- DCT-II via even-extension reindexing ---- */

void DCT8::dctII(QVector<double>& data) const
{
    int N = data.size();
    // Even-extension reindexing: pack N-point DCT into 2N-point FFT
    // Reorder: data[i] -> y[2i] = x[i], y[2N-1-2i] = x[N-1-i]
    int N2 = 2 * N;
    QVector<double> re(N2, 0.0), im(N2, 0.0);

    // Even extension reindexing
    for (int n = 0; n < N; ++n) {
        re[n] = data[n];
        re[N2 - 1 - n] = data[n];
    }

    fftButterfly(re, im);

    // Extract DCT coefficients from FFT result
    // X[k] = Re{FFT[k]} * 2 * cos_factor
    for (int k = 0; k < N; ++k) {
        double phase = M_PI * k / (2.0 * N);
        double cosP = qCos(phase);
        data[k] = 0.5 * (re[k] * cosP + im[k] * qSin(phase)) * m_scale[k];
    }
}

/* ---- DCT-III (inverse) via reindexing ---- */

void DCT8::dctIII(QVector<double>& data) const
{
    int N = data.size();
    // Inverse DCT: x[n] = sum_k w[k]*X[k]*cos(pi*k*(2n+1)/(2N))
    for (int n = 0; n < N; ++n) {
        double sum = data[0] / qSqrt(static_cast<double>(N));
        for (int k = 1; k < N; ++k) {
            sum += data[k] * qSqrt(2.0 / N) *
                   qCos(M_PI * k * (2 * n + 1) / (2.0 * N));
        }
        data[n] = sum;
    }
}

/* ---- Forward DCT-II ---- */

QVector<double> DCT8::forward(const QVector<double>& input)
{
    if (input.size() != m_N) return {};
    QVector<double> data = input;
    forwardInPlace(data);
    return data;
}

/* ---- Inverse DCT-III ---- */

QVector<double> DCT8::inverse(const QVector<double>& spectrum)
{
    if (spectrum.size() != m_N) return {};
    QVector<double> data = spectrum;
    inverseInPlace(data);
    return data;
}

/* ---- Forward in-place ---- */

void DCT8::forwardInPlace(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    dctII(data);

    m_stats.numForward++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit forwardCompleted(m_N, timer.elapsed());
}

/* ---- Inverse in-place ---- */

void DCT8::inverseInPlace(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    dctIII(data);

    m_stats.numInverse++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit inverseCompleted(m_N, timer.elapsed());
}

/* ---- Reset ---- */

void DCT8::resetStatistics()
{
    m_cosTable.clear(); m_scale.clear();
    m_N = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
