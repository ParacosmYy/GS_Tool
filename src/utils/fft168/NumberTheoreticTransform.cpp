/**
 * @file NumberTheoreticTransform.cpp
 * @brief NumberTheoreticTransform 实现
 *
 * 实现数论变换：有限域Cooley-Tukey蝶形、模运算、循环卷积。
 */

#include "utils/fft168/NumberTheoreticTransform.h"

#include <QElapsedTimer>
#include <algorithm>

NumberTheoreticTransform::NumberTheoreticTransform(QObject* parent)
    : QObject(parent)
{
}

NumberTheoreticTransform::~NumberTheoreticTransform() = default;

void NumberTheoreticTransform::setModulus(quint64 mod)
{
    m_mod = qMax(mod, quint64(2));
}

void NumberTheoreticTransform::setPrimitiveRoot(quint64 g)
{
    m_root = g;
}

quint64 NumberTheoreticTransform::modAdd(quint64 a, quint64 b) const
{
    a %= m_mod;
    b %= m_mod;
    return (a >= m_mod - b) ? (a - (m_mod - b)) : (a + b);
}

quint64 NumberTheoreticTransform::modSub(quint64 a, quint64 b) const
{
    a %= m_mod;
    b %= m_mod;
    return (a >= b) ? (a - b) : (m_mod - b + a);
}

quint64 NumberTheoreticTransform::modMul(quint64 a, quint64 b) const
{
    a %= m_mod;
    b %= m_mod;
    /* Use __int128 for overflow-safe multiplication */
    unsigned __int128 prod = static_cast<unsigned __int128>(a) * b;
    return static_cast<quint64>(prod % m_mod);
}

quint64 NumberTheoreticTransform::modPow(quint64 base, quint64 exp) const
{
    quint64 result = 1;
    base %= m_mod;
    while (exp > 0) {
        if (exp & 1) result = modMul(result, base);
        base = modMul(base, base);
        exp >>= 1;
    }
    return result;
}

quint64 NumberTheoreticTransform::modInverse(quint64 a) const
{
    /* Fermat's little theorem: a^(p-2) mod p */
    return modPow(a, m_mod - 2);
}

void NumberTheoreticTransform::nttCore(QVector<quint64>& data, bool inverse)
{
    int n = data.size();
    /* Bit-reversal permutation */
    int log2n = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; log2n++; }

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        int x = i;
        for (int b = 0; b < log2n; ++b) {
            rev = (rev << 1) | (x & 1);
            x >>= 1;
        }
        if (rev > i) std::swap(data[i], data[rev]);
    }

    /* Cooley-Tukey butterfly */
    for (int len = 2; len <= n; len <<= 1) {
        quint64 w = inverse ? modInverse(m_root) : m_root;
        /* w^(m_mod-1)/len is the len-th root of unity */
        quint64 step = (m_mod - 1) / len;
        quint64 wn = modPow(w, step);

        for (int i = 0; i < n; i += len) {
            quint64 wCur = 1;
            for (int j = 0; j < len / 2; ++j) {
                quint64 u = data[i + j];
                quint64 v = modMul(data[i + j + len / 2], wCur);
                data[i + j] = modAdd(u, v);
                data[i + j + len / 2] = modSub(u, v);
                wCur = modMul(wCur, wn);
            }
        }
    }

    if (inverse) {
        quint64 invN = modInverse(static_cast<quint64>(n));
        for (int i = 0; i < n; ++i)
            data[i] = modMul(data[i], invN);
    }
}

QVector<quint64> NumberTheoreticTransform::forward(const QVector<quint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    /* Pad to power of 2 */
    int sz = 1;
    while (sz < n) sz <<= 1;

    QVector<quint64> data(sz, 0);
    for (int i = 0; i < n; ++i) data[i] = input[i] % m_mod;

    nttCore(data, false);

    data.resize(n);

    m_stats.totalForward++;
    m_stats.lastTransformSize = sz;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalForward + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit forwardCompleted(sz);
    return data;
}

QVector<quint64> NumberTheoreticTransform::inverse(const QVector<quint64>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    int n = coeffs.size();
    int sz = 1;
    while (sz < n) sz <<= 1;

    QVector<quint64> data(sz, 0);
    for (int i = 0; i < n; ++i) data[i] = coeffs[i] % m_mod;

    nttCore(data, true);
    data.resize(n);

    m_stats.totalInverse++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalForward + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit inverseCompleted(sz);
    return data;
}

QVector<quint64> NumberTheoreticTransform::cyclicConvolution(
    const QVector<quint64>& a, const QVector<quint64>& b)
{
    int na = a.size(), nb = b.size();
    int len = qMax(na, nb);
    int sz = 1;
    while (sz < len) sz <<= 1;

    QVector<quint64> pa(sz, 0), pb(sz, 0);
    for (int i = 0; i < na; ++i) pa[i] = a[i] % m_mod;
    for (int i = 0; i < nb; ++i) pb[i] = b[i] % m_mod;

    nttCore(pa, false);
    nttCore(pb, false);

    QVector<quint64> result(sz);
    for (int i = 0; i < sz; ++i)
        result[i] = modMul(pa[i], pb[i]);

    nttCore(result, true);

    int resultLen = qMin(na + nb - 1, sz);
    result.resize(qMax(1, resultLen));
    return result;
}

void NumberTheoreticTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
