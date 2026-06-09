/**
 * @file NumberTheoreticTransform6.cpp
 * @brief NumberTheoreticTransform6 实现
 *
 * 实现数论变换：Cooley-Tukey蝶形运算与NTT多项式乘法。
 */

#include "utils/fft259/NumberTheoreticTransform6.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

NumberTheoreticTransform6::NumberTheoreticTransform6(QObject *parent)
    : QObject(parent) {}
NumberTheoreticTransform6::~NumberTheoreticTransform6() = default;

/* ---- Configuration ---- */

void NumberTheoreticTransform6::setModulus(quint64 mod) { m_mod = mod; }
void NumberTheoreticTransform6::setPrimitiveRoot(quint64 root) { m_root = root; }

/* ---- Modular exponentiation ---- */

quint64 NumberTheoreticTransform6::modPow(quint64 base, quint64 exp, quint64 mod) const
{
    quint64 result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) {
            // Safe multiplication to avoid overflow
            quint64 prod = 0;
            quint64 b = base;
            quint64 e = result;
            // Use __int128 or shift-add for safety
            if (b == 0) { prod = 0; }
            else {
                while (e > 0) {
                    if (e & 1) {
                        prod = (prod + b) % mod;
                    }
                    b = (b * 2) % mod;
                    e >>= 1;
                }
            }
            result = prod;
        }
        exp >>= 1;
        // Square base safely
        quint64 sq = 0;
        quint64 bb = base;
        quint64 eb = base;
        if (bb == 0) { sq = 0; }
        else {
            while (eb > 0) {
                if (eb & 1) sq = (sq + bb) % mod;
                bb = (bb * 2) % mod;
                eb >>= 1;
            }
        }
        base = sq;
    }
    return result;
}

/* ---- Modular inverse ---- */

quint64 NumberTheoreticTransform6::modInverse(quint64 a, quint64 mod) const
{
    // Fermat's little theorem: a^(p-2) mod p
    return modPow(a, mod - 2, mod);
}

/* ---- Next power of two ---- */

int NumberTheoreticTransform6::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Bit-reversal permutation ---- */

void NumberTheoreticTransform6::bitReverse(QVector<quint64>& data) const
{
    int n = data.size();
    int logN = 0;
    for (int tmp = n; tmp > 1; tmp >>= 1) logN++;

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        for (int b = 0; b < logN; ++b) {
            if (i & (1 << b))
                rev |= (1 << (logN - 1 - b));
        }
        if (i < rev)
            std::swap(data[i], data[rev]);
    }
}

/* ---- Cooley-Tukey butterfly NTT ---- */

void NumberTheoreticTransform6::butterflyNTT(QVector<quint64>& data, bool inverse)
{
    int n = data.size();
    if (n <= 1) return;

    bitReverse(data);

    // Determine root of unity
    quint64 root = m_root;
    if (inverse) root = modInverse(root, m_mod);

    for (int len = 2; len <= n; len <<= 1) {
        // w = root^((mod-1)/len) mod mod
        quint64 w = modPow(root, (m_mod - 1) / len, m_mod);
        for (int i = 0; i < n; i += len) {
            quint64 wn = 1;
            for (int j = 0; j < len / 2; ++j) {
                quint64 u = data[i + j];
                // v = data[i+j+len/2] * wn mod mod (safe mult)
                quint64 v = data[i + j + len / 2];
                quint64 prod = 0;
                quint64 b = wn;
                quint64 e = v;
                while (e > 0) {
                    if (e & 1) prod = (prod + b) % m_mod;
                    b = (b * 2) % m_mod;
                    e >>= 1;
                }
                v = prod;

                data[i + j] = (u + v) % m_mod;
                data[i + j + len / 2] = (u + m_mod - v) % m_mod;

                // wn = wn * w mod m_mod (safe mult)
                quint64 newWn = 0;
                b = w;
                e = wn;
                while (e > 0) {
                    if (e & 1) newWn = (newWn + b) % m_mod;
                    b = (b * 2) % m_mod;
                    e >>= 1;
                }
                wn = newWn;
            }
        }
    }

    if (inverse) {
        quint64 nInv = modInverse(static_cast<quint64>(n), m_mod);
        for (int i = 0; i < n; ++i) {
            // data[i] = data[i] * nInv % m_mod
            quint64 prod = 0;
            quint64 b = nInv;
            quint64 e = data[i];
            while (e > 0) {
                if (e & 1) prod = (prod + b) % m_mod;
                b = (b * 2) % m_mod;
                e >>= 1;
            }
            data[i] = prod;
        }
    }
}

/* ---- Forward NTT ---- */

void NumberTheoreticTransform6::forward(QVector<quint64>& data)
{
    QElapsedTimer timer;
    timer.start();

    butterflyNTT(data, false);

    double elapsed = timer.elapsed();
    m_stats.transformSize = data.size();
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformCompleted(data.size(), false, elapsed);
}

/* ---- Inverse NTT ---- */

void NumberTheoreticTransform6::inverse(QVector<quint64>& data)
{
    QElapsedTimer timer;
    timer.start();

    butterflyNTT(data, true);

    double elapsed = timer.elapsed();
    m_stats.transformSize = data.size();
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformCompleted(data.size(), true, elapsed);
}

/* ---- Point-wise multiply ---- */

QVector<quint64> NumberTheoreticTransform6::pointwiseMultiply(
    const QVector<quint64>& a, const QVector<quint64>& b) const
{
    int n = qMin(a.size(), b.size());
    QVector<quint64> result(n);
    for (int i = 0; i < n; ++i) {
        quint64 prod = 0;
        quint64 bb = a[i];
        quint64 e = b[i];
        while (e > 0) {
            if (e & 1) prod = (prod + bb) % m_mod;
            bb = (bb * 2) % m_mod;
            e >>= 1;
        }
        result[i] = prod;
    }
    return result;
}

/* ---- Polynomial multiplication via NTT ---- */

QVector<quint64> NumberTheoreticTransform6::multiply(
    const QVector<quint64>& a, const QVector<quint64>& b)
{
    QElapsedTimer timer;
    timer.start();

    int resultLen = a.size() + b.size() - 1;
    int n = nextPow2(resultLen);

    QVector<quint64> fa(n, 0), fb(n, 0);
    for (int i = 0; i < a.size(); ++i) fa[i] = a[i] % m_mod;
    for (int i = 0; i < b.size(); ++i) fb[i] = b[i] % m_mod;

    butterflyNTT(fa, false);
    butterflyNTT(fb, false);

    QVector<quint64> fc = pointwiseMultiply(fa, fb);
    butterflyNTT(fc, true);

    QVector<quint64> result(resultLen);
    for (int i = 0; i < resultLen; ++i)
        result[i] = fc[i];

    double elapsed = timer.elapsed();
    m_stats.numConvolutions++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit convolutionCompleted(resultLen, elapsed);
    return result;
}

/* ---- Reset ---- */

void NumberTheoreticTransform6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
