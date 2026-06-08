/**
 * @file NumberTheoreticTransform3.cpp
 * @brief NumberTheoreticTransform3 实现
 *
 * 实现数论变换：Barrett约减、Fermat数变换、蝶形运算。
 */

#include "utils/fft217/NumberTheoreticTransform3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

NumberTheoreticTransform3::NumberTheoreticTransform3(QObject *parent)
    : QObject(parent)
{
    setParameters(256);
}

NumberTheoreticTransform3::~NumberTheoreticTransform3() = default;

/* ---- Configuration ---- */

void NumberTheoreticTransform3::setParameters(int size, quint64 modulus,
                                                quint64 primitiveRoot)
{
    // Ensure power of two
    int n = 1;
    while (n < size) n <<= 1;
    m_size = n;
    m_modulus = (modulus > 0) ? modulus : 998244353;  // NTT-friendly prime
    m_primitiveRoot = (primitiveRoot > 0) ? primitiveRoot : 3;

    computeBitRev();
    computeRootTables();
    computeBarrettFactor();

    m_stats.transformSize = m_size;
    m_stats.modulus = m_modulus;
    m_stats.primitiveRoot = m_primitiveRoot;
}

/* ---- Bit-reversal permutation ---- */

void NumberTheoreticTransform3::computeBitRev()
{
    m_bitRev.resize(m_size);
    int logN = 0;
    for (int tmp = m_size; tmp > 1; tmp >>= 1) logN++;
    for (int i = 0; i < m_size; ++i) {
        int rev = 0;
        for (int b = 0; b < logN; ++b)
            if (i & (1 << b)) rev |= (1 << (logN - 1 - b));
        m_bitRev[i] = rev;
    }
}

/* ---- Root power tables ---- */

void NumberTheoreticTransform3::computeRootTables()
{
    quint64 root = modPow(m_primitiveRoot, (m_modulus - 1) / m_size, m_modulus);
    quint64 invRoot = modInverse(root, m_modulus);

    m_rootPowers.resize(m_size);
    m_invRootPowers.resize(m_size);
    m_rootPowers[0] = 1;
    m_invRootPowers[0] = 1;

    for (int i = 1; i < m_size; ++i) {
        m_rootPowers[i] = barrettReduce(
            m_rootPowers[i - 1] * root, m_modulus, m_barrettFactor);
        m_invRootPowers[i] = barrettReduce(
            m_invRootPowers[i - 1] * invRoot, m_modulus, m_barrettFactor);
    }
}

/* ---- Barrett factor ---- */

void NumberTheoreticTransform3::computeBarrettFactor()
{
    // Factor = floor(2^64 / modulus) for 64-bit Barrett reduction
    // Use 2^32 for simplicity with uint64 arithmetic
    m_barrettFactor = (static_cast<quint64>(1) << 32) / m_modulus;
}

/* ---- Barrett reduction ---- */

quint64 NumberTheoreticTransform3::barrettReduce(quint64 x, quint64 m,
                                                   quint64 factor) const
{
    if (x < m) return x;
    quint64 q = (x >> 32) * factor >> 32;
    quint64 r = x - q * m;
    while (r >= m) r -= m;
    return r;
}

/* ---- Modular exponentiation ---- */

quint64 NumberTheoreticTransform3::modPow(quint64 base, quint64 exp,
                                            quint64 m) const
{
    quint64 result = 1;
    base %= m;
    while (exp > 0) {
        if (exp & 1)
            result = barrettReduce(result * base, m, m_barrettFactor);
        exp >>= 1;
        base = barrettReduce(base * base, m, m_barrettFactor);
    }
    return result;
}

/* ---- Modular inverse ---- */

quint64 NumberTheoreticTransform3::modInverse(quint64 a, quint64 m) const
{
    return modPow(a, m - 2, m);  // Fermat's little theorem for prime m
}

/* ---- Find primitive root ---- */

quint64 NumberTheoreticTransform3::findPrimitiveRoot(quint64 m) const
{
    // Factor m-1
    quint64 phi = m - 1;
    QVector<quint64> factors;
    quint64 n = phi;
    for (quint64 d = 2; d * d <= n; ++d) {
        if (n % d == 0) {
            factors.append(d);
            while (n % d == 0) n /= d;
        }
    }
    if (n > 1) factors.append(n);

    // Test candidates
    for (quint64 g = 2; g < m; ++g) {
        bool ok = true;
        for (quint64 f : factors) {
            if (modPow(g, phi / f, m) == 1) { ok = false; break; }
        }
        if (ok) return g;
    }
    return 0;
}

/* ---- Butterfly with Barrett reduction ---- */

void NumberTheoreticTransform3::butterfly(QVector<quint64>& data, bool inverse)
{
    const QVector<quint64>& powers = inverse ? m_invRootPowers : m_rootPowers;

    for (int len = 2; len <= m_size; len <<= 1) {
        int halfLen = len >> 1;
        int step = m_size / len;
        for (int i = 0; i < m_size; i += len) {
            for (int j = 0; j < halfLen; ++j) {
                int wIdx = j * step;
                quint64 w = powers[wIdx];
                quint64 u = data[i + j];
                quint64 v = barrettReduce(data[i + j + halfLen] * w,
                                           m_modulus, m_barrettFactor);
                data[i + j] = barrettReduce(u + v, m_modulus, m_barrettFactor);
                data[i + j + halfLen] = barrettReduce(
                    u + m_modulus - v, m_modulus, m_barrettFactor);
            }
        }
    }

    if (inverse) {
        quint64 invN = modInverse(m_size, m_modulus);
        for (int i = 0; i < m_size; ++i)
            data[i] = barrettReduce(data[i] * invN, m_modulus, m_barrettFactor);
    }
}

/* ---- Forward NTT ---- */

QVector<quint64> NumberTheoreticTransform3::forward(const QVector<quint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint64> data(m_size, 0);
    for (int i = 0; i < qMin(input.size(), m_size); ++i)
        data[m_bitRev[i]] = input[i] % m_modulus;

    butterfly(data, false);

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    emit transformCompleted(m_size, m_modulus, timer.elapsed());
    return data;
}

/* ---- Inverse NTT ---- */

QVector<quint64> NumberTheoreticTransform3::inverse(const QVector<quint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint64> data(m_size, 0);
    for (int i = 0; i < qMin(input.size(), m_size); ++i)
        data[m_bitRev[i]] = input[i] % m_modulus;

    butterfly(data, true);

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    return data;
}

/* ---- Polynomial multiplication via NTT ---- */

QVector<quint64> NumberTheoreticTransform3::multiply(
    const QVector<quint64>& a, const QVector<quint64>& b)
{
    QElapsedTimer timer;
    timer.start();

    int resultLen = a.size() + b.size() - 1;
    int n = 1;
    while (n < resultLen) n <<= 1;
    setParameters(n, m_modulus, m_primitiveRoot);

    // Zero-pad inputs
    QVector<quint64> pa(n, 0), pb(n, 0);
    for (int i = 0; i < a.size(); ++i) pa[i] = a[i] % m_modulus;
    for (int i = 0; i < b.size(); ++i) pb[i] = b[i] % m_modulus;

    QVector<quint64> fa = forward(pa);
    QVector<quint64> fb = forward(pb);

    // Pointwise multiply
    QVector<quint64> fc(n);
    for (int i = 0; i < n; ++i)
        fc[i] = barrettReduce(fa[i] * fb[i], m_modulus, m_barrettFactor);

    QVector<quint64> result = inverse(fc);
    result.resize(resultLen);

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    return result;
}

/* ---- Reset ---- */

void NumberTheoreticTransform3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_bitRev.clear();
    m_rootPowers.clear();
    m_invRootPowers.clear();
}
