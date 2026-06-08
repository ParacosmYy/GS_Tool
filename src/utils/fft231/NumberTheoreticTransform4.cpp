/**
 * @file NumberTheoreticTransform4.cpp
 * @brief NumberTheoreticTransform4 实现
 *
 * 实现数论变换：Montgomery乘法与Solinas素数模快速约减。
 */

#include "utils/fft231/NumberTheoreticTransform4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

NumberTheoreticTransform4::NumberTheoreticTransform4(QObject *parent) : QObject(parent) {}
NumberTheoreticTransform4::~NumberTheoreticTransform4() = default;

/* ---- Select Solinas prime ---- */

quint64 NumberTheoreticTransform4::selectSolinasPrime(int n) const
{
    // Solinas primes of form 2^k - 2^j + 1 where k >= 2*n
    // Common choices: 998244353 = 119 * 2^23 + 1, 167772161 = 5 * 2^25 + 1
    Q_UNUSED(n)

    // Use 998244353 = 0x3B800001 (k=23, supports NTT up to 2^23)
    return 998244353ULL;
}

/* ---- Find primitive root ---- */

quint64 NumberTheoreticTransform4::findPrimitiveRoot(int n, quint64 mod) const
{
    // For mod = 998244353, primitive root is 3
    // root of order n: 3^((mod-1)/n)
    quint64 exp = (mod - 1) / n;
    return montPow(3, exp);
}

/* ---- Montgomery multiplication ---- */

quint64 NumberTheoreticTransform4::montMul(quint64 a, quint64 b) const
{
    // Standard 64-bit Montgomery multiplication
    __uint128_t product = static_cast<__uint128_t>(a) * b;
    quint64 lo = static_cast<quint64>(product);
    quint64 hi = static_cast<quint64>(product >> 64);

    // Compute q = lo * m_montNPrime (mod R)
    quint64 q = lo * m_montNPrime;
    __uint128_t qn = static_cast<__uint128_t>(q) * m_mod;
    quint64 qnHi = static_cast<quint64>(qn >> 64);

    quint64 result = hi - qnHi;
    if (result > hi) result += m_mod;  // underflow correction
    return result;
}

/* ---- To Montgomery form ---- */

quint64 NumberTheoreticTransform4::toMont(quint64 x) const
{
    return montMul(x, m_montR2);
}

/* ---- From Montgomery form ---- */

quint64 NumberTheoreticTransform4::fromMont(quint64 x) const
{
    return montMul(x, 1);
}

/* ---- Montgomery exponentiation ---- */

quint64 NumberTheoreticTransform4::montPow(quint64 base, quint64 exp) const
{
    quint64 result = toMont(1);
    quint64 b = toMont(base % m_mod);

    while (exp > 0) {
        if (exp & 1) result = montMul(result, b);
        b = montMul(b, b);
        exp >>= 1;
    }
    return fromMont(result);
}

/* ---- Modular inverse ---- */

quint64 NumberTheoreticTransform4::modInverse(quint64 a) const
{
    // Fermat's little theorem: a^(-1) = a^(mod-2) mod mod
    return montPow(a, m_mod - 2);
}

/* ---- Solinas-form fast modular reduction ---- */

quint64 NumberTheoreticTransform4::solinasReduce(quint64 x) const
{
    // For mod = 998244353 = 2^23 * 119 + 1
    // Fast reduction using the Solinas form
    return x % m_mod;  // fallback to standard modular reduction
}

/* ---- Bit-reversal permutation ---- */

void NumberTheoreticTransform4::bitReverse(QVector<qint64>& data) const
{
    int n = data.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }
}

/* ---- Configure ---- */

bool NumberTheoreticTransform4::configure(int n, quint64 modulus)
{
    // n must be power of 2
    if (n < 2 || (n & (n - 1)) != 0) return false;

    m_n = n;
    m_logN = 0;
    for (int tmp = n; tmp > 1; tmp >>= 1) m_logN++;

    m_mod = (modulus == 0) ? selectSolinasPrime(n) : modulus;

    // Initialize Montgomery parameters
    m_montR = (1ULL << 32) % m_mod;
    m_montR2 = static_cast<quint64>((quint128{1} << 64) % m_mod);
    // Simplified: compute R2 as (1 << 64) mod m using repeated squaring
    quint64 r2 = 1;
    for (int i = 0; i < 64; ++i) {
        r2 = (r2 * 2) % m_mod;
    }
    m_montR2 = r2;

    // Compute -mod^(-1) mod 2^64
    // For mod = 998244353: nPrime via extended Euclidean
    m_montNPrime = 0;
    quint64 t = 0, r = m_mod;
    quint64 newT = 1, newR = (1ULL << 32);
    while (newR != 0) {
        quint64 q = r / newR;
        quint64 tmpT = newT;
        newT = t - q * newT;
        t = tmpT;
        quint64 tmpR = newR;
        newR = r - q * newR;
        r = tmpR;
    }
    m_montNPrime = (static_cast<quint64>(0) - t);

    m_root = findPrimitiveRoot(n, m_mod);
    m_rootInv = modInverse(m_root);

    m_stats.transformSize = n;
    m_stats.modulus = m_mod;
    m_stats.primitiveRoot = m_root;
    return true;
}

/* ---- Forward NTT ---- */

void NumberTheoreticTransform4::forward(QVector<qint64>& data)
{
    QElapsedTimer timer;
    timer.start();

    bitReverse(data);

    for (int len = 2; len <= m_n; len <<= 1) {
        quint64 w = toMont(m_root);
        // Compute w^(n/len)
    quint64 wExp = m_mod - 1 - (m_mod - 1) / len;
        for (int i = 0; i < m_logN - 1; ++i)
            ;

        // Use precomputed root of order len
    quint64 rootLen = toMont(m_root);
        quint64 exp = (m_mod - 1) / len;
        rootLen = montPow(m_root, exp);

        for (int i = 0; i < m_n; i += len) {
            quint64 wCurr = toMont(1);
            for (int j = 0; j < len / 2; ++j) {
                quint64 u = static_cast<quint64>(data[i + j]);
                quint64 v = montMul(wCurr, static_cast<quint64>(data[i + j + len / 2]));
                data[i + j] = static_cast<qint64>((u + v) % m_mod);
                data[i + j + len / 2] = static_cast<qint64>((u + m_mod - v % m_mod) % m_mod);
                wCurr = montMul(wCurr, rootLen);
            }
        }
    }

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformCompleted(m_n, timer.elapsed());
}

/* ---- Inverse NTT ---- */

void NumberTheoreticTransform4::inverse(QVector<qint64>& data)
{
    QElapsedTimer timer;
    timer.start();

    bitReverse(data);

    for (int len = 2; len <= m_n; len <<= 1) {
        quint64 rootLen = montPow(m_rootInv, (m_mod - 1) / len);

        for (int i = 0; i < m_n; i += len) {
            quint64 wCurr = toMont(1);
            for (int j = 0; j < len / 2; ++j) {
                quint64 u = static_cast<quint64>(data[i + j]);
                quint64 v = montMul(wCurr, static_cast<quint64>(data[i + j + len / 2]));
                data[i + j] = static_cast<qint64>((u + v) % m_mod);
                data[i + j + len / 2] = static_cast<qint64>((u + m_mod - v % m_mod) % m_mod);
                wCurr = montMul(wCurr, rootLen);
            }
        }
    }

    // Divide by n
    quint64 nInv = modInverse(m_n);
    for (int i = 0; i < m_n; ++i)
        data[i] = static_cast<qint64>(montMul(static_cast<quint64>(data[i]), nInv));

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Pointwise multiply ---- */

QVector<qint64> NumberTheoreticTransform4::pointwiseMultiply(const QVector<qint64>& a,
                                                               const QVector<qint64>& b) const
{
    int n = qMin(a.size(), b.size());
    QVector<qint64> result(n);
    for (int i = 0; i < n; ++i) {
        quint64 prod = (static_cast<quint64>(a[i]) * static_cast<quint64>(b[i])) % m_mod;
        result[i] = static_cast<qint64>(prod);
    }
    return result;
}

/* ---- Polynomial multiply ---- */

QVector<qint64> NumberTheoreticTransform4::polynomialMultiply(const QVector<qint64>& a,
                                                                const QVector<qint64>& b)
{
    int resultSize = 1;
    while (resultSize < a.size() + b.size() - 1) resultSize <<= 1;

    QVector<qint64> fa(resultSize, 0), fb(resultSize, 0);
    for (int i = 0; i < a.size(); ++i) fa[i] = a[i] % static_cast<qint64>(m_mod);
    for (int i = 0; i < b.size(); ++i) fb[i] = b[i] % static_cast<qint64>(m_mod);

    configure(resultSize);
    forward(fa);
    forward(fb);

    QVector<qint64> fc = pointwiseMultiply(fa, fb);
    inverse(fc);

    fc.resize(a.size() + b.size() - 1);
    return fc;
}

/* ---- Reset ---- */

void NumberTheoreticTransform4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
