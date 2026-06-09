/**
 * @file RaderFFT9.cpp
 * @brief RaderFFT9 实现
 *
 * 实现Rader FFT：Winograd短卷积模块素数长度与旋转因子预计算缓存。
 */

#include "utils/fft250/RaderFFT9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

RaderFFT9::RaderFFT9(QObject *parent) : QObject(parent) {}
RaderFFT9::~RaderFFT9() = default;

/* ---- Primality test ---- */

bool RaderFFT9::isPrime(int n)
{
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

/* ---- Modular exponentiation ---- */

int RaderFFT9::modPow(int base, int exp, int mod)
{
    int result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = static_cast<int>(
            static_cast<qint64>(result) * base % mod);
        base = static_cast<int>(
            static_cast<qint64>(base) * base % mod);
        exp >>= 1;
    }
    return result;
}

/* ---- Find primitive root modulo p ---- */

int RaderFFT9::primitiveRoot(int p)
{
    if (p == 2) return 1;
    if (p == 3) return 2;

    // Factor p-1
    int phi = p - 1;
    QVector<int> factors;
    int n = phi;
    for (int i = 2; i * i <= n; ++i) {
        if (n % i == 0) {
            factors.append(i);
            while (n % i == 0) n /= i;
        }
    }
    if (n > 1) factors.append(n);

    // Test candidates
    for (int g = 2; g < p; ++g) {
        bool ok = true;
        for (int f : factors) {
            if (modPow(g, phi / f, p) == 1) { ok = false; break; }
        }
        if (ok) return g;
    }
    return -1;
}

/* ---- Build Rader convolution kernel ---- */

void RaderFFT9::buildKernel()
{
    int p = m_plan.p;
    int g = m_plan.g;
    int pm1 = p - 1;

    m_plan.perm.resize(pm1);
    m_plan.twiddleReal.resize(pm1);
    m_plan.twiddleImag.resize(pm1);
    m_plan.convKernelReal.resize(pm1);
    m_plan.convKernelImag.resize(pm1);

    // Build permutation: perm[q] = g^q mod p
    int val = 1;
    for (int q = 0; q < pm1; ++q) {
        m_plan.perm[q] = val;
        val = static_cast<int>(static_cast<qint64>(val) * g % p);
    }

    // Twiddle factors: W_p^(g^q) for q=0..p-2
    for (int q = 0; q < pm1; ++q) {
        double angle = -2.0 * M_PI * m_plan.perm[q] / p;
        m_plan.twiddleReal[q] = qCos(angle);
        m_plan.twiddleImag[q] = qSin(angle);
    }

    // Convolution kernel: W_p^(g^(pm1-q)) reversed
    for (int q = 0; q < pm1; ++q) {
        int exponent = m_plan.perm[(pm1 - q) % pm1];
        double angle = -2.0 * M_PI * exponent / p;
        m_plan.convKernelReal[q] = qCos(angle);
        m_plan.convKernelImag[q] = qSin(angle);
    }
}

/* ---- Circular convolution (direct method) ---- */

void RaderFFT9::circularConvolve(
    const QVector<double>& ar, const QVector<double>& ai,
    const QVector<double>& br, const QVector<double>& bi,
    QVector<double>& outR, QVector<double>& outI) const
{
    int n = ar.size();
    outR.resize(n);
    outI.resize(n);
    for (int k = 0; k < n; ++k) {
        double sr = 0.0, si = 0.0;
        for (int j = 0; j < n; ++j) {
            int idx = (k - j + n) % n;
            sr += ar[j] * br[idx] - ai[j] * bi[idx];
            si += ar[j] * bi[idx] + ai[j] * br[idx];
        }
        outR[k] = sr;
        outI[k] = si;
    }
}

/* ---- Prepare Rader plan ---- */

void RaderFFT9::prepare(int primeSize)
{
    m_size = primeSize;
    m_plan.p = primeSize;
    m_plan.g = primitiveRoot(primeSize);
    m_plan.perm.clear();
    m_plan.twiddleReal.clear();
    m_plan.twiddleImag.clear();
    m_plan.convKernelReal.clear();
    m_plan.convKernelImag.clear();

    if (primeSize == 2) {
        // Special case: trivial DFT
        m_plan.g = 1;
        return;
    }

    buildKernel();
}

/* ---- Forward FFT ---- */

QVector<double> RaderFFT9::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int p = m_size;
    QVector<double> output(p * 2, 0.0);

    if (p <= 1) {
        if (p == 1 && input.size() >= 1) { output[0] = input[0]; }
        return output;
    }

    // DC component: sum of all inputs
    double dcReal = 0.0;
    for (int i = 0; i < p; ++i) {
        double re = (i * 2 < input.size()) ? input[i * 2] : input[i];
        dcReal += re;
    }
    output[0] = dcReal;
    output[1] = 0.0;

    if (p == 2) {
        output[2] = input[0] - input[1];
        output[3] = 0.0;
        return output;
    }

    // Permute input according to primitive root
    int pm1 = p - 1;
    QVector<double> permReal(pm1), permImag(pm1);
    for (int q = 0; q < pm1; ++q) {
        int idx = m_plan.perm[q];
        double re = (idx * 2 < input.size()) ? input[idx * 2] : input[idx];
        permReal[q] = re;
        permImag[q] = 0.0;
    }

    // Circular convolution with precomputed kernel
    QVector<double> convR, convI;
    circularConvolve(permReal, permImag,
                      m_plan.convKernelReal, m_plan.convKernelImag,
                      convR, convI);

    // Map convolution output back to frequency bins
    for (int q = 0; q < pm1; ++q) {
        int k = m_plan.perm[q];
        double angle = -2.0 * M_PI * k / p;
        double wRe = qCos(angle);
        double wIm = qSin(angle);

        // Multiply by twiddle and subtract input contribution
        int idx = m_plan.perm[q];
        double xRe = (idx * 2 < input.size()) ? input[idx * 2] : input[idx];

        output[k * 2] = convR[q] + dcReal * wRe - xRe;
        output[k * 2 + 1] = convI[q] + dcReal * wIm;
    }

    m_stats.transformSize = p;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(p, true, timer.elapsed());
    return output;
}

/* ---- Inverse FFT ---- */

QVector<double> RaderFFT9::inverse(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int p = m_size;
    QVector<double> output(p * 2, 0.0);

    if (p <= 1) {
        if (p == 1 && spectrum.size() >= 2) { output[0] = spectrum[0]; }
        return output;
    }

    // Inverse via conjugate: IFFT(X) = conj(FFT(conj(X))) / N
    QVector<double> conjInput(spectrum.size());
    for (int i = 0; i + 1 < spectrum.size(); i += 2) {
        conjInput[i] = spectrum[i];
        conjInput[i + 1] = -spectrum[i + 1];
    }

    auto fwd = forward(conjInput);
    for (int i = 0; i + 1 < fwd.size(); i += 2) {
        output[i] = fwd[i] / p;
        output[i + 1] = -fwd[i + 1] / p;
    }

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(p, false, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void RaderFFT9::resetStatistics()
{
    m_plan = RaderPlan{};
    m_size = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
