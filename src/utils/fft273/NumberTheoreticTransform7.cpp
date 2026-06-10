/**
 * @file NumberTheoreticTransform7.cpp
 * @brief NumberTheoreticTransform7 实现
 *
 * 实现数论变换：Fermat素数模算术与Cooley-Tukey蝶形精确整数卷积。
 */

#include "utils/fft273/NumberTheoreticTransform7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

NumberTheoreticTransform7::NumberTheoreticTransform7(QObject *parent)
    : QObject(parent)
{
    precomputeTwiddles();
}

NumberTheoreticTransform7::~NumberTheoreticTransform7() = default;

/* ---- Configuration ---- */

void NumberTheoreticTransform7::setSize(int n)
{
    // Round up to next power of 2
    int p = 1;
    while (p < n) p <<= 1;
    m_size = qBound(2, p, 1 << 23);
    precomputeTwiddles();
}

void NumberTheoreticTransform7::setModulus(qint64 mod)
{
    m_mod = qMax(2LL, mod);
    precomputeTwiddles();
}

void NumberTheoreticTransform7::setPrimitiveRoot(qint64 g)
{
    m_primRoot = g;
    precomputeTwiddles();
}

/* ---- Modular exponentiation (binary exponentiation) ---- */

qint64 NumberTheoreticTransform7::modPow(qint64 base, qint64 exp, qint64 mod) const
{
    qint64 result = 1;
    base %= mod;
    if (base < 0) base += mod;
    while (exp > 0) {
        if (exp & 1) {
            // Use __int128-safe multiplication via splitting
            result = static_cast<qint64>(
                static_cast<__int128>(result) * base % mod);
        }
        exp >>= 1;
        base = static_cast<qint64>(
            static_cast<__int128>(base) * base % mod);
    }
    return result;
}

/* ---- Modular inverse via Fermat's little theorem ---- */

qint64 NumberTheoreticTransform7::modInverse(qint64 a, qint64 mod) const
{
    return modPow(a, mod - 2, mod);
}

/* ---- Bit-reversal permutation ---- */

int NumberTheoreticTransform7::bitReverse(int x, int logN) const
{
    int result = 0;
    for (int i = 0; i < logN; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

/* ---- Precompute twiddle factors ---- */

void NumberTheoreticTransform7::precomputeTwiddles()
{
    qint64 root = modPow(m_primRoot, (m_mod - 1) / m_size, m_mod);
    qint64 invRoot = modInverse(root, m_mod);
    m_nInverse = modInverse(m_size, m_mod);

    m_twiddles.resize(m_size);
    m_invTwiddles.resize(m_size);

    m_twiddles[0] = 1;
    m_invTwiddles[0] = 1;
    for (int i = 1; i < m_size; ++i) {
        m_twiddles[i] = static_cast<qint64>(
            static_cast<__int128>(m_twiddles[i - 1]) * root % m_mod);
        m_invTwiddles[i] = static_cast<qint64>(
            static_cast<__int128>(m_invTwiddles[i - 1]) * invRoot % m_mod);
    }
}

/* ---- Cooley-Tukey butterfly NTT core (in-place) ---- */

void NumberTheoreticTransform7::butterfly(QVector<qint64>& data, bool inverse)
{
    int n = data.size();
    int logN = 0;
    {
        int tmp = n;
        while (tmp > 1) { tmp >>= 1; logN++; }
    }

    // Bit-reversal permutation
    for (int i = 0; i < n; ++i) {
        int j = bitReverse(i, logN);
        if (i < j) std::swap(data[i], data[j]);
    }

    const QVector<qint64>& tw = inverse ? m_invTwiddles : m_twiddles;

    // Cooley-Tukey iterative butterfly
    for (int len = 2; len <= n; len <<= 1) {
        int half = len >> 1;
        int step = n / len;

        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < half; ++j) {
                int twIdx = j * step;
                qint64 w = tw[twIdx];

                qint64 u = data[i + j];
                qint64 v = static_cast<qint64>(
                    static_cast<__int128>(data[i + j + half]) * w % m_mod);

                data[i + j] = (u + v) % m_mod;
                data[i + j + half] = (u - v + m_mod) % m_mod;
            }
        }
    }

    // For inverse: multiply by n^{-1}
    if (inverse) {
        for (int i = 0; i < n; ++i)
            data[i] = static_cast<qint64>(
                static_cast<__int128>(data[i]) * m_nInverse % m_mod);
    }
}

/* ---- Forward NTT ---- */

QVector<qint64> NumberTheoreticTransform7::forward(const QVector<qint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<qint64> data = input;
    data.resize(m_size, 0);

    // Reduce all values to [0, mod)
    for (auto& v : data)
        v = ((v % m_mod) + m_mod) % m_mod;

    butterfly(data, false);

    double elapsed = timer.elapsed();
    m_stats.transformSize = m_size;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(m_size, m_stats.numTransforms, elapsed);

    return data;
}

/* ---- Inverse NTT ---- */

QVector<qint64> NumberTheoreticTransform7::inverse(const QVector<qint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<qint64> data = input;
    data.resize(m_size, 0);

    for (auto& v : data)
        v = ((v % m_mod) + m_mod) % m_mod;

    butterfly(data, true);

    double elapsed = timer.elapsed();
    m_stats.transformSize = m_size;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(m_size, m_stats.numTransforms, elapsed);

    return data;
}

/* ---- Cyclic convolution via NTT: A * B = INTT(NTT(A) * NTT(B)) ---- */

QVector<qint64> NumberTheoreticTransform7::convolve(const QVector<qint64>& a,
                                                      const QVector<qint64>& b)
{
    QElapsedTimer timer;
    timer.start();

    int resultLen = a.size() + b.size() - 1;

    // Determine required NTT size (next power of 2)
    int nttSize = 1;
    while (nttSize < resultLen) nttSize <<= 1;

    // Save and temporarily use the larger size
    int savedSize = m_size;
    m_size = nttSize;
    precomputeTwiddles();

    // Pad inputs
    QVector<qint64> pa = a;
    QVector<qint64> pb = b;
    pa.resize(nttSize, 0);
    pb.resize(nttSize, 0);

    // Reduce to [0, mod)
    for (auto& v : pa) v = ((v % m_mod) + m_mod) % m_mod;
    for (auto& v : pb) v = ((v % m_mod) + m_mod) % m_mod;

    // Forward NTT on both
    butterfly(pa, false);
    butterfly(pb, false);

    // Pointwise multiply in NTT domain
    QVector<qint64> result(nttSize);
    for (int i = 0; i < nttSize; ++i) {
        result[i] = static_cast<qint64>(
            static_cast<__int128>(pa[i]) * pb[i] % m_mod);
    }

    // Inverse NTT
    butterfly(result, true);

    // Restore original size
    m_size = savedSize;
    precomputeTwiddles();

    // Trim to convolution length
    result.resize(resultLen);

    double elapsed = timer.elapsed();
    m_stats.transformSize = nttSize;
    m_stats.numTransforms += 3;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(nttSize, m_stats.numTransforms, elapsed);

    return result;
}

/* ---- Reset ---- */

void NumberTheoreticTransform7::resetStatistics()
{
    m_twiddles.clear();
    m_invTwiddles.clear();
    m_nInverse = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
    precomputeTwiddles();
}
