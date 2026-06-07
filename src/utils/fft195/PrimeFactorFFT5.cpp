/**
 * @file PrimeFactorFFT5.cpp
 * @brief PrimeFactorFFT5 实现
 *
 * 实现素因子FFT：Rader算法集成、素因子分解、任意长度频域计算。
 */

#include "utils/fft195/PrimeFactorFFT5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

PrimeFactorFFT5::PrimeFactorFFT5(QObject *parent) : QObject(parent) {}
PrimeFactorFFT5::~PrimeFactorFFT5() = default;

/* ---- Flat <-> Complex conversion ---- */

QVector<double> PrimeFactorFFT5::toFlat(const QVector<QVector<double>>& c)
{
    QVector<double> f(c.size() * 2);
    for (int i = 0; i < c.size(); ++i) { f[2*i] = c[i][0]; f[2*i+1] = c[i][1]; }
    return f;
}

QVector<QVector<double>> PrimeFactorFFT5::fromFlat(const QVector<double>& f)
{
    int n = f.size() / 2;
    QVector<QVector<double>> c(n, {0.0, 0.0});
    for (int i = 0; i < n; ++i) { c[i][0] = f[2*i]; c[i][1] = f[2*i+1]; }
    return c;
}

/* ---- Primality test ---- */

bool PrimeFactorFFT5::isPrime(int n) const
{
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0) return false;
    return true;
}

/* ---- Power mod ---- */

int PrimeFactorFFT5::powMod(int base, int exp, int mod) const
{
    long long result = 1, b = base % mod;
    while (exp > 0) {
        if (exp & 1) result = (result * b) % mod;
        b = (b * b) % mod;
        exp >>= 1;
    }
    return static_cast<int>(result);
}

/* ---- Primitive root ---- */

int PrimeFactorFFT5::primitiveRoot(int p) const
{
    for (int g = 2; g < p; ++g) {
        bool ok = true;
        for (int d = 2; d < p - 1; ++d) {
            if ((p - 1) % d == 0 && powMod(g, d, p) == 1) { ok = false; break; }
        }
        if (ok) return g;
    }
    return 2;
}

/* ---- Factorize into coprime factors ---- */

QVector<int> PrimeFactorFFT5::factorize(int n) const
{
    QVector<int> factors;
    // Extract small factors: 2, 3, 4, 5, 7, 8, 9, 11, 13, 16
    static const int small[] = {16, 13, 11, 9, 8, 7, 5, 4, 3, 2};
    for (int f : small) {
        while (n % f == 0) { factors.append(f); n /= f; }
    }
    if (n > 1) factors.append(n);  // remaining prime
    return factors;
}

/* ---- Small-N DFT kernel ---- */

QVector<QVector<double>> PrimeFactorFFT5::smallDFT(
    const QVector<QVector<double>>& x, int N, bool inverse) const
{
    QVector<QVector<double>> X(N, {0.0, 0.0});
    double sign = inverse ? 1.0 : -1.0;
    double scale = inverse ? (1.0 / N) : 1.0;

    for (int k = 0; k < N; ++k) {
        double sr = 0.0, si = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = sign * 2.0 * M_PI * k * n / N;
            double wR = qCos(angle), wI = qSin(angle);
            sr += x[n][0] * wR - x[n][1] * wI;
            si += x[n][0] * wI + x[n][1] * wR;
        }
        X[k] = {sr * scale, si * scale};
    }
    return X;
}

/* ---- Rader's algorithm for prime-length FFT ---- */

QVector<QVector<double>> PrimeFactorFFT5::raderFFT(
    const QVector<QVector<double>>& x) const
{
    int p = x.size();
    if (p == 1) return {{x[0][0], x[0][1]}};
    if (p == 2) return smallDFT(x, 2, false);

    int g = primitiveRoot(p);
    int pm1 = p - 1;

    // Permute input using primitive root
    QVector<QVector<double>> a(pm1, {0.0, 0.0});
    QVector<QVector<double>> b(pm1, {0.0, 0.0});
    for (int q = 0; q < pm1; ++q) {
        int idx = powMod(g, q, p);
        a[q] = x[idx];
        double angle = 2.0 * M_PI * powMod(g, pm1 - q, p) / p;
        b[q] = {qCos(angle), -qSin(angle)};
    }

    // Convolution via DFT (size = next power of 2 >= 2*(p-1)-1)
    int convLen = 1;
    while (convLen < 2 * pm1 - 1) convLen *= 2;

    // Pad and compute DFT of both sequences
    QVector<QVector<double>> aPad(convLen, {0.0, 0.0});
    QVector<QVector<double>> bPad(convLen, {0.0, 0.0});
    for (int i = 0; i < pm1; ++i) { aPad[i] = a[i]; bPad[i] = b[i]; }

    QVector<QVector<double>> AF = smallDFT(aPad, convLen, false);
    QVector<QVector<double>> BF = smallDFT(bPad, convLen, false);

    // Pointwise multiply
    QVector<QVector<double>> CF(convLen, {0.0, 0.0});
    for (int i = 0; i < convLen; ++i) {
        CF[i] = {AF[i][0] * BF[i][0] - AF[i][1] * BF[i][1],
                 AF[i][0] * BF[i][1] + AF[i][1] * BF[i][0]};
    }

    // Inverse DFT
    QVector<QVector<double>> C = smallDFT(CF, convLen, true);

    // Extract result
    QVector<QVector<double>> result(p, {0.0, 0.0});
    // x[0] contribution
    double s0r = 0.0, s0i = 0.0;
    for (int n = 0; n < p; ++n) { s0r += x[n][0]; s0i += x[n][1]; }
    result[0] = {s0r, s0i};

    for (int k = 1; k < p; ++k) {
        int q = 0;
        for (; q < pm1; ++q) {
            if (powMod(g, q, p) == k) break;
        }
        result[k] = {x[0][0] + C[q % pm1][0], x[0][1] + C[q % pm1][1]};
    }
    return result;
}

/* ---- PFA core ---- */

QVector<QVector<double>> PrimeFactorFFT5::pfaCore(
    const QVector<QVector<double>>& x,
    const QVector<int>& factors) const
{
    int N = x.size();
    QVector<QVector<double>> data = x;

    // Apply small DFTs along each factor dimension
    int stride = 1;
    for (int f : factors) {
        int blocks = N / (f * stride);
        for (int b = 0; b < blocks; ++b) {
            for (int s = 0; s < stride; ++s) {
                QVector<QVector<double>> col(f, {0.0, 0.0});
                for (int r = 0; r < f; ++r)
                    col[r] = data[b * f * stride + r * stride + s];
                col = smallDFT(col, f, false);
                for (int r = 0; r < f; ++r)
                    data[b * f * stride + r * stride + s] = col[r];
            }
        }
        stride *= f;
    }
    return data;
}

/* ---- Forward FFT ---- */

QVector<double> PrimeFactorFFT5::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    auto cx = fromFlat(input);
    int N = cx.size();
    if (N == 0) return input;

    QVector<int> factors = factorize(N);
    bool usedRader = false;

    // Check if any factor is a large prime -> use Rader
    for (int f : factors) {
        if (f > 5 && isPrime(f)) { usedRader = true; break; }
    }

    QVector<QVector<double>> result;
    if (isPrime(N)) {
        result = raderFFT(cx);
    } else {
        result = pfaCore(cx, factors);
    }

    m_stats.totalTransforms++;
    m_stats.lastSize = N;
    m_stats.numPrimeFactors = factors.size();
    m_stats.usedRader = usedRader;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, factors.size(), usedRader, timer.elapsed());
    return toFlat(result);
}

/* ---- Inverse FFT ---- */

QVector<double> PrimeFactorFFT5::inverse(const QVector<double>& spectrum)
{
    // Conjugate, forward, conjugate, scale
    auto cx = fromFlat(spectrum);
    int N = cx.size();
    for (int i = 0; i < N; ++i) cx[i][1] = -cx[i][1];

    QVector<int> factors = factorize(N);
    QVector<QVector<double>> result;
    if (isPrime(N)) {
        result = raderFFT(cx);
    } else {
        result = pfaCore(cx, factors);
    }

    for (int i = 0; i < N; ++i) {
        result[i][0] /= N;
        result[i][1] = -result[i][1] / N;
    }
    return toFlat(result);
}

/* ---- Reset ---- */

void PrimeFactorFFT5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
