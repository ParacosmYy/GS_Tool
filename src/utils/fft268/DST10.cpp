/**
 * @file DST10.cpp
 * @brief DST10 实现
 *
 * 实现离散正弦变换：II型DFT嵌入与奇对称扩展正交DST快速计算。
 */

#include "utils/fft268/DST10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DST10::DST10(QObject *parent)
    : QObject(parent) {}

DST10::~DST10() = default;

/* ---- Utility ---- */

int DST10::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return qMax(2, p);
}

int DST10::numStages(int n)
{
    int s = 0;
    while (n > 1) { n >>= 1; s++; }
    return s;
}

/* ---- Bit-reversal ---- */

void DST10::bitReverse(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    int bits = numStages(n);

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        }
        if (rev > i) {
            std::swap(re[i], re[rev]);
            std::swap(im[i], im[rev]);
        }
    }
}

/* ---- Radix-2 FFT ---- */

void DST10::fftRadix2(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    int stages = numStages(n);

    bitReverse(re, im);

    for (int s = 1; s <= stages; ++s) {
        int m = 1 << s;
        int halfM = m >> 1;
        double angle = -2.0 * M_PI / m;

        for (int k = 0; k < n; k += m) {
            for (int j = 0; j < halfM; ++j) {
                double wAngle = angle * j;
                double wRe = qCos(wAngle);
                double wIm = qSin(wAngle);

                double tRe = wRe * re[k + j + halfM] - wIm * im[k + j + halfM];
                double tIm = wRe * im[k + j + halfM] + wIm * re[k + j + halfM];

                re[k + j + halfM] = re[k + j] - tRe;
                im[k + j + halfM] = im[k + j] - tIm;
                re[k + j] += tRe;
                im[k + j] += tIm;
            }
        }
    }
}

/* ---- Forward DST-II (orthonormal) ---- */

QVector<double> DST10::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    int stages = numStages(n);
    double scale = qSqrt(2.0 / (n + 1));

    // DST-II direct formula:
    // Y[k] = sum_{i=0}^{N-1} x[i] * sin(pi*(i+1)*(k+1)/(N+1))
    QVector<double> result(n, 0.0);
    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i) {
            sum += input[i] * qSin(M_PI * (i + 1) * (k + 1) / (n + 1));
        }
        result[k] = sum * scale;
    }

    double elapsed = timer.elapsed();
    m_stats.transformSize = n;
    m_stats.numStages = stages;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformComputed(n, stages, elapsed);
    return result;
}

/* ---- DST-II via DFT embedding with odd extension ---- */

QVector<double> DST10::transformViaDFT(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    int stages = numStages(nextPow2(2 * N + 2));

    // Odd extension: construct y of length 2(N+1)
    // y[0] = 0, y[i] = x[i-1] for i=1..N, y[N+1] = 0,
    // y[N+2..2N+1] = -x[2N+1-i] (odd symmetry)
    int M = 2 * (N + 1);
    int M2 = nextPow2(M);

    QVector<double> re(M2, 0.0);
    QVector<double> im(M2, 0.0);

    // Build odd-symmetric extension
    for (int i = 0; i < N; ++i) {
        re[i + 1] = input[i];
        re[M - 1 - i] = -input[i]; // Odd symmetry
    }
    // y[0] = 0, y[N+1] = 0 (already zero)

    // Compute DFT
    fftRadix2(re, im);

    // Extract DST coefficients from imaginary parts
    double scale = qSqrt(2.0 / (N + 1));
    QVector<double> result(N);
    for (int k = 0; k < N; ++k) {
        result[k] = -im[k + 1] * scale;
    }

    double elapsed = timer.elapsed();
    m_stats.transformSize = N;
    m_stats.numStages = stages;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformComputed(N, stages, elapsed);
    return result;
}

/* ---- Inverse DST-II (orthonormal, = DST-III) ---- */

QVector<double> DST10::inverseTransform(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int n = spectrum.size();
    double scale = qSqrt(2.0 / (n + 1));

    // Inverse DST-II = DST-III (orthonormal DST is self-inverse up to definition)
    // Y[i] = sum_{k=0}^{N-1} X[k] * sin(pi*(i+1)*(k+1)/(N+1))
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < n; ++k) {
            sum += spectrum[k] * qSin(M_PI * (i + 1) * (k + 1) / (n + 1));
        }
        result[i] = sum * scale;
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Reset ---- */

void DST10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
