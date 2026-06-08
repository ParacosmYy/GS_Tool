/**
 * @file WHT5.cpp
 * @brief WHT5 实现
 *
 * 实现沃尔什-哈达玛变换：序列序蝶形、原位Gray码置换、快速计算。
 */

#include "utils/fft213/WHT5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WHT5::WHT5(QObject *parent) : QObject(parent) {}
WHT5::~WHT5() = default;

/* ---- Utility functions ---- */

quint32 WHT5::grayCode(quint32 n)
{
    return n ^ (n >> 1);
}

bool WHT5::isPowerOfTwo(int n)
{
    return n > 0 && (n & (n - 1)) == 0;
}

int WHT5::nextPowerOfTwo(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Gray code permutation ---- */

void WHT5::grayCodePermute(QVector<double>& data)
{
    int N = data.size();
    QVector<bool> visited(N, false);

    for (int i = 0; i < N; ++i) {
        if (visited[i]) continue;
        int j = i;
        double temp = data[i];
        while (!visited[grayCode(static_cast<quint32>(j))]) {
            int next = static_cast<int>(grayCode(static_cast<quint32>(j)));
            if (next == i) {
                data[j] = temp;
                visited[j] = true;
                break;
            }
            data[j] = data[next];
            visited[j] = true;
            j = next;
        }
    }
}

/* ---- In-place butterfly WHT ---- */

void WHT5::butterflyWHT(QVector<double>& data)
{
    int N = data.size();
    if (N <= 1) return;

    // Fast in-place WHT: O(N log N) butterfly
    for (int stride = 1; stride < N; stride <<= 1) {
        for (int i = 0; i < N; i += stride * 2) {
            for (int j = 0; j < stride; ++j) {
                double a = data[i + j];
                double b = data[i + j + stride];
                data[i + j] = a + b;
                data[i + j + stride] = a - b;
            }
        }
    }
}

/* ---- Forward WHT (natural/Hadamard order) ---- */

QVector<double> WHT5::forward(const QVector<double>& input) const
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    if (N == 0) return {};
    if (!isPowerOfTwo(N)) N = nextPowerOfTwo(N);

    QVector<double> data(N, 0.0);
    for (int i = 0; i < qMin(input.size(), N); ++i)
        data[i] = input[i];

    butterflyWHT(data);

    // Scale by 1/sqrt(N) for orthonormal form
    double scale = 1.0 / qSqrt(static_cast<double>(N));
    for (int i = 0; i < N; ++i)
        data[i] *= scale;

    auto self = const_cast<WHT5*>(this);
    self->m_stats.totalTransforms++;
    self->m_stats.lastSize = input.size();
    self->m_stats.order = N;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    self->emit transformCompleted(input.size(), N, timer.elapsed());

    return data;
}

/* ---- Forward sequency-ordered WHT ---- */

QVector<double> WHT5::forwardSequency(const QVector<double>& input) const
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    if (N == 0) return {};
    if (!isPowerOfTwo(N)) N = nextPowerOfTwo(N);

    QVector<double> data(N, 0.0);
    for (int i = 0; i < qMin(input.size(), N); ++i)
        data[i] = input[i];

    // Step 1: Compute natural-order WHT
    butterflyWHT(data);

    // Step 2: Reorder to sequency using Gray code bit-reversal
    // Sequency order = Gray code order of natural order
    QVector<double> sequency(N, 0.0);
    int logN = 0;
    int tmp = N;
    while (tmp > 1) { tmp >>= 1; logN++; }

    for (int i = 0; i < N; ++i) {
        // Bit-reverse i
        int rev = 0;
        for (int b = 0; b < logN; ++b) {
            if (i & (1 << b)) rev |= (1 << (logN - 1 - b));
        }
        // Apply Gray code to get sequency index
        int seqIdx = static_cast<int>(grayCode(static_cast<quint32>(rev)));
        if (seqIdx < N) sequency[seqIdx] = data[i];
    }

    // Scale
    double scale = 1.0 / qSqrt(static_cast<double>(N));
    for (int i = 0; i < N; ++i)
        sequency[i] *= scale;

    auto self = const_cast<WHT5*>(this);
    self->m_stats.totalTransforms++;
    self->m_stats.lastSize = input.size();
    self->m_stats.order = N;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    self->emit transformCompleted(input.size(), N, timer.elapsed());

    return sequency;
}

/* ---- Inverse WHT ---- */

QVector<double> WHT5::inverse(const QVector<double>& spectrum) const
{
    // WHT is its own inverse (up to scaling)
    // Forward uses 1/sqrt(N), so inverse = forward * sqrt(N) / N = forward
    return forward(spectrum);
}

/* ---- Inverse sequency-ordered WHT ---- */

QVector<double> WHT5::inverseSequency(const QVector<double>& spectrum) const
{
    return forwardSequency(spectrum);
}

/* ---- Naive WHT ---- */

QVector<double> WHT5::naiveWHT(const QVector<double>& input)
{
    int N = input.size();
    QVector<double> result(N, 0.0);

    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n) {
            // Hadamard entry: H[k][n] = (-1)^{popcount(k AND n)}
            int bitwise = k & n;
            int popcount = 0;
            while (bitwise) { popcount += bitwise & 1; bitwise >>= 1; }
            sum += input[n] * ((popcount % 2 == 0) ? 1.0 : -1.0);
        }
        result[k] = sum / qSqrt(static_cast<double>(N));
    }
    return result;
}

/* ---- Reset ---- */

void WHT5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
