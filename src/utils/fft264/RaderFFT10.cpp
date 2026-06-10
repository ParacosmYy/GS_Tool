/**
 * @file RaderFFT10.cpp
 * @brief RaderFFT10 实现
 *
 * 实现Rader FFT：Bluestein chirp-z任意素数长度DFT卷积计算。
 */

#include "utils/fft264/RaderFFT10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RaderFFT10::RaderFFT10(QObject *parent)
    : QObject(parent) {}

RaderFFT10::~RaderFFT10() = default;

/* ---- Prime check ---- */

bool RaderFFT10::isPrime(int n)
{
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0) return false;
    return true;
}

/* ---- Next power of 2 ---- */

int RaderFFT10::nextPower2(int n) const
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Power modulo ---- */

int RaderFFT10::powMod(int base, int exp, int m) const
{
    int result = 1;
    base %= m;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % m;
        exp >>= 1;
        base = (base * base) % m;
    }
    return result;
}

/* ---- Find primitive root mod p ---- */

int RaderFFT10::primitiveRoot(int p) const
{
    // Factor p-1
    int phi = p - 1;
    QVector<int> factors;
    int temp = phi;
    for (int i = 2; i * i <= temp; ++i) {
        if (temp % i == 0) {
            factors.append(i);
            while (temp % i == 0) temp /= i;
        }
    }
    if (temp > 1) factors.append(temp);

    // Test candidates
    for (int g = 2; g < p; ++g) {
        bool ok = true;
        for (int f : factors) {
            if (powMod(g, phi / f, p) == 1) { ok = false; break; }
        }
        if (ok) return g;
    }
    return 2;
}

/* ---- Direct DFT (O(n^2)) ---- */

QVector<RaderFFT10::Complex> RaderFFT10::directDFT(const QVector<Complex>& input) const
{
    int n = input.size();
    QVector<Complex> output(n);
    for (int k = 0; k < n; ++k) {
        Complex sum;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / n;
            Complex tw = {qCos(angle), qSin(angle)};
            sum = sum + input[i] * tw;
        }
        output[k] = sum;
    }
    return output;
}

/* ---- Radix-2 FFT ---- */

QVector<RaderFFT10::Complex> RaderFFT10::fftPower2(const QVector<Complex>& input) const
{
    int n = input.size();
    if (n <= 1) return input;

    QVector<Complex> even(n / 2), odd(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        even[i] = input[2 * i];
        odd[i] = input[2 * i + 1];
    }

    auto fe = fftPower2(even);
    auto fo = fftPower2(odd);

    QVector<Complex> result(n);
    for (int k = 0; k < n / 2; ++k) {
        double angle = -2.0 * M_PI * k / n;
        Complex tw = {qCos(angle), qSin(angle)};
        result[k] = fe[k] + tw * fo[k];
        result[k + n / 2] = fe[k] - tw * fo[k];
    }
    return result;
}

/* ---- Rader DFT for prime length ---- */

QVector<RaderFFT10::Complex> RaderFFT10::raderDFT(const QVector<Complex>& input) const
{
    int p = input.size();
    if (p <= 1) return input;

    // Find primitive root
    int g = primitiveRoot(p);

    // Build permuted sequences using generator powers
    QVector<Complex> a(p - 1), b(p - 1);
    for (int i = 0; i < p - 1; ++i) {
        int gi = powMod(g, i, p);
        int gni = powMod(g, p - 1 - i, p);
        a[i] = input[gi];
        double angle = -2.0 * M_PI * gni / p;
        b[i] = {qCos(angle), qSin(angle)};
    }

    // Compute convolution via FFT (pad to power of 2)
    int padLen = nextPower2(2 * (p - 1) - 1);

    QVector<Complex> aPad(padLen), bPad(padLen);
    for (int i = 0; i < p - 1; ++i) { aPad[i] = a[i]; bPad[i] = b[i]; }

    auto aFft = fftPower2(aPad);
    auto bFft = fftPower2(bPad);

    QVector<Complex> cFft(padLen);
    for (int i = 0; i < padLen; ++i)
        cFft[i] = aFft[i] * bFft[i];

    auto conv = fftPower2(cFft);

    // Scale by 1/N for inverse
    QVector<Complex> result(p);
    result[0] = input[0];
    for (int i = 1; i < p; ++i) result[0] = result[0] + input[i];

    for (int k = 0; k < p - 1; ++k) {
        int gk = powMod(g, k, p);
        Complex val = conv[k];
        val.re /= padLen;
        val.im /= padLen;
        result[gk] = val + input[0];
    }
    return result;
}

/* ---- Bluestein chirp-z DFT ---- */

QVector<RaderFFT10::Complex> RaderFFT10::bluesteinDFT(const QVector<Complex>& input) const
{
    int n = input.size();
    if (n <= 1) return input;

    // Build chirp sequence: W^(k^2/2) where W = e^{-2*pi*i/N}
    int padLen = nextPower2(2 * n - 1);

    QVector<Complex> chirp(padLen);
    for (int i = 0; i < n; ++i) {
        double phase = M_PI * i * i / n;
        chirp[i] = {qCos(phase), qSin(phase)};
        if (i > 0 && i < padLen - n + 1)
            chirp[padLen - n + i] = {qCos(phase), qSin(phase)};
    }

    // Build input sequence with chirp multiplication
    QVector<Complex> a(padLen);
    for (int i = 0; i < n; ++i) {
        double phase = M_PI * i * i / n;
        Complex conjChirp = {qCos(phase), -qSin(phase)};
        a[i] = input[i] * conjChirp;
    }

    // Convolve via FFT
    auto aFft = fftPower2(a);
    auto cFft = fftPower2(chirp);

    QVector<Complex> prod(padLen);
    for (int i = 0; i < padLen; ++i)
        prod[i] = aFft[i] * cFft[i];

    auto convFull = fftPower2(prod);

    // Extract result and apply final chirp
    QVector<Complex> result(n);
    for (int i = 0; i < n; ++i) {
        convFull[i].re /= padLen;
        convFull[i].im /= padLen;
        double phase = M_PI * i * i / n;
        Complex conjChirp = {qCos(phase), -qSin(phase)};
        result[i] = convFull[i] * conjChirp;
    }
    return result;
}

/* ---- Forward transform ---- */

QVector<RaderFFT10::Complex> RaderFFT10::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<Complex> cx(n);
    for (int i = 0; i < n; ++i) cx[i] = {input[i], 0.0};

    QVector<Complex> result;
    int padded = n;
    if (n <= 64) {
        result = directDFT(cx);
    } else if (isPrime(n)) {
        result = raderDFT(cx);
        padded = nextPower2(2 * (n - 1) - 1);
    } else {
        result = bluesteinDFT(cx);
        padded = nextPower2(2 * n - 1);
    }

    double elapsed = timer.elapsed();
    m_stats.transformSize = n;
    m_stats.paddedSize = padded;
    m_stats.isPrime = isPrime(n);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformComputed(n, padded, elapsed);
    return result;
}

/* ---- Inverse transform ---- */

QVector<double> RaderFFT10::inverseTransform(const QVector<Complex>& spectrum)
{
    int n = spectrum.size();
    // Conjugate, forward DFT, conjugate, scale
    QVector<Complex> conjInput(n);
    for (int i = 0; i < n; ++i) conjInput[i] = conj(spectrum[i]);

    QVector<Complex> transformed;
    if (n <= 64) transformed = directDFT(conjInput);
    else if (isPrime(n)) transformed = raderDFT(conjInput);
    else transformed = bluesteinDFT(conjInput);

    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = conj(transformed[i]).re / n;
    }
    return result;
}

/* ---- Reset ---- */

void RaderFFT10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
