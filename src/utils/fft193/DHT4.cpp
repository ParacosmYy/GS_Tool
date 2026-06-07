/**
 * @file DHT4.cpp
 * @brief DHT4 实现
 *
 * 实现离散哈特利变换：cas(x)基函数、自逆性质、快速递归FHT。
 */

#include "utils/fft193/DHT4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DHT4::DHT4(QObject *parent) : QObject(parent) {}
DHT4::~DHT4() = default;

/* ---- cas(x) = cos(x) + sin(x) ---- */

double DHT4::cas(double x)
{
    return qCos(x) + qSin(x);
}

/* ---- Next power of 2 ---- */

int DHT4::nextPow2(int n)
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/* ---- Bit-reverse permutation ---- */

void DHT4::bitReverse(QVector<double>& data, int n)
{
    int bits = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; bits++; }

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        int val = i;
        for (int b = 0; b < bits; ++b) {
            rev = (rev << 1) | (val & 1);
            val >>= 1;
        }
        if (rev > i)
            std::swap(data[i], data[rev]);
    }
}

/* ---- Fast Hartley Transform (radix-2 DIT) ---- */

void DHT4::fht(QVector<double>& data, int n)
{
    bitReverse(data, n);

    for (int stride = 2; stride <= n; stride *= 2) {
        int half = stride / 2;
        for (int base = 0; base < n; base += stride) {
            for (int k = 0; k < half; ++k) {
                double angle = 2.0 * M_PI * k / stride;
                double c = qCos(angle);
                double s = qSin(angle);

                int idx0 = base + k;
                int idx1 = base + k + half;
                double a = data[idx0];
                double b = data[idx1];

                // Hartley butterfly: combine using cas
                data[idx0] = a + b * c + b * s;
                data[idx1] = a - b * c + b * s;
            }
        }
    }
}

/* ---- Forward DHT ---- */

QVector<double> DHT4::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return {};

    // Pad to power of 2 if needed
    int N = nextPow2(n);
    QVector<double> data(N, 0.0);
    for (int i = 0; i < n; ++i)
        data[i] = input[i];

    fht(data, N);

    m_stats.totalTransforms++;
    m_stats.lastSize = n;
    m_stats.radixUsed = 2;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, timer.elapsed());
    return data;
}

/* ---- In-place DHT ---- */

void DHT4::transformInPlace(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return;

    int N = nextPow2(n);
    data.resize(N);

    fht(data, N);

    m_stats.totalTransforms++;
    m_stats.lastSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, timer.elapsed());
}

/* ---- Direct O(N^2) DHT for verification ---- */

QVector<double> DHT4::directDHT(const QVector<double>& input) const
{
    int N = input.size();
    QVector<double> output(N, 0.0);

    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n)
            sum += input[n] * cas(2.0 * M_PI * k * n / N);
        output[k] = sum;
    }
    return output;
}

/* ---- Extract real part ---- */

QVector<double> DHT4::extractReal(const QVector<double>& hartley) const
{
    int N = hartley.size();
    QVector<double> re(N / 2 + 1, 0.0);

    for (int k = 0; k <= N / 2; ++k) {
        double hk = hartley[k];
        double hNk = (k == 0 || k == N / 2) ? hartley[k] : hartley[N - k];
        re[k] = (hk + hNk) * 0.5;
    }
    return re;
}

/* ---- Extract imaginary part ---- */

QVector<double> DHT4::extractImag(const QVector<double>& hartley) const
{
    int N = hartley.size();
    QVector<double> im(N / 2 + 1, 0.0);

    for (int k = 0; k <= N / 2; ++k) {
        double hk = hartley[k];
        double hNk = (k == 0 || k == N / 2) ? hartley[k] : hartley[N - k];
        im[k] = (hk - hNk) * 0.5;
    }
    return im;
}

/* ---- Padded transform ---- */

QVector<double> DHT4::transformPadded(const QVector<double>& input)
{
    return transform(input);
}

/* ---- Reset ---- */

void DHT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
