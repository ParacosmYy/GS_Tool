/**
 * @file SplitRadixFFT9.cpp
 * @brief SplitRadixFFT9 实现
 *
 * 实现分裂基数FFT：原位比特反转与共轭对称实值输入变换。
 */

#include "utils/fft262/SplitRadixFFT9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplitRadixFFT9::SplitRadixFFT9(QObject *parent)
    : QObject(parent) {}
SplitRadixFFT9::~SplitRadixFFT9() = default;

/* ---- Utility helpers ---- */

int SplitRadixFFT9::log2Int(int n) const
{
    int bits = 0;
    while ((1 << bits) < n) ++bits;
    return bits;
}

int SplitRadixFFT9::reverseBits(int val, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (val & 1);
        val >>= 1;
    }
    return result;
}

/* ---- In-place bit-reversal permutation ---- */

void SplitRadixFFT9::bitReversePermute(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    int bits = log2Int(n);
    for (int i = 0; i < n; ++i) {
        int j = reverseBits(i, bits);
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
}

/* ---- Split-radix butterfly core ---- */

void SplitRadixFFT9::splitRadixCore(QVector<double>& re, QVector<double>& im,
                                      int n, bool inverse) const
{
    // L-shaped butterfly: split n into one n/2 and two n/4 sub-transforms
    if (n <= 4) {
        // Base case: small DFT
        for (int k = 0; k < n; ++k) {
            double sumRe = 0.0, sumIm = 0.0;
            for (int i = 0; i < n; ++i) {
                double angle = (inverse ? 2.0 : -2.0) * M_PI * k * i / n;
                sumRe += re[i] * qCos(angle) - im[i] * qSin(angle);
                sumIm += re[i] * qSin(angle) + im[i] * qCos(angle);
            }
            // Store temporarily (in-place with copy)
        }
        return;
    }

    int n2 = n / 2;
    int n4 = n / 4;

    // Split-radix decomposition:
    // X[k]       = E[k] + W^k * O1[k] + W^(3k) * O3[k]
    // X[k + n/4] = E[k] - j*W^k * O1[k] + j*W^(3k) * O3[k]
    // etc.
    double sign = inverse ? 1.0 : -1.0;

    for (int k = 0; k < n4; ++k) {
        // Twiddle factors
        double angle1 = sign * 2.0 * M_PI * k / n;
        double angle3 = sign * 2.0 * M_PI * 3.0 * k / n;
        double w1Re = qCos(angle1), w1Im = qSin(angle1);
        double w3Re = qCos(angle3), w3Im = qSin(angle3);

        // Gather sub-outputs
        int idx0 = k, idx1 = k + n4, idx2 = k + n2, idx3 = k + n2 + n4;

        double t0Re = re[idx0], t0Im = im[idx0];
        double t1Re = re[idx1], t1Im = im[idx1];
        double t2Re = re[idx2], t2Im = im[idx2];
        double t3Re = re[idx3], t3Im = im[idx3];

        // Apply twiddle to odd-index parts
        double o1Re = w1Re * t2Re - w1Im * t2Im;
        double o1Im = w1Re * t2Im + w1Im * t2Re;
        double o3Re = w3Re * t3Re - w3Im * t3Im;
        double o3Im = w3Re * t3Im + w3Im * t3Re;

        // L-shaped butterfly combine
        re[idx0] = t0Re + t1Re + o1Re + o3Re;
        im[idx0] = t0Im + t1Im + o1Im + o3Im;

        re[idx1] = t0Re - t1Re - o1Im + o3Im;
        im[idx1] = t0Im - t1Im + o1Re - o3Re;

        re[idx2] = t0Re + t1Re - o1Re - o3Re;
        im[idx2] = t0Im + t1Im - o1Im - o3Im;

        re[idx3] = t0Re - t1Re + o1Im - o3Im;
        im[idx3] = t0Im - t1Im - o1Re + o3Re;
    }
}

/* ---- Pack real input into half-size complex ---- */

void SplitRadixFFT9::packReal(const QVector<double>& input,
                                QVector<double>& re, QVector<double>& im) const
{
    int n = input.size();
    re.resize(n / 2);
    im.resize(n / 2);
    // Pack even indices as real, odd as imaginary
    for (int i = 0; i < n / 2; ++i) {
        re[i] = input[2 * i];
        im[i] = input[2 * i + 1];
    }
}

/* ---- Unpack half-complex spectrum ---- */

QVector<double> SplitRadixFFT9::unpackSpectrum(const QVector<double>& re,
                                                  const QVector<double>& im) const
{
    int halfN = re.size();
    int fullBins = halfN + 1;
    QVector<double> spectrum(2 * fullBins);

    // DC component
    spectrum[0] = re[0] + im[0];   // real
    spectrum[1] = 0.0;              // imaginary (always 0 for DC)

    // Nyquist
    spectrum[2 * halfN] = re[0] - im[0];
    spectrum[2 * halfN + 1] = 0.0;

    // Hermitian symmetry: use conjugate pairs
    for (int k = 1; k < halfN; ++k) {
        double xkRe = 0.5 * (re[k] + re[halfN - k]);
        double xkIm = 0.5 * (im[k] - im[halfN - k]);
        double xkNRe = 0.5 * (im[k] + im[halfN - k]);
        double xkNIm = 0.5 * (re[halfN - k] - re[k]);

        spectrum[2 * k] = xkRe + xkNRe;
        spectrum[2 * k + 1] = xkIm + xkNIm;
    }
    return spectrum;
}

/* ---- Pre-process for inverse RFFT ---- */

void SplitRadixFFT9::preprocessInverse(const QVector<double>& spectrum,
                                          QVector<double>& re, QVector<double>& im) const
{
    int fullBins = spectrum.size() / 2;
    int halfN = fullBins - 1;
    re.resize(halfN);
    im.resize(halfN);

    // Reconstruct from conjugate-symmetric full spectrum
    re[0] = 0.5 * (spectrum[0] + spectrum[2 * halfN]);
    im[0] = 0.5 * (spectrum[0] - spectrum[2 * halfN]);

    for (int k = 1; k < halfN; ++k) {
        re[k] = spectrum[2 * k];
        im[k] = spectrum[2 * k + 1];
    }
}

/* ---- Forward real-valued FFT ---- */

QVector<double> SplitRadixFFT9::forwardReal(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n < 4 || (n & (n - 1)) != 0) return {};  // Must be power of 2

    // Pack into half-size complex FFT
    QVector<double> re, im;
    packReal(input, re, im);

    // Bit-reverse and compute half-size FFT
    bitReversePermute(re, im);
    int halfN = n / 2;
    // Iterative split-radix on half-size
    for (int m = 2; m <= halfN; m *= 2) {
        for (int start = 0; start < halfN; start += m) {
            splitRadixCore(re, im, m, false);
        }
    }

    // Unpack to full conjugate-symmetric spectrum
    QVector<double> result = unpackSpectrum(re, im);

    double elapsed = timer.elapsed();
    m_stats.transformSize = n;
    m_stats.numForwardTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformCompleted(n, true, false, elapsed);
    return result;
}

/* ---- Inverse real-valued FFT ---- */

QVector<double> SplitRadixFFT9::inverseReal(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> re, im;
    preprocessInverse(spectrum, re, im);

    int halfN = re.size();
    bitReversePermute(re, im);
    for (int m = 2; m <= halfN; m *= 2) {
        for (int start = 0; start < halfN; start += m) {
            splitRadixCore(re, im, m, true);
        }
    }

    // Unpack: even=output_real, odd=output_imag
    int n = halfN * 2;
    QVector<double> output(n);
    for (int i = 0; i < halfN; ++i) {
        output[2 * i] = re[i] / halfN;
        output[2 * i + 1] = im[i] / halfN;
    }

    double elapsed = timer.elapsed();
    m_stats.numInverseTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformCompleted(n, true, true, elapsed);
    return output;
}

/* ---- General complex FFT ---- */

QVector<double> SplitRadixFFT9::forwardComplex(const QVector<double>& interleaved)
{
    QElapsedTimer timer;
    timer.start();

    int n = interleaved.size() / 2;
    if (n < 2 || (n & (n - 1)) != 0) return {};

    QVector<double> re(n), im(n);
    for (int i = 0; i < n; ++i) {
        re[i] = interleaved[2 * i];
        im[i] = interleaved[2 * i + 1];
    }

    bitReversePermute(re, im);
    for (int m = 2; m <= n; m *= 2) {
        for (int start = 0; start < n; start += m) {
            splitRadixCore(re, im, m, false);
        }
    }

    QVector<double> result(2 * n);
    for (int i = 0; i < n; ++i) {
        result[2 * i] = re[i];
        result[2 * i + 1] = im[i];
    }

    double elapsed = timer.elapsed();
    m_stats.transformSize = n;
    m_stats.numForwardTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformCompleted(n, false, false, elapsed);
    return result;
}

QVector<double> SplitRadixFFT9::inverseComplex(const QVector<double>& interleaved)
{
    QElapsedTimer timer;
    timer.start();

    int n = interleaved.size() / 2;
    if (n < 2 || (n & (n - 1)) != 0) return {};

    QVector<double> re(n), im(n);
    for (int i = 0; i < n; ++i) {
        re[i] = interleaved[2 * i];
        im[i] = interleaved[2 * i + 1];
    }

    bitReversePermute(re, im);
    for (int m = 2; m <= n; m *= 2) {
        for (int start = 0; start < n; start += m) {
            splitRadixCore(re, im, m, true);
        }
    }

    QVector<double> result(2 * n);
    for (int i = 0; i < n; ++i) {
        result[2 * i] = re[i] / n;
        result[2 * i + 1] = im[i] / n;
    }

    double elapsed = timer.elapsed();
    m_stats.numInverseTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformCompleted(n, false, true, elapsed);
    return result;
}

/* ---- Reset ---- */

void SplitRadixFFT9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
