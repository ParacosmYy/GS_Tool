/**
 * @file NumberTheoreticTransform9.cpp
 * @brief NumberTheoreticTransform9 实现
 *
 * 实现数论变换：Cooley-Tukey蝶形分解与盲化因子实现零知识证明兼容数论变换。
 */

#include "utils/fft301/NumberTheoreticTransform9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

NumberTheoreticTransform9::NumberTheoreticTransform9(QObject *parent)
    : QObject(parent) {}

NumberTheoreticTransform9::~NumberTheoreticTransform9() = default;

/* ---- Configuration ---- */

void NumberTheoreticTransform9::setModulus(qint64 mod) { m_modulus = qMax(2LL, mod); }
void NumberTheoreticTransform9::setPrimitiveRoot(qint64 root) { m_primitiveRoot = root; }
void NumberTheoreticTransform9::setBlindingEnabled(bool enabled) { m_blinding = enabled; }

/* ---- Modular exponentiation (square-and-multiply) ---- */

qint64 NumberTheoreticTransform9::modPow(qint64 base, qint64 exp, qint64 mod) const
{
    qint64 result = 1;
    base %= mod;
    if (base < 0) base += mod;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % mod;
        exp >>= 1;
        base = (base * base) % mod;
    }
    return result;
}

/* ---- Modular inverse via Fermat's little theorem ---- */

qint64 NumberTheoreticTransform9::modInverse(qint64 a, qint64 mod) const
{
    return modPow(a, mod - 2, mod);
}

/* ---- Find n-th root of unity ---- */

qint64 NumberTheoreticTransform9::findRootOfUnity(int n) const
{
    // omega = primitiveRoot^((p-1)/n) mod p
    qint64 exp = (m_modulus - 1) / n;
    return modPow(m_primitiveRoot, exp, m_modulus);
}

/* ---- Bit-reversal permutation ---- */

void NumberTheoreticTransform9::bitReverse(QVector<qint64>& data) const
{
    int n = data.size();
    int logN = 0;
    while ((1 << logN) < n) ++logN;

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        int x = i;
        for (int j = 0; j < logN; ++j) {
            rev = (rev << 1) | (x & 1);
            x >>= 1;
        }
        if (i < rev) std::swap(data[i], data[rev]);
    }
}

/* ---- Generate blinding factor for ZKP masking ---- */

qint64 NumberTheoreticTransform9::generateBlindingFactor() const
{
    // Deterministic pseudo-random blinding scalar in [1, modulus-1]
    // Uses a simple LCG seeded from modulus for reproducibility
    static qint64 state = 42;
    state = (state * 1103515245 + 12345) % m_modulus;
    return qMax(1LL, state);
}

/* ---- Apply blinding to data ---- */

void NumberTheoreticTransform9::applyBlinding(QVector<qint64>& data) const
{
    qint64 blind = generateBlindingFactor();
    for (auto& v : data)
        v = (v * blind) % m_modulus;
}

/* ---- Core Cooley-Tukey butterfly NTT ---- */

void NumberTheoreticTransform9::butterflyNTT(QVector<qint64>& data, bool inverse)
{
    int n = data.size();
    if (n <= 1) return;

    bitReverse(data);

    // Cooley-Tukey iterative butterfly stages
    for (int len = 2; len <= n; len <<= 1) {
        // Root of unity for this stage
        qint64 omega = findRootOfUnity(len);
        if (inverse)
            omega = modInverse(omega, m_modulus);

        qint64 wStep = 1;
        int halfLen = len >> 1;

        for (int i = 0; i < n; i += len) {
            wStep = 1;
            for (int j = 0; j < halfLen; ++j) {
                // Butterfly: u = data[i+j], v = w * data[i+j+halfLen]
                qint64 u = data[i + j];
                qint64 v = (wStep * data[i + j + halfLen]) % m_modulus;

                // Store results
                data[i + j] = (u + v) % m_modulus;
                data[i + j + halfLen] = (u - v + m_modulus) % m_modulus;

                // Advance twiddle factor
                wStep = (wStep * omega) % m_modulus;
            }
        }
    }

    // Inverse normalization: multiply by 1/n
    if (inverse) {
        qint64 invN = modInverse(n, m_modulus);
        for (auto& v : data)
            v = (v * invN) % m_modulus;
    }
}

/* ---- Forward NTT ---- */

NumberTheoreticTransform9::TransformResult NumberTheoreticTransform9::forward(const QVector<qint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    TransformResult result;

    // Pad to next power of 2
    int n = 1;
    while (n < input.size()) n <<= 1;

    QVector<qint64> data(n, 0);
    for (int i = 0; i < input.size(); ++i)
        data[i] = ((input[i] % m_modulus) + m_modulus) % m_modulus;

    // Apply blinding factor for ZKP compatibility
    if (m_blinding)
        applyBlinding(data);

    // Execute Cooley-Tukey butterfly NTT
    butterflyNTT(data, false);

    result.values = data;
    result.size = n;
    result.modulus = m_modulus;

    double elapsed = timer.elapsed();
    result.elapsedMs = elapsed;
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.transformSize = n;
    m_stats.modulus = m_modulus;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(n, m_modulus, elapsed);
    return result;
}

/* ---- Inverse NTT ---- */

NumberTheoreticTransform9::TransformResult NumberTheoreticTransform9::inverse(const QVector<qint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    TransformResult result;

    int n = input.size();
    QVector<qint64> data(n);
    for (int i = 0; i < n; ++i)
        data[i] = ((input[i] % m_modulus) + m_modulus) % m_modulus;

    // Inverse butterfly NTT
    butterflyNTT(data, true);

    // If blinding was used during forward, remove blinding here
    // (In real ZKP protocol this would use the blinding factor's inverse)
    if (m_blinding) {
        qint64 blind = generateBlindingFactor();
        qint64 invBlind = modInverse(blind, m_modulus);
        for (auto& v : data)
            v = (v * invBlind) % m_modulus;
    }

    result.values = data;
    result.size = n;
    result.modulus = m_modulus;

    double elapsed = timer.elapsed();
    result.elapsedMs = elapsed;
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(n, m_modulus, elapsed);
    return result;
}

/* ---- Pointwise multiply ---- */

QVector<qint64> NumberTheoreticTransform9::pointwiseMultiply(const QVector<qint64>& a,
                                                              const QVector<qint64>& b) const
{
    int n = qMin(a.size(), b.size());
    QVector<qint64> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = (a[i] * b[i]) % m_modulus;
    return result;
}

/* ---- Reset ---- */

void NumberTheoreticTransform9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
