/**
 * @file DST8.cpp
 * @brief DST8 实现
 *
 * 实现离散正弦变换：I型快速计算与FFT奇对称扩展。
 */

#include "utils/fft240/DST8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DST8::DST8(QObject *parent) : QObject(parent) {}
DST8::~DST8() = default;

/* ---- Helper: power of 2 check ---- */

bool DST8::isPowerOf2(int n) { return n > 0 && (n & (n - 1)) == 0; }

/* ---- Configure ---- */

bool DST8::configure(int N)
{
    if (N < 2 || !isPowerOf2(N + 1)) return false;
    m_N = N;
    m_stats.transformSize = N;
    return true;
}

/* ---- Bit reversal ---- */

void DST8::bitReverse(QVector<double>& re, QVector<double>& im) const
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

void DST8::fftButterfly(QVector<double>& re, QVector<double>& im) const
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

/* ---- DST-I via FFT odd-symmetry extension ---- */

void DST8::dstI(QVector<double>& data) const
{
    int N = data.size();
    // DST-I: X[k] = sum_{n=0}^{N-1} x[n] * sin(pi*(n+1)*(k+1)/(N+1))
    // Via odd-symmetry extension: embed N-point DST into 2(N+1)-point FFT
    int M = 2 * (N + 1);

    QVector<double> re(M, 0.0), im(M, 0.0);

    // Odd-symmetric extension around boundaries
    for (int n = 0; n < N; ++n) {
        re[n + 1] = data[n];
        re[M - 1 - n] = -data[n];  // odd symmetry
    }

    fftButterfly(re, im);

    // Extract DST coefficients: X[k] = -Im{FFT[k+1]} for k=0..N-1
    for (int k = 0; k < N; ++k)
        data[k] = -im[k + 1];
}

/* ---- Forward DST-I ---- */

QVector<double> DST8::forward(const QVector<double>& input)
{
    if (input.size() != m_N) return {};
    QVector<double> data = input;
    forwardInPlace(data);
    return data;
}

/* ---- Inverse DST-I ---- */

QVector<double> DST8::inverse(const QVector<double>& spectrum)
{
    if (spectrum.size() != m_N) return {};
    // DST-I is self-inverse up to scaling: x = (2/(N+1)) * DST(X)
    QVector<double> data = spectrum;
    inverseInPlace(data);
    return data;
}

/* ---- Forward in-place ---- */

void DST8::forwardInPlace(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    dstI(data);

    m_stats.numForward++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit forwardCompleted(m_N, timer.elapsed());
}

/* ---- Inverse in-place ---- */

void DST8::inverseInPlace(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    // DST-I is its own inverse with 2/(N+1) scaling
    dstI(data);
    double scale = 2.0 / (m_N + 1);
    for (int i = 0; i < data.size(); ++i)
        data[i] *= scale;

    m_stats.numInverse++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit inverseCompleted(m_N, timer.elapsed());
}

/* ---- Reset ---- */

void DST8::resetStatistics()
{
    m_N = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
