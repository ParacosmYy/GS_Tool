/**
 * @file MixedRadixFFT5.cpp
 * @brief MixedRadixFFT5 实现
 *
 * 实现混合基FFT：自动因子分解、SIMD友好内存布局、多基数蝶形运算。
 */

#include "utils/fft199/MixedRadixFFT5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MixedRadixFFT5::MixedRadixFFT5(QObject *parent) : QObject(parent) { buildTables(); }
MixedRadixFFT5::~MixedRadixFFT5() = default;

/* ---- Configuration ---- */

void MixedRadixFFT5::setTransformSize(int n) { m_size = qMax(2, n); buildTables(); }
void MixedRadixFFT5::setAllowInPlace(bool e) { m_inPlace = e; }

/* ---- Factorize ---- */

QVector<MixedRadixFFT5::Factor> MixedRadixFFT5::factorize(int n) const
{
    QVector<Factor> factors;
    static const int radices[] = {4, 2, 3, 5, 7};

    int remaining = n;
    for (int r : radices) {
        if (remaining <= 1) break;
        int count = 0;
        while (remaining % r == 0) { remaining /= r; count++; }
        if (count > 0) factors.append({r, count});
    }

    // Remaining prime factor
    if (remaining > 1) factors.append({remaining, 1});
    return factors;
}

/* ---- Build tables ---- */

void MixedRadixFFT5::buildTables()
{
    m_factors = factorize(m_size);
    m_twiddleRe.resize(m_size);
    m_twiddleIm.resize(m_size);

    for (int i = 0; i < m_size; ++i) {
        double angle = -2.0 * M_PI * i / m_size;
        m_twiddleRe[i] = qCos(angle);
        m_twiddleIm[i] = qSin(angle);
    }

    m_permutation = bitReversePermutation(m_size, m_factors);
}

/* ---- Bit/digit-reverse permutation ---- */

QVector<int> MixedRadixFFT5::bitReversePermutation(int n, const QVector<Factor>& factors) const
{
    // Compute mixed-radix digit reversal
    QVector<int> perm(n, 0);
    int product = 1;

    // Compute stride products
    QVector<int> strides;
    for (const auto& f : factors) {
        for (int c = 0; c < f.count; ++c) {
            strides.append(product);
            product *= f.radix;
        }
    }

    if (product != n) return perm;

    int numDigits = strides.size();
    for (int i = 0; i < n; ++i) {
        int val = i, rev = 0;
        for (int d = numDigits - 1; d >= 0; --d) {
            int radix = 2; // default
            // Find radix for this digit position
            int pos = 0;
            for (const auto& f : factors) {
                for (int c = 0; c < f.count; ++c) {
                    if (pos == d) { radix = f.radix; break; }
                    pos++;
                }
                if (pos > d) break;
            }
            rev = rev * radix + (val % radix);
            val /= radix;
        }
        perm[i] = rev;
    }
    return perm;
}

void MixedRadixFFT5::digitReverse(QVector<double>& re, QVector<double>& im,
                                   const QVector<Factor>& factors) const
{
    int n = re.size();
    QVector<double> tmpRe(n), tmpIm(n);
    for (int i = 0; i < n; ++i) {
        if (m_permutation[i] < n) {
            tmpRe[i] = re[m_permutation[i]];
            tmpIm[i] = im[m_permutation[i]];
        }
    }
    re = tmpRe;
    im = tmpIm;
}

/* ---- Twiddle factors ---- */

QVector<double> MixedRadixFFT5::computeTwiddles(int n) const
{
    QVector<double> tw(n);
    for (int i = 0; i < n; ++i)
        tw[i] = -2.0 * M_PI * i / n;
    return tw;
}

/* ---- DFT kernel for small radices ---- */

void MixedRadixFFT5::dftKernel(double* re, double* im, int radix, double twRe, double twIm) const
{
    if (radix == 2) {
        double tR = re[1]*twRe - im[1]*twIm;
        double tI = re[1]*twIm + im[1]*twRe;
        re[1] = re[0] - tR; im[1] = im[0] - tI;
        re[0] += tR; im[0] += tI;
    } else if (radix == 3) {
        double c1 = qCos(-2.0*M_PI/3), s1 = qSin(-2.0*M_PI/3);
        double c2 = qCos(-4.0*M_PI/3), s2 = qSin(-4.0*M_PI/3);
        double aR = re[1]*c1 - im[1]*s1, aI = re[1]*s1 + im[1]*c1;
        double bR = re[2]*c2 - im[2]*s2, bI = re[2]*s2 + im[2]*c2;
        double sR = re[0]+aR+bR, sI = im[0]+aI+bI;
        re[0] = sR; im[0] = sI;
        // Apply twiddle
        re[1] = re[0]+aR*2-bR; im[1] = im[0]+aI*2-bI;
        re[2] = re[0]-aR+bR*2; im[2] = im[0]-aI+bI*2;
    } else if (radix == 4) {
        // Radix-4 butterfly
        double t0R = re[0]+re[2], t0I = im[0]+im[2];
        double t1R = re[0]-re[2], t1I = im[0]-im[2];
        double t2R = re[1]+re[3], t2I = im[1]+im[3];
        double t3R = re[1]-re[3], t3I = im[1]-im[3];
        re[0] = t0R+t2R; im[0] = t0I+t2I;
        re[1] = t1R+t3I*twRe; im[1] = t1I-t3R*twIm;
        re[2] = t0R-t2R; im[2] = t0I-t2I;
        re[3] = t1R-t3I; im[3] = t1I+t3R;
    } else {
        // Generic DFT for other radices
        int n = radix;
        QVector<double> outR(n, 0.0), outI(n, 0.0);
        for (int k = 0; k < n; ++k) {
            for (int j = 0; j < n; ++j) {
                double angle = -2.0 * M_PI * j * k / n;
                double cR = qCos(angle), cI = qSin(angle);
                outR[k] += re[j] * cR - im[j] * cI;
                outI[k] += re[j] * cI + im[j] * cR;
            }
        }
        for (int k = 0; k < n; ++k) { re[k] = outR[k]; im[k] = outI[k]; }
    }
}

/* ---- Butterfly stages ---- */

void MixedRadixFFT5::butterfly(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    int L = 1;   // current stride

    for (const auto& f : m_factors) {
        for (int c = 0; c < f.count; ++c) {
            int radix = f.radix;
            int m = n / (L * radix);

            for (int k = 0; k < m; ++k) {
                for (int j = 0; j < radix; ++j) {
                    int base = k * L * radix + j * L;
                    double twR = 1.0, twI = 0.0;
                    if (j > 0 && m_twiddleRe.size() > 0) {
                        int twIdx = j * m * L;
                        if (twIdx < m_twiddleRe.size()) {
                            twR = m_twiddleRe[twIdx];
                            twI = m_twiddleIm[twIdx];
                        }
                    }
                    // Apply twiddle then kernel
                    for (int i = 0; i < L; ++i) {
                        int idx = base + i;
                        double nR = re[idx]*twR - im[idx]*twI;
                        double nI = re[idx]*twI + im[idx]*twR;
                        re[idx] = nR; im[idx] = nI;
                    }
                }
            }
            L *= radix;
        }
    }
}

/* ---- Forward ---- */

void MixedRadixFFT5::forward(QVector<double>& re, QVector<double>& im)
{
    QElapsedTimer timer;
    timer.start();

    int n = re.size();
    if (n != m_size) { m_size = n; buildTables(); }

    digitReverse(re, im, m_factors);
    butterfly(re, im);

    m_stats.totalTransforms++;
    m_stats.transformSize = n;
    m_stats.numFactors = m_factors.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, m_factors.size(), timer.elapsed());
}

/* ---- Inverse ---- */

void MixedRadixFFT5::inverse(QVector<double>& re, QVector<double>& im)
{
    int n = re.size();
    // Conjugate input
    for (int i = 0; i < n; ++i) im[i] = -im[i];
    forward(re, im);
    double invN = 1.0 / n;
    for (int i = 0; i < n; ++i) { re[i] *= invN; im[i] *= -invN; }
}

/* ---- Forward real ---- */

QVector<double> MixedRadixFFT5::forwardReal(const QVector<double>& input)
{
    int n = input.size();
    QVector<double> re(n, 0.0), im(n, 0.0);
    for (int i = 0; i < n; ++i) re[i] = input[i];
    forward(re, im);

    QVector<double> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = qSqrt(re[i]*re[i] + im[i]*im[i]);
    return result;
}

/* ---- Reset ---- */

void MixedRadixFFT5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
