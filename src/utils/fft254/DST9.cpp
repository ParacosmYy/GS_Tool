/**
 * @file DST9.cpp
 * @brief DST9 实现
 *
 * 实现离散正弦变换：Type-III快速预/后旋转FFT分解与纯正弦输出优化。
 */

#include "utils/fft254/DST9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DST9::DST9(QObject *parent) : QObject(parent) {}
DST9::~DST9() = default;

/* ---- Check power of 2 ---- */

bool DST9::isPowerOf2(int n) { return n > 0 && (n & (n - 1)) == 0; }

/* ---- Precompute sine table and twiddle factors ---- */

void DST9::precompute()
{
    int n = m_size;
    m_sinTable.resize(n * n);
    m_twiddleRe.resize(n);
    m_twiddleIm.resize(n);

    // Sine table: sin(pi * (k+1) * (j+1) / (N+1))
    for (int k = 0; k < n; ++k)
        for (int j = 0; j < n; ++j)
            m_sinTable[k * n + j] = qSin(M_PI * (k + 1) * (j + 1) / (n + 1.0));

    // Pre/post-twiddle factors for DST-III FFT decomposition
    for (int k = 0; k < n; ++k) {
        double angle = M_PI * (k + 0.5) / (2.0 * n);
        m_twiddleRe[k] = qCos(angle);
        m_twiddleIm[k] = qSin(angle);
    }
}

/* ---- Simple radix-2 FFT (in-place) ---- */

void DST9::fft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    if (n <= 1) return;

    // Bit-reversal permutation
    int bits = 0;
    for (int tmp = n; tmp > 1; tmp >>= 1) bits++;
    for (int i = 0; i < n; ++i) {
        int rev = 0;
        for (int b = 0; b < bits; ++b)
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        if (i < rev) {
            std::swap(re[i], re[rev]);
            std::swap(im[i], im[rev]);
        }
    }

    // Butterfly stages
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wRe = qCos(ang);
        double wIm = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int even = i + j;
                int odd = i + j + len / 2;
                double tRe = curRe * re[odd] - curIm * im[odd];
                double tIm = curRe * im[odd] + curIm * re[odd];
                re[odd] = re[even] - tRe;
                im[odd] = im[even] - tIm;
                re[even] += tRe;
                im[even] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }
}

/* ---- DST-I direct computation ---- */

QVector<double> DST9::dstI(const QVector<double>& x) const
{
    int n = m_size;
    QVector<double> y(n, 0.0);
    // DST-I: Y[k] = sum_{j=0}^{N-1} x[j]*sin(pi*(j+1)*(k+1)/(N+1))
    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j)
            sum += x[j] * m_sinTable[k * n + j];
        y[k] = sum;
    }
    return y;
}

/* ---- DST-II direct computation ---- */

QVector<double> DST9::dstII(const QVector<double>& x) const
{
    int n = m_size;
    QVector<double> y(n, 0.0);
    // DST-II: Y[k] = sum_{j=0}^{N-1} x[j]*sin(pi*(j+0.5)*(k+1)/N)
    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j)
            sum += x[j] * qSin(M_PI * (j + 0.5) * (k + 1) / n);
        y[k] = sum;
    }
    return y;
}

/* ---- DST-III via pre/post-twiddle FFT decomposition ---- */

QVector<double> DST9::dstIII(const QVector<double>& x) const
{
    int n = m_size;

    // Pre-twiddle: multiply input by twiddle factors
    QVector<double> re(n, 0.0);
    QVector<double> im(n, 0.0);
    for (int j = 0; j < n; ++j) {
        re[j] = x[j] * m_twiddleRe[j];
        im[j] = x[j] * m_twiddleIm[j];
        m_stats.numTwiddleOps++;
    }

    // Compute FFT of twiddled sequence
    fft(re, im);

    // Post-twiddle: extract sine-only output
    QVector<double> y(n, 0.0);
    for (int k = 0; k < n; ++k) {
        // Combine FFT output to isolate sine component
        y[k] = 2.0 * (re[k] * m_twiddleIm[k] + im[k] * m_twiddleRe[k]);
        m_stats.numTwiddleOps++;
    }

    return y;
}

/* ---- Prepare ---- */

bool DST9::prepare(int n, Type type)
{
    if (n < 2) return false;
    m_size = n;
    m_type = type;
    precompute();
    m_stats.transformSize = n;
    return true;
}

/* ---- Forward DST ---- */

QVector<double> DST9::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_size;
    if (n <= 0) return {};

    QVector<double> x(n, 0.0);
    for (int i = 0; i < qMin(input.size(), n); ++i)
        x[i] = input[i];

    QVector<double> result;
    switch (m_type) {
    case TypeI:
        result = dstI(x);
        break;
    case TypeII:
        result = dstII(x);
        break;
    case TypeIII:
        result = dstIII(x);
        m_stats.numTypeIII++;
        break;
    }

    // Normalization factor
    double norm = qSqrt(2.0 / (n + 1));
    for (auto& v : result) v *= norm;

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(n, static_cast<int>(m_type), timer.elapsed());
    return result;
}

/* ---- Inverse DST ---- */

QVector<double> DST9::inverse(const QVector<double>& coefficients)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_size;
    if (n <= 0) return {};

    // Undo normalization
    double norm = qSqrt(2.0 / (n + 1));
    QVector<double> x(n, 0.0);
    for (int k = 0; k < qMin(coefficients.size(), n); ++k)
        x[k] = coefficients[k] / qMax(norm, 1e-15);

    // DST-I and DST-II are their own inverse (up to scaling)
    QVector<double> result;
    switch (m_type) {
    case TypeI:
        result = dstI(x);
        for (auto& v : result) v *= 2.0 / (n + 1);
        break;
    case TypeII:
        // Inverse of DST-II is DST-III (scaled)
        result = dstIII(x);
        for (auto& v : result) v *= 2.0 / n;
        break;
    case TypeIII:
        // Inverse of DST-III is DST-II (scaled)
        result = dstII(x);
        for (auto& v : result) v *= 2.0 / n;
        m_stats.numTypeIII++;
        break;
    }

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(n, static_cast<int>(m_type), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void DST9::resetStatistics()
{
    m_sinTable.clear();
    m_twiddleRe.clear();
    m_twiddleIm.clear();
    m_size = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
