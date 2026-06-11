/**
 * @file NumberTheoreticTransform8.cpp
 * @brief NumberTheoreticTransform8 实现
 *
 * 实现数论变换：Montgomery乘法与Barrett约减的高效模算术大整数卷积。
 */

#include "utils/fft287/NumberTheoreticTransform8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

NumberTheoreticTransform8::NumberTheoreticTransform8(QObject *parent)
    : QObject(parent)
{
    setConfig(NTTConfig{});
}

NumberTheoreticTransform8::~NumberTheoreticTransform8() = default;

/* ---- Configuration ---- */

void NumberTheoreticTransform8::setConfig(const NTTConfig& cfg)
{
    m_config = cfg;
    initModParams();
}

/* ---- Initialize Montgomery and Barrett parameters ---- */

void NumberTheoreticTransform8::initModParams()
{
    quint64 mod = m_config.modulus;

    // Barrett: mu = floor(2^(2*64) / mod), approximate with 64-bit
    m_barrettK = 64;
    // Compute mu = (1 << 64) / mod using double for approximation
    m_barrettMu = static_cast<quint64>(
        static_cast<double>((static_cast<__uint128_t>(1) << 64)) / static_cast<double>(mod));

    // Montgomery: find R = 2^k > mod, coprime to mod
    m_montR = 1ULL << 63;
    while (m_montR <= mod) m_montR >>= 1;
    if (m_montR < 2) m_montR = 2;

    // Compute R^-1 mod mod via extended Euclidean
    quint64 r = m_montR, newR = mod;
    quint64 s = 1, newS = 0;
    while (newR != 0) {
        quint64 q = r / newR;
        quint64 tmp = newR; newR = r - q * newR; r = tmp;
        tmp = newS; newS = s - q * newS; s = tmp;
    }
    m_montRInv = (s + mod) % mod;

    // nPrime = -mod^-1 mod R
    // Use mod inverse of mod under R
    quint64 mp = mod, np = m_montR;
    quint64 ms = 0, ns = 1;
    while (mp != 0) {
        quint64 q = np / mp;
        quint64 tmp = mp; mp = np - q * mp; np = tmp;
        tmp = ms; ms = ns - q * ms; ns = tmp;
    }
    m_montNPrime = (m_montR - ns) % m_montR;
}

/* ---- Next power of 2 ---- */

int NumberTheoreticTransform8::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Barrett reduction ---- */

quint64 NumberTheoreticTransform8::barrettReduce(quint64 a) const
{
    quint64 mod = m_config.modulus;
    // q = floor(a / mod) via Barrett estimation
    quint64 q = static_cast<quint64>(
        (static_cast<__uint128_t>(a) * m_barrettMu) >> 64);
    quint64 r = a - q * mod;
    return (r >= mod) ? r - mod : r;
}

/* ---- Montgomery multiplication ---- */

quint64 NumberTheoreticTransform8::montMul(quint64 a, quint64 b) const
{
    quint64 mod = m_config.modulus;
    __uint128_t product = static_cast<__uint128_t>(a) * b;

    // Montgomery reduction
    quint64 t = static_cast<quint64>(product) * m_montNPrime;
    __uint128_t m = static_cast<__uint128_t>(t) * mod;
    quint64 result = static_cast<quint64>((product + m) >> 63);

    return (result >= mod) ? result - mod : result;
}

/* ---- Modular helpers ---- */

quint64 NumberTheoreticTransform8::modAdd(quint64 a, quint64 b) const
{
    quint64 mod = m_config.modulus;
    quint64 s = a + b;
    return (s >= mod || s < a) ? s - mod : s;
}

quint64 NumberTheoreticTransform8::modSub(quint64 a, quint64 b) const
{
    quint64 mod = m_config.modulus;
    return (a >= b) ? a - b : a + mod - b;
}

quint64 NumberTheoreticTransform8::modPow(quint64 base, quint64 exp) const
{
    quint64 mod = m_config.modulus;
    quint64 result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = barrettReduce(
            static_cast<quint64>((static_cast<__uint128_t>(result) * base)));
        base = barrettReduce(
            static_cast<quint64>((static_cast<__uint128_t>(base) * base)));
        exp >>= 1;
    }
    return result;
}

/* ---- Root of unity ---- */

quint64 NumberTheoreticTransform8::rootOfUnity(int n) const
{
    quint64 mod = m_config.modulus;
    quint64 g = m_config.primitiveRoot;
    quint64 order = (mod - 1) / static_cast<quint64>(n);
    return modPow(g, order);
}

/* ---- Forward NTT ---- */

void NumberTheoreticTransform8::forwardNTT(QVector<quint64>& data)
{
    QElapsedTimer timer;
    timer.start();

    quint64 mod = m_config.modulus;
    int n = data.size();

    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }

    // Butterfly stages
    for (int len = 2; len <= n; len <<= 1) {
        quint64 w = rootOfUnity(len);
        quint64 wLen = modPow(w, 1);

        for (int i = 0; i < n; i += len) {
            quint64 wCur = 1;
            for (int j = 0; j < len / 2; ++j) {
                quint64 u = data[i + j];
                quint64 v = barrettReduce(
                    static_cast<quint64>(static_cast<__uint128_t>(data[i + j + len / 2]) * wCur));

                data[i + j] = modAdd(u, v);
                data[i + j + len / 2] = modSub(u, v);

                wCur = barrettReduce(
                    static_cast<quint64>(static_cast<__uint128_t>(wCur) * wLen));
            }
        }
    }

    double elapsed = timer.elapsed();
    m_stats.lastTransformSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit nttDone(n, elapsed);
}

/* ---- Inverse NTT ---- */

void NumberTheoreticTransform8::inverseNTT(QVector<quint64>& data)
{
    QElapsedTimer timer;
    timer.start();

    quint64 mod = m_config.modulus;
    int n = data.size();

    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }

    // Inverse butterfly stages
    for (int len = 2; len <= n; len <<= 1) {
        quint64 wInv = rootOfUnity(len);
        wInv = modPow(wInv, static_cast<quint64>(mod - 2));

        for (int i = 0; i < n; i += len) {
            quint64 wCur = 1;
            for (int j = 0; j < len / 2; ++j) {
                quint64 u = data[i + j];
                quint64 v = barrettReduce(
                    static_cast<quint64>(static_cast<__uint128_t>(data[i + j + len / 2]) * wCur));

                data[i + j] = modAdd(u, v);
                data[i + j + len / 2] = modSub(u, v);

                wCur = barrettReduce(
                    static_cast<quint64>(static_cast<__uint128_t>(wCur) * wInv));
            }
        }
    }

    // Scale by n^-1 mod mod
    quint64 nInv = modPow(static_cast<quint64>(n), mod - 2);
    for (int i = 0; i < n; ++i)
        data[i] = barrettReduce(
            static_cast<quint64>(static_cast<__uint128_t>(data[i]) * nInv));

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Polynomial multiplication ---- */

NumberTheoreticTransform8::ConvolutionResult
NumberTheoreticTransform8::multiply(const QVector<quint64>& a, const QVector<quint64>& b)
{
    QElapsedTimer timer;
    timer.start();

    ConvolutionResult result;
    int len = nextPow2(a.size() + b.size() - 1);

    QVector<quint64> fa(len, 0), fb(len, 0);
    for (int i = 0; i < a.size(); ++i) fa[i] = a[i] % m_config.modulus;
    for (int i = 0; i < b.size(); ++i) fb[i] = b[i] % m_config.modulus;

    forwardNTT(fa);
    forwardNTT(fb);

    for (int i = 0; i < len; ++i)
        fa[i] = barrettReduce(
            static_cast<quint64>(static_cast<__uint128_t>(fa[i]) * fb[i]));

    inverseNTT(fa);

    result.transformSize = len;
    result.actualLength = a.size() + b.size() - 1;
    result.result = fa.mid(0, result.actualLength);

    double elapsed = timer.elapsed();
    emit convolutionDone(result.actualLength, elapsed);
    return result;
}

/* ---- Cyclic convolution ---- */

QVector<quint64> NumberTheoreticTransform8::cyclicConvolve(
    const QVector<quint64>& a, const QVector<quint64>& b)
{
    int n = qMax(a.size(), b.size());
    int size = nextPow2(n);

    QVector<quint64> fa(size, 0), fb(size, 0);
    for (int i = 0; i < a.size(); ++i) fa[i] = a[i] % m_config.modulus;
    for (int i = 0; i < b.size(); ++i) fb[i] = b[i] % m_config.modulus;

    forwardNTT(fa);
    forwardNTT(fb);

    for (int i = 0; i < size; ++i)
        fa[i] = barrettReduce(
            static_cast<quint64>(static_cast<__uint128_t>(fa[i]) * fb[i]));

    inverseNTT(fa);

    // Wrap around for cyclic
    QVector<quint64> result(n, 0);
    for (int i = 0; i < size; ++i)
        result[i % n] = modAdd(result[i % n], fa[i]);
    return result;
}

/* ---- Reset ---- */

void NumberTheoreticTransform8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
