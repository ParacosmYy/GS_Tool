/**
 * @file NumberTheoreticTransform5.cpp
 * @brief NumberTheoreticTransform5 实现
 *
 * 实现数论变换：Barrett约减模算术与Bluestein扩展支持任意复合长度。
 */

#include "utils/fft245/NumberTheoreticTransform5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

NumberTheoreticTransform5::NumberTheoreticTransform5(QObject *parent)
    : QObject(parent)
{
    precomputeBarrett();
}

NumberTheoreticTransform5::~NumberTheoreticTransform5() = default;

/* ---- Configuration ---- */

void NumberTheoreticTransform5::setModulus(quint64 mod)
{
    m_mod = mod;
    precomputeBarrett();
}

void NumberTheoreticTransform5::setPrimitiveRoot(quint64 g)
{
    m_primRoot = g;
}

/* ---- Barrett precomputation ---- */

void NumberTheoreticTransform5::precomputeBarrett()
{
    // k = floor(2^64 / m) for Barrett reduction
    // We use 2^32 scale for simplicity with 64-bit arithmetic
    m_barrettK = (static_cast<quint64>(1) << 32) / m_mod;
}

/* ---- Barrett reduction ---- */

quint64 NumberTheoreticTransform5::barrettReduce(quint64 a) const
{
    // Approximate quotient: q ≈ (a * k) >> 32
    quint64 q = (a * m_barrettK) >> 32;
    quint64 r = a - q * m_mod;
    if (r >= m_mod) r -= m_mod;
    return r;
}

/* ---- Modular multiplication ---- */

quint64 NumberTheoreticTransform5::modMul(quint64 a, quint64 b) const
{
    // Use __int128-like decomposition to avoid overflow
    // a * b can overflow quint64, so decompose
    quint64 a_hi = a >> 32;
    quint64 a_lo = a & 0xFFFFFFFF;
    quint64 b_hi = b >> 32;
    quint64 b_lo = b & 0xFFFFFFFF;

    quint64 result = a_lo * b_lo;
    result += (a_hi * b_lo) << 32;
    result += (a_lo * b_hi) << 32;

    return barrettReduce(result);
}

/* ---- Modular exponentiation ---- */

quint64 NumberTheoreticTransform5::modPow(quint64 base, quint64 exp) const
{
    quint64 result = 1;
    base = barrettReduce(base);
    while (exp > 0) {
        if (exp & 1)
            result = modMul(result, base);
        exp >>= 1;
        base = modMul(base, base);
    }
    return result;
}

/* ---- Bit-reverse permutation ---- */

void NumberTheoreticTransform5::bitReverse(QVector<quint64>& data) const
{
    int n = data.size();
    int bits = 0;
    while ((1 << bits) < n) bits++;

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        int val = i;
        for (int b = 0; b < bits; ++b) {
            rev = (rev << 1) | (val & 1);
            val >>= 1;
        }
        if (i < rev) std::swap(data[i], data[rev]);
    }
}

/* ---- Power-of-two Cooley-Tukey NTT ---- */

void NumberTheoreticTransform5::nttCT(QVector<quint64>& data, bool inverse)
{
    int n = data.size();
    bitReverse(data);

    for (int len = 2; len <= n; len <<= 1) {
        quint64 w = inverse
            ? modPow(m_primRoot, m_mod - 1 - (m_mod - 1) / len)
            : modPow(m_primRoot, (m_mod - 1) / len);

        for (int i = 0; i < n; i += len) {
            quint64 wn = 1;
            for (int j = 0; j < len / 2; ++j) {
                quint64 u = data[i + j];
                quint64 v = modMul(data[i + j + len / 2], wn);
                data[i + j] = barrettReduce(u + v);
                data[i + j + len / 2] = barrettReduce(u + m_mod - v);
                wn = modMul(wn, w);
            }
        }
    }

    if (inverse) {
        quint64 invN = modPow(n, m_mod - 2);
        for (auto& x : data)
            x = modMul(x, invN);
    }
}

/* ---- Next power of two ---- */

int NumberTheoreticTransform5::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Find n-th root of unity ---- */

quint64 NumberTheoreticTransform5::findRoot(int n) const
{
    // g^((mod-1)/n) is an n-th root of unity
    return modPow(m_primRoot, (m_mod - 1) / n);
}

/* ---- Bluestein's algorithm ---- */

QVector<quint64> NumberTheoreticTransform5::bluestein(const QVector<quint64>& input, bool inverse)
{
    int n = input.size();
    int m = nextPow2(2 * n - 1);  // Pad to power of 2 >= 2n-1

    // Chirp: h[k] = w^(k^2/2) where w = n-th root of unity
    quint64 w = inverse ? modPow(m_primRoot, m_mod - 1 - (m_mod - 1) / n)
                        : modPow(m_primRoot, (m_mod - 1) / n);

    QVector<quint64> chirp(n, 0);
    QVector<quint64> a(m, 0);
    QVector<quint64> b(m, 0);

    for (int k = 0; k < n; ++k) {
        int idx = static_cast<int>((static_cast<qint64>(k) * k / 2) % n);
        chirp[k] = modPow(w, idx);
    }

    // Build padded sequences
    for (int i = 0; i < n; ++i) {
        a[i] = modMul(input[i], chirp[i]);
        int j = static_cast<int>((static_cast<qint64>(i) * i / 2) % n);
        b[i] = modPow(m_primRoot, m_mod - 1 - j);  // Inverse chirp
    }
    // Neg-wrap b
    for (int i = 1; i < n; ++i) {
        int j = static_cast<int>((static_cast<qint64>(i) * i / 2) % n);
        b[m - i] = modPow(m_primRoot, m_mod - 1 - j);
    }

    // Convolution via NTT
    nttCT(a, false);
    nttCT(b, false);
    for (int i = 0; i < m; ++i)
        a[i] = modMul(a[i], b[i]);
    nttCT(a, true);

    // Extract result
    QVector<quint64> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = modMul(a[i], chirp[i]);
        if (inverse) {
            quint64 invN = modPow(n, m_mod - 2);
            result[i] = modMul(result[i], invN);
        }
    }
    return result;
}

/* ---- Forward NTT ---- */

QVector<quint64> NumberTheoreticTransform5::forward(const QVector<quint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<quint64> data = input;

    bool usedBluestein = false;
    // Check if n is power of 2 and divides mod-1
    if ((n & (n - 1)) == 0) {
        nttCT(data, false);
    } else {
        data = bluestein(input, false);
        usedBluestein = true;
    }

    m_stats.transformSize = n;
    m_stats.usedBluestein = usedBluestein;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(n, usedBluestein, timer.elapsed());
    return data;
}

/* ---- Inverse NTT ---- */

QVector<quint64> NumberTheoreticTransform5::inverse(const QVector<quint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<quint64> data = input;

    bool usedBluestein = false;
    if ((n & (n - 1)) == 0) {
        nttCT(data, true);
    } else {
        data = bluestein(input, true);
        usedBluestein = true;
    }

    m_stats.transformSize = n;
    m_stats.usedBluestein = usedBluestein;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return data;
}

/* ---- Polynomial multiplication ---- */

QVector<quint64> NumberTheoreticTransform5::multiply(const QVector<quint64>& a,
                                                       const QVector<quint64>& b)
{
    QElapsedTimer timer;
    timer.start();

    int resultLen = a.size() + b.size() - 1;
    int n = nextPow2(resultLen);

    QVector<quint64> pa(n, 0), pb(n, 0);
    for (int i = 0; i < a.size(); ++i) pa[i] = barrettReduce(a[i]);
    for (int i = 0; i < b.size(); ++i) pb[i] = barrettReduce(b[i]);

    nttCT(pa, false);
    nttCT(pb, false);

    QVector<quint64> pc(n);
    for (int i = 0; i < n; ++i)
        pc[i] = modMul(pa[i], pb[i]);

    nttCT(pc, true);
    pc.resize(resultLen);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return pc;
}

/* ---- Reset ---- */

void NumberTheoreticTransform5::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
