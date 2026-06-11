/**
 * @file MixedRadixFFT11.cpp
 * @brief MixedRadixFFT11 实现
 *
 * 实现混合基FFT：自动素因子分解与自排序原地蝶形运算支持任意复合长度。
 */

#include "utils/fft289/MixedRadixFFT11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MixedRadixFFT11::MixedRadixFFT11(QObject *parent)
    : QObject(parent) {}

MixedRadixFFT11::~MixedRadixFFT11() = default;

/* ---- Factorize n into small primes {2, 3, 5, 7} ---- */

QVector<int> MixedRadixFFT11::factorize(int n) const
{
    QVector<int> factors;
    if (n <= 1) return factors;
    int primes[] = {2, 3, 5, 7};
    for (int p : primes) {
        while (n % p == 0) { factors.append(p); n /= p; }
    }
    // Remaining prime factor
    if (n > 1) factors.append(n);
    return factors;
}

/* ---- Check if n is product of small primes ---- */

bool MixedRadixFFT11::isSmallComposite(int n) const
{
    int primes[] = {2, 3, 5, 7};
    for (int p : primes) {
        while (n % p == 0) n /= p;
    }
    return n == 1;
}

/* ---- Next composite number >= n ---- */

int MixedRadixFFT11::nextComposite(int n) const
{
    while (!isSmallComposite(n)) ++n;
    return n;
}

/* ---- Digit-reversed index ---- */

int MixedRadixFFT11::digitReverse(int idx, int n, const QVector<int>& factors) const
{
    int reversed = 0;
    int remain = n;
    for (int f : factors) {
        remain /= f;
        reversed = reversed * f + (idx % f);
        idx /= f;
    }
    return reversed;
}

/* ---- Twiddle factor ---- */

void MixedRadixFFT11::twiddle(int k, int n, double sign, double& wRe, double& wIm) const
{
    double angle = sign * 2.0 * M_PI * k / n;
    wRe = qCos(angle);
    wIm = qSin(angle);
}

/* ---- Small prime DFT (direct) ---- */

void MixedRadixFFT11::smallPrimeDFT(QVector<double>& re, QVector<double>& im,
                                      int start, int stride, int p, double sign) const
{
    if (p == 2) {
        // Radix-2 butterfly
        double aRe = re[start], aIm = im[start];
        double bRe = re[start + stride], bIm = im[start + stride];
        re[start] = aRe + bRe; im[start] = aIm + bIm;
        re[start + stride] = aRe - bRe; im[start + stride] = aIm - bIm;
    } else if (p == 3) {
        // Radix-3 butterfly
        double t = sign * 2.0 * M_PI / 3.0;
        double c1Re = qCos(t), c1Im = qSin(t);
        double c2Re = qCos(2 * t), c2Im = qSin(2 * t);
        double x0Re = re[start], x0Im = im[start];
        double x1Re = re[start + stride], x1Im = im[start + stride];
        double x2Re = re[start + 2 * stride], x2Im = im[start + 2 * stride];
        double s1Re = x1Re + x2Re, s1Im = x1Im + x2Im;
        double d1Re = x1Re - x2Re, d1Im = x1Im - x2Im;
        re[start] = x0Re + s1Re; im[start] = x0Im + s1Im;
        re[start + stride] = x0Re + c1Re * s1Re - c1Im * s1Im + c1Im * d1Re + c1Re * d1Im;
        im[start + stride] = x0Im + c1Re * s1Im + c1Im * s1Re - c1Re * d1Re + c1Im * d1Im;
        re[start + 2 * stride] = x0Re + c2Re * s1Re - c2Im * s1Im - c2Im * d1Re - c2Re * d1Im;
        im[start + 2 * stride] = x0Im + c2Re * s1Im + c2Im * s1Re + c2Re * d1Re - c2Im * d1Im;
    } else {
        // General DFT for prime p (Brute force for small p)
        QVector<double> tmpRe(p, 0.0), tmpIm(p, 0.0);
        for (int k = 0; k < p; ++k) {
            for (int j = 0; j < p; ++j) {
                double angle = sign * 2.0 * M_PI * k * j / p;
                double wR = qCos(angle), wI = qSin(angle);
                tmpRe[k] += re[start + j * stride] * wR - im[start + j * stride] * wI;
                tmpIm[k] += re[start + j * stride] * wI + im[start + j * stride] * wR;
            }
        }
        for (int k = 0; k < p; ++k) {
            re[start + k * stride] = tmpRe[k];
            im[start + k * stride] = tmpIm[k];
        }
    }
}

/* ---- Core mixed-radix FFT ---- */

void MixedRadixFFT11::mixedRadixCore(QVector<double>& re, QVector<double>& im,
                                       int n, const QVector<int>& factors, bool inverse)
{
    double sign = inverse ? 1.0 : -1.0;

    // Step 1: Digit-reverse permutation (self-sorting)
    for (int i = 0; i < n; ++i) {
        int j = digitReverse(i, n, factors);
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    // Step 2: Iterative butterfly across factors
    int stride = 1;
    int L = 1; // Product of factors processed so far
    for (int f : factors) {
        int m = n / (L * f);
        for (int k = 0; k < m; ++k) {
            int base = k * L * f;
            for (int s = 0; s < L; ++s) {
                int pos = base + s;
                // Apply twiddle factors before DFT
                for (int j = 0; j < f; ++j) {
                    int idx = pos + j * L;
                    double wRe, wIm;
                    twiddle(j * s, L * f, sign, wRe, wIm);
                    double r = re[idx], im_ = im[idx];
                    re[idx] = r * wRe - im_ * wIm;
                    im[idx] = r * wIm + im_ * wRe;
                }
                smallPrimeDFT(re, im, pos, L, f, sign);
            }
        }
        L *= f;
        stride *= f;
    }

    // Inverse scaling
    if (inverse) {
        for (int i = 0; i < n; ++i) { re[i] /= n; im[i] /= n; }
    }
}

/* ---- Forward FFT (real input) ---- */

MixedRadixFFT11::FFTResult MixedRadixFFT11::forward(const QVector<double>& input)
{
    int n = nextComposite(input.size());
    QVector<double> re(n, 0.0), im(n, 0.0);
    for (int i = 0; i < input.size(); ++i) re[i] = input[i];
    return forwardComplex(re, im);
}

/* ---- Forward FFT (complex input) ---- */

MixedRadixFFT11::FFTResult MixedRadixFFT11::forwardComplex(
    const QVector<double>& real, const QVector<double>& imag)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMax(real.size(), imag.size());
    n = nextComposite(n);

    QVector<double> re(n, 0.0), im(n, 0.0);
    for (int i = 0; i < n; ++i) {
        re[i] = (i < real.size()) ? real[i] : 0.0;
        im[i] = (i < imag.size()) ? imag[i] : 0.0;
    }

    QVector<int> factors = factorize(n);
    mixedRadixCore(re, im, n, factors, false);

    FFTResult result;
    result.real = re;
    result.imag = im;
    result.n = n;
    result.factors = factors;
    result.magnitude.resize(n);
    result.phase.resize(n);
    for (int i = 0; i < n; ++i) {
        result.magnitude[i] = qSqrt(re[i] * re[i] + im[i] * im[i]);
        result.phase[i] = qAtan2(im[i], re[i]);
    }

    double elapsed = timer.elapsed();
    m_stats.lastN = n;
    m_stats.numFactors = factors.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fftDone(n, factors.size(), elapsed);
    return result;
}

/* ---- Inverse FFT ---- */

MixedRadixFFT11::FFTResult MixedRadixFFT11::inverse(
    const QVector<double>& real, const QVector<double>& imag)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMax(real.size(), imag.size());
    n = nextComposite(n);

    QVector<double> re(n, 0.0), im(n, 0.0);
    for (int i = 0; i < n; ++i) {
        re[i] = (i < real.size()) ? real[i] : 0.0;
        im[i] = (i < imag.size()) ? imag[i] : 0.0;
    }

    QVector<int> factors = factorize(n);
    mixedRadixCore(re, im, n, factors, true);

    FFTResult result;
    result.real = re;
    result.imag = im;
    result.n = n;
    result.factors = factors;
    result.magnitude.resize(n);
    result.phase.resize(n);
    for (int i = 0; i < n; ++i) {
        result.magnitude[i] = qSqrt(re[i] * re[i] + im[i] * im[i]);
        result.phase[i] = qAtan2(im[i], re[i]);
    }

    double elapsed = timer.elapsed();
    m_stats.lastN = n;
    m_stats.numFactors = factors.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Reset ---- */

void MixedRadixFFT11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
