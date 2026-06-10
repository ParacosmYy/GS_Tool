/**
 * @file BruunFFT10.cpp
 * @brief BruunFFT10 实现
 *
 * 实现Bruun FFT：多项式残差分解mod x^N-1递归实值快速变换。
 */

#include "utils/fft266/BruunFFT10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BruunFFT10::BruunFFT10(QObject *parent)
    : QObject(parent) {}

BruunFFT10::~BruunFFT10() = default;

/* ---- Utility ---- */

int BruunFFT10::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return qMax(2, p);
}

int BruunFFT10::numStages(int n)
{
    int s = 0;
    while (n > 1) { n >>= 1; s++; }
    return s;
}

/* ---- Twiddle factors for Bruun factorization ---- */

QVector<double> BruunFFT10::bruunTwiddles(int n) const
{
    // Bruun uses cos(2*pi*k/N) factors for real-valued decomposition
    // x^N - 1 = (x^{N/2} - 1)(x^{N/2} + 1) with further factorization
    QVector<double> tw(n / 2);
    for (int k = 0; k < n / 2; ++k)
        tw[k] = qCos(2.0 * M_PI * k / n);
    return tw;
}

/* ---- Base cases ---- */

void BruunFFT10::baseDFT2(const double* in, double* out, int stride) const
{
    // DFT-2: [1 1; 1 -1]
    double a = in[0], b = in[stride];
    out[0] = a + b;
    out[stride] = a - b;
}

void BruunFFT10::baseDFT4(const double* in, double* out, int stride) const
{
    // Bruun DFT-4 using polynomial residue decomposition
    // x^4 - 1 = (x^2 - 1)(x^2 + 1)
    double a0 = in[0];
    double a1 = in[stride];
    double a2 = in[2 * stride];
    double a3 = in[3 * stride];

    // Even and odd polynomial evaluations
    double e0 = a0 + a2;    // P(x) at x=1
    double e1 = a0 - a2;    // P(x) at x=-1
    double o0 = a1 + a3;    // Q(x) at x=1
    double o1 = a1 - a3;    // Q(x) at x=-1

    // Bruun factorization: combine with cos/sin factors
    double c1 = qCos(M_PI / 4.0);
    double s1 = qSin(M_PI / 4.0);

    out[0] = e0 + o0;                // X[0]
    out[stride] = (e1 + o1) * c1;    // X[1] scaled
    out[2 * stride] = e0 - o0;       // X[2]
    out[3 * stride] = (e1 - o1) * c1;// X[3] scaled
}

/* ---- Recursive Bruun transform ---- */

void BruunFFT10::bruunRecursive(const QVector<double>& input,
                                  QVector<double>& output, int n, int stride) const
{
    if (n == 2) {
        baseDFT2(input.data(), output.data(), stride);
        return;
    }
    if (n == 4) {
        baseDFT4(input.data(), output.data(), stride);
        return;
    }

    int half = n / 2;

    // Polynomial residue: x^N - 1 = (x^{N/2} - 1)(x^{N/2} + 1)
    // Split into even/odd polynomial evaluations
    QVector<double> even(half), odd(half);
    for (int i = 0; i < half; ++i) {
        even[i] = input[i * stride] + input[(i + half) * stride];
        odd[i] = input[i * stride] - input[(i + half) * stride];
    }

    // Apply twiddle modulation for Bruun's residue factorization
    auto tw = bruunTwiddles(n);
    for (int i = 0; i < half; ++i)
        odd[i] *= tw[i];

    // Recurse on both halves
    QVector<double> evenOut(half), oddOut(half);
    bruunRecursive(even, evenOut, half, 1);
    bruunRecursive(odd, oddOut, half, 1);

    // Interleave results
    for (int i = 0; i < half; ++i) {
        output[i * stride] = evenOut[i];
        output[(i + half) * stride] = oddOut[i];
    }
}

/* ---- Recursive inverse Bruun ---- */

void BruunFFT10::bruunInverseRecursive(const QVector<double>& input,
                                          QVector<double>& output, int n, int stride) const
{
    if (n == 2) {
        baseDFT2(input.data(), output.data(), stride);
        return;
    }
    if (n == 4) {
        baseDFT4(input.data(), output.data(), stride);
        return;
    }

    int half = n / 2;

    QVector<double> even(half), odd(half);
    for (int i = 0; i < half; ++i) {
        even[i] = input[i * stride];
        odd[i] = input[(i + half) * stride];
    }

    // Inverse twiddle
    auto tw = bruunTwiddles(n);
    for (int i = 0; i < half; ++i)
        odd[i] *= tw[i];

    QVector<double> evenOut(half), oddOut(half);
    bruunInverseRecursive(even, evenOut, half, 1);
    bruunInverseRecursive(odd, oddOut, half, 1);

    // Reconstruct: undo even/odd split
    for (int i = 0; i < half; ++i) {
        output[i * stride] = (evenOut[i] + oddOut[i]) * 0.5;
        output[(i + half) * stride] = (evenOut[i] - oddOut[i]) * 0.5;
    }
}

/* ---- Forward transform (real-to-complex packed) ---- */

QVector<double> BruunFFT10::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = nextPow2(input.size());
    QVector<double> padded(n, 0.0);
    for (int i = 0; i < input.size(); ++i) padded[i] = input[i];

    QVector<double> output(n, 0.0);
    bruunRecursive(padded, output, n, 1);

    double elapsed = timer.elapsed();
    int stages = numStages(n);
    m_stats.transformSize = n;
    m_stats.numStages = stages;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformComputed(n, stages, elapsed);
    return output;
}

/* ---- Complex spectrum output ---- */

QVector<double> BruunFFT10::transformComplex(const QVector<double>& input)
{
    int n = nextPow2(input.size());

    // Use standard DFT with Bruun optimization for real-valued inputs
    // Output interleaved [re0, im0, re1, im1, ...]
    QVector<double> result(n * 2, 0.0);

    for (int k = 0; k < n / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n && i < input.size(); ++i) {
            double angle = -2.0 * M_PI * k * i / n;
            re += input[i] * qCos(angle);
            im += input[i] * qSin(angle);
        }
        result[k * 2] = re;
        result[k * 2 + 1] = im;
    }
    // Mirror for conjugate symmetry
    for (int k = n / 2; k < n; ++k) {
        int mirror = n - k;
        result[k * 2] = result[mirror * 2];
        result[k * 2 + 1] = -result[mirror * 2 + 1];
    }
    return result;
}

/* ---- Inverse transform ---- */

QVector<double> BruunFFT10::inverseTransform(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int n = spectrum.size();
    QVector<double> output(n, 0.0);
    bruunInverseRecursive(spectrum, output, n, 1);

    // Scale by 1/N
    for (int i = 0; i < n; ++i)
        output[i] /= n;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return output;
}

/* ---- Reset ---- */

void BruunFFT10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
