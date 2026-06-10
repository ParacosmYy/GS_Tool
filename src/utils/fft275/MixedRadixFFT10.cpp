/**
 * @file MixedRadixFFT10.cpp
 * @brief MixedRadixFFT10 实现
 *
 * 实现混合基数FFT：预计算旋转因子表与自排序原位置换缓存友好复合N变换。
 */

#include "utils/fft275/MixedRadixFFT10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MixedRadixFFT10::MixedRadixFFT10(QObject *parent)
    : QObject(parent)
{
    factorize();
    buildTwiddleTable();
}

MixedRadixFFT10::~MixedRadixFFT10() = default;

/* ---- Configuration ---- */

void MixedRadixFFT10::setSize(int n)
{
    m_n = qBound(1, n, 1 << 20);
    factorize();
    buildTwiddleTable();
}

/* ---- Factorize N into small radices (prefer 4, then 2,3,5) ---- */

void MixedRadixFFT10::factorize()
{
    m_factors.clear();
    int n = m_n;

    // Prefer radix-4 for best cache behavior
    while (n % 4 == 0) { m_factors.append(4); n /= 4; }
    while (n % 2 == 0) { m_factors.append(2); n /= 2; }
    while (n % 3 == 0) { m_factors.append(3); n /= 3; }
    while (n % 5 == 0) { m_factors.append(5); n /= 5; }
    // Remaining prime factor
    if (n > 1) m_factors.append(n);

    // Reverse so we process from inner to outer
    std::reverse(m_factors.begin(), m_factors.end());
}

/* ---- Build precomputed twiddle table ---- */

void MixedRadixFFT10::buildTwiddleTable()
{
    m_twiddles.resize(2 * m_n);  // [cos0, sin0, cos1, sin1, ...]
    for (int k = 0; k < m_n; ++k) {
        double angle = -2.0 * M_PI * k / m_n;
        m_twiddles[2 * k] = qCos(angle);
        m_twiddles[2 * k + 1] = qSin(angle);
    }
}

/* ---- Digit-reversed index for mixed radices ---- */

int MixedRadixFFT10::digitReverse(int index) const
{
    int reversed = 0;
    int numStages = m_factors.size();
    for (int s = 0; s < numStages; ++s) {
        int radix = m_factors[s];
        reversed = reversed * radix + (index % radix);
        index /= radix;
    }
    return reversed;
}

/* ---- In-place digit-reversal permutation ---- */

void MixedRadixFFT10::digitReversePermute(QVector<double>& re, QVector<double>& im) const
{
    for (int i = 0; i < m_n; ++i) {
        int j = digitReverse(i);
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
}

/* ---- Small-N DFT kernel (direct computation) ---- */

void MixedRadixFFT10::dftKernel(double* reOut, double* imOut,
                                 const double* reIn, const double* imIn,
                                 int radix, double twStart, double twStep,
                                 bool inverse) const
{
    double dir = inverse ? 1.0 : -1.0;
    for (int k = 0; k < radix; ++k) {
        double sumRe = 0.0, sumIm = 0.0;
        for (int j = 0; j < radix; ++j) {
            double angle = dir * twStep * j * k;
            double wr = qCos(twStart + angle);
            double wi = qSin(twStart + angle);
            sumRe += reIn[j] * wr - imIn[j] * wi;
            sumIm += reIn[j] * wi + imIn[j] * wr;
        }
        reOut[k] = sumRe;
        imOut[k] = sumIm;
    }
}

/* ---- Single butterfly stage ---- */

void MixedRadixFFT10::butterflyStage(QVector<double>& re, QVector<double>& im,
                                      int stageIdx, bool inverse)
{
    int radix = m_factors[stageIdx];
    // Compute stride and number of groups
    int stride = 1;
    for (int s = 0; s < stageIdx; ++s) stride *= m_factors[s];
    int groups = m_n / (stride * radix);

    for (int g = 0; g < groups; ++g) {
        int base = g * stride * radix;
        for (int b = 0; b < stride; ++b) {
            // Extract radix-length sub-sequences
            QVector<double> subRe(radix), subIm(radix);
            for (int r = 0; r < radix; ++r) {
                subRe[r] = re[base + r * stride + b];
                subIm[r] = im[base + r * stride + b];
            }

            // Compute twiddle step for this stage
            double twStep = 2.0 * M_PI / (stride * radix);
            QVector<double> outRe(radix), outIm(radix);
            dftKernel(outRe.data(), outIm.data(),
                      subRe.data(), subIm.data(),
                      radix, 0.0, twStep, inverse);

            // Apply twiddle factors and write back
            for (int r = 0; r < radix; ++r) {
                int twIdx = r * stride + b;
                double wr = m_twiddles[2 * (twIdx % m_n)];
                double wi = m_twiddles[2 * (twIdx % m_n) + 1];
                if (inverse) wi = -wi;
                double tr = outRe[r] * wr - outIm[r] * wi;
                double ti = outRe[r] * wi + outIm[r] * wr;
                re[base + r * stride + b] = tr;
                im[base + r * stride + b] = ti;
            }
        }
    }
}

/* ---- Forward FFT ---- */

QVector<double> MixedRadixFFT10::forward(const QVector<double>& real,
                                          const QVector<double>& imag)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> re(m_n, 0.0), im(m_n, 0.0);
    for (int i = 0; i < qMin(m_n, real.size()); ++i) re[i] = real[i];
    for (int i = 0; i < qMin(m_n, imag.size()); ++i) im[i] = imag[i];

    // Self-sorting: digit-reverse first
    digitReversePermute(re, im);

    // Process each butterfly stage
    for (int s = 0; s < m_factors.size(); ++s)
        butterflyStage(re, im, s, false);

    // Pack interleaved output
    QVector<double> result(2 * m_n);
    for (int i = 0; i < m_n; ++i) {
        result[2 * i] = re[i];
        result[2 * i + 1] = im[i];
    }

    double elapsed = timer.elapsed();
    m_stats.transformSize = m_n;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(m_n, m_stats.numTransforms, elapsed);

    return result;
}

/* ---- Inverse FFT ---- */

QVector<double> MixedRadixFFT10::inverse(const QVector<double>& real,
                                          const QVector<double>& imag)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> re(m_n, 0.0), im(m_n, 0.0);
    for (int i = 0; i < qMin(m_n, real.size()); ++i) re[i] = real[i];
    for (int i = 0; i < qMin(m_n, imag.size()); ++i) im[i] = -imag[i];  // Conjugate

    digitReversePermute(re, im);
    for (int s = 0; s < m_factors.size(); ++s)
        butterflyStage(re, im, s, true);

    // Scale and conjugate back
    for (int i = 0; i < m_n; ++i) {
        re[i] /= m_n;
        im[i] = -im[i] / m_n;
    }

    QVector<double> result(2 * m_n);
    for (int i = 0; i < m_n; ++i) {
        result[2 * i] = re[i];
        result[2 * i + 1] = im[i];
    }

    double elapsed = timer.elapsed();
    m_stats.transformSize = m_n;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(m_n, m_stats.numTransforms, elapsed);

    return result;
}

/* ---- Get factorization ---- */

QVector<int> MixedRadixFFT10::factors() const { return m_factors; }

/* ---- Reset ---- */

void MixedRadixFFT10::resetStatistics()
{
    m_n = 256;
    m_factors.clear();
    m_twiddles.clear();
    factorize();
    buildTwiddleTable();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
