/**
 * @file NumberTheoreticTransform2.cpp
 * @brief NumberTheoreticTransform2 实现
 *
 * 实现数论变换：有限域NTT蝶形、原根寻找、模逆、CRT大整数卷积。
 */

#include "utils/fft188/NumberTheoreticTransform2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

NumberTheoreticTransform2::NumberTheoreticTransform2(QObject *parent)
    : QObject(parent) {}
NumberTheoreticTransform2::~NumberTheoreticTransform2() = default;

/* ---- Configuration ---- */

void NumberTheoreticTransform2::setModulus(qint64 mod) { m_modulus = qMax(2LL, mod); }
void NumberTheoreticTransform2::setPrimitiveRoot(qint64 root) { m_primitiveRoot = root; }

/* ---- Helpers ---- */

int NumberTheoreticTransform2::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/* ---- Modular exponentiation (binary exponentiation with overflow-safe mul) ---- */

qint64 NumberTheoreticTransform2::modPow(qint64 base, qint64 exp, qint64 mod) const
{
    qint64 result = 1;
    base %= mod;
    if (base < 0) base += mod;
    while (exp > 0) {
        if (exp & 1) {
            // Overflow-safe multiplication
            __int128 a = static_cast<__int128>(result) * base % mod;
            result = static_cast<qint64>(a);
        }
        exp >>= 1;
        __int128 b = static_cast<__int128>(base) * base % mod;
        base = static_cast<qint64>(b);
    }
    return result;
}

/* ---- Modular inverse (extended Euclidean) ---- */

qint64 NumberTheoreticTransform2::modInverse(qint64 a, qint64 mod) const
{
    a = ((a % mod) + mod) % mod;
    qint64 t = 0, newT = 1;
    qint64 r = mod, newR = a;
    while (newR != 0) {
        qint64 q = r / newR;
        qint64 tmp = newT; newT = t - q * newT; t = tmp;
        tmp = newR; newR = r - q * newR; r = tmp;
    }
    if (r > 1) return -1; // No inverse
    if (t < 0) t += mod;
    return t;
}

/* ---- Primality test (deterministic Miller-Rabin for 64-bit) ---- */

bool NumberTheoreticTransform2::isPrime(qint64 n) const
{
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;

    // Write n-1 = d * 2^r
    qint64 d = n - 1;
    int r = 0;
    while (d % 2 == 0) { d /= 2; ++r; }

    // Deterministic witnesses for n < 3.3e24
    QVector<qint64> witnesses = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for (qint64 a : witnesses) {
        if (a >= n) continue;
        qint64 x = modPow(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool composite = true;
        for (int i = 0; i < r - 1; ++i) {
            x = modPow(x, 2, n);
            if (x == n - 1) { composite = false; break; }
        }
        if (composite) return false;
    }
    return true;
}

/* ---- Find primitive root ---- */

qint64 NumberTheoreticTransform2::findPrimitiveRoot(qint64 mod) const
{
    if (mod == 2) return 1;
    if (mod == 4) return 3;

    // Factor mod-1
    qint64 phi = mod - 1;
    QVector<qint64> factors;
    qint64 temp = phi;
    for (qint64 i = 2; i * i <= temp; ++i) {
        if (temp % i == 0) {
            factors.append(i);
            while (temp % i == 0) temp /= i;
        }
    }
    if (temp > 1) factors.append(temp);

    // Test candidates
    for (qint64 g = 2; g < mod; ++g) {
        bool ok = true;
        for (qint64 f : factors) {
            if (modPow(g, phi / f, mod) == 1) { ok = false; break; }
        }
        if (ok) return g;
    }
    return -1; // No primitive root found
}

/* ---- Bit-reversal permutation ---- */

void NumberTheoreticTransform2::bitReverse(QVector<qint64>& data) const
{
    int N = data.size();
    int log2N = 0;
    while ((1 << log2N) < N) ++log2N;
    for (int i = 0; i < N; ++i) {
        int j = 0;
        for (int b = 0; b < log2N; ++b)
            j = (j << 1) | ((i >> b) & 1);
        if (j > i) std::swap(data[i], data[j]);
    }
}

/* ---- In-place NTT butterfly ---- */

void NumberTheoreticTransform2::nttButterfly(QVector<qint64>& data, bool invert)
{
    int N = data.size();
    bitReverse(data);

    for (int len = 2; len <= N; len *= 2) {
        qint64 wRoot = invert
            ? modInverse(m_primitiveRoot, m_modulus)
            : m_primitiveRoot;
        qint64 wLen = modPow(wRoot, (m_modulus - 1) / len, m_modulus);

        for (int i = 0; i < N; i += len) {
            qint64 w = 1;
            for (int j = 0; j < len / 2; ++j) {
                qint64 u = data[i + j];
                __int128 v128 = static_cast<__int128>(w) * data[i + j + len / 2] % m_modulus;
                qint64 v = static_cast<qint64>(v128);
                data[i + j] = (u + v) % m_modulus;
                data[i + j + len / 2] = (u - v + m_modulus) % m_modulus;
                __int128 wNew = static_cast<__int128>(w) * wLen % m_modulus;
                w = static_cast<qint64>(wNew);
            }
        }
    }

    if (invert) {
        qint64 nInv = modInverse(N, m_modulus);
        for (int i = 0; i < N; ++i) {
            __int128 tmp = static_cast<__int128>(data[i]) * nInv % m_modulus;
            data[i] = static_cast<qint64>(tmp);
        }
    }
}

/* ---- Forward NTT ---- */

QVector<qint64> NumberTheoreticTransform2::forward(const QVector<qint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = nextPow2(input.size());
    QVector<qint64> data(N, 0);
    for (int i = 0; i < input.size(); ++i)
        data[i] = ((input[i] % m_modulus) + m_modulus) % m_modulus;

    nttButterfly(data, false);

    m_stats.totalTransforms++;
    m_stats.transformSize = N;
    m_stats.modulus = m_modulus;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, m_modulus, timer.elapsed());
    return data;
}

/* ---- Inverse NTT ---- */

QVector<qint64> NumberTheoreticTransform2::inverse(const QVector<qint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<qint64> data = input;
    nttButterfly(data, true);

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(input.size(), m_modulus, timer.elapsed());
    return data;
}

/* ---- Polynomial multiplication via NTT convolution ---- */

QVector<qint64> NumberTheoreticTransform2::multiply(const QVector<qint64>& a,
                                                     const QVector<qint64>& b)
{
    QElapsedTimer timer;
    timer.start();

    int resultSize = a.size() + b.size() - 1;
    int N = nextPow2(resultSize);

    QVector<qint64> fa(N, 0), fb(N, 0);
    for (int i = 0; i < a.size(); ++i)
        fa[i] = ((a[i] % m_modulus) + m_modulus) % m_modulus;
    for (int i = 0; i < b.size(); ++i)
        fb[i] = ((b[i] % m_modulus) + m_modulus) % m_modulus;

    nttButterfly(fa, false);
    nttButterfly(fb, false);

    // Pointwise multiply
    for (int i = 0; i < N; ++i) {
        __int128 tmp = static_cast<__int128>(fa[i]) * fb[i] % m_modulus;
        fa[i] = static_cast<qint64>(tmp);
    }

    nttButterfly(fa, true);

    // Trim to result size
    QVector<qint64> result(resultSize);
    for (int i = 0; i < resultSize; ++i)
        result[i] = fa[i];

    m_stats.totalTransforms++;
    m_stats.transformSize = N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, m_modulus, timer.elapsed());
    return result;
}

/* ---- CRT merge two modular results ---- */

QVector<qint64> NumberTheoreticTransform2::crtMerge(
    const QVector<qint64>& r1, qint64 m1,
    const QVector<qint64>& r2, qint64 m2) const
{
    int n = qMin(r1.size(), r2.size());
    QVector<qint64> result(n);

    qint64 m1Inv = modInverse(m1, m2);
    qint64 m2Inv = modInverse(m2, m1);
    qint64 M = m1 * m2; // Combined modulus (may overflow for large primes)

    for (int i = 0; i < n; ++i) {
        // CRT: x = r1 + m1 * ((r2 - r1) * m1Inv mod m2)
        qint64 diff = ((r2[i] - r1[i]) % m2 + m2) % m2;
        __int128 tmp = static_cast<__int128>(diff) * m1Inv % m2;
        __int128 x = (static_cast<__int128>(r1[i]) + static_cast<__int128>(m1) * tmp);
        result[i] = static_cast<qint64>(x);
    }
    return result;
}

/* ---- Reset ---- */

void NumberTheoreticTransform2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
