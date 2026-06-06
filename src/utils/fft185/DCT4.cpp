/**
 * @file DCT4.cpp
 * @brief DCT4 实现
 *
 * 实现DCT-IV：半移位DFT方法、快速递推算法、正交归一化。
 */

#include "utils/fft185/DCT4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DCT4::DCT4(QObject *parent) : QObject(parent) {}
DCT4::~DCT4() = default;

/* ---- Configuration ---- */

void DCT4::setNormalized(bool enabled) { m_normalized = enabled; }

/* ---- Helpers ---- */

int DCT4::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/* ---- FFT ---- */

void DCT4::fft(QVector<double>& re, QVector<double>& im, bool inv) const
{
    int N = re.size();
    if (N <= 1) return;
    int log2N = 0;
    while ((1 << log2N) < N) ++log2N;

    for (int i = 0; i < N; ++i) {
        int j = 0;
        for (int b = 0; b < log2N; ++b) j = (j << 1) | ((i >> b) & 1);
        if (j > i) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }

    double sign = inv ? 1.0 : -1.0;
    for (int len = 2; len <= N; len *= 2) {
        double ang = sign * 2.0 * M_PI / len;
        double wR = qCos(ang), wI = qSin(ang);
        for (int i = 0; i < N; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = i + j + len / 2;
                double tR = cR * re[o] - cI * im[o];
                double tI = cR * im[o] + cI * re[o];
                re[o] = re[e] - tR; im[o] = im[e] - tI;
                re[e] += tR; im[e] += tI;
                double nR = cR * wR - cI * wI;
                cI = cR * wI + cI * wR; cR = nR;
            }
        }
    }
    if (inv) for (int i = 0; i < N; ++i) { re[i] /= N; im[i] /= N; }
}

/* ---- Half-shift DFT method ---- */

QVector<double> DCT4::viaHalfShiftDFT(const QVector<double>& input) const
{
    int N = input.size();
    if (N == 0) return {};

    // DCT-IV via half-shifted DFT of size 4N
    // Construct half-shifted sequence: x'[k] = x[k] with half-sample shift
    int N4 = 4 * N;
    QVector<double> re(N4, 0.0), im(N4, 0.0);

    // Build extended sequence with half-sample shift
    for (int k = 0; k < N; ++k) {
        re[2 * k + 1] = input[k];    // Half-shift: place at odd indices
        re[2 * N + 2 * k + 1] = -input[N - 1 - k]; // Antisymmetric extension
    }

    fft(re, im, false);

    // Extract DCT-IV coefficients
    QVector<double> result(N);
    for (int k = 0; k < N; ++k) {
        double angle = M_PI * (k + 0.5) * (0.5) / N;
        double cs = qCos(angle), sn = qSin(angle);
        // Pick from the 2k+1 bin (half-shifted)
        int idx = 2 * k + 1;
        result[k] = 2.0 * (re[idx] * cs + im[idx] * sn);
    }

    if (m_normalized) {
        double scale = qSqrt(2.0 / N);
        for (int k = 0; k < N; ++k) result[k] *= scale;
    }
    return result;
}

/* ---- Fast recursive DCT-IV ---- */

QVector<double> DCT4::fastRecurse(const QVector<double>& input) const
{
    int N = input.size();
    if (N == 0) return {};
    if (N == 1) {
        QVector<double> r(1);
        r[0] = m_normalized ? input[0] * qSqrt(2.0) : input[0];
        return r;
    }

    // DCT-IV recursion: split into even and odd symmetric parts
    // Build half-size vectors via pre-twiddle
    int N2 = N / 2;
    QVector<double> u(N2), v(N2);

    for (int k = 0; k < N2; ++k) {
        double angle = M_PI * (2 * k + 1) * 0.5 / (2 * N2);
        double cs = qCos(angle), sn = qSin(angle);
        double a = input[k];
        double b = input[N - 1 - k];
        u[k] = a * cs + b * sn;
        v[k] = -a * sn + b * cs;
    }

    // Recursively compute half-size DCT-IV
    auto uOut = fastRecurse(u);
    auto vOut = fastRecurse(v);

    // Post-twiddle and combine
    QVector<double> result(N);
    for (int k = 0; k < N2; ++k) {
        double angle = M_PI * (2 * k + 1) * 0.5 / N;
        double cs = qCos(angle), sn = qSin(angle);
        result[2 * k] = uOut[k] * cs - vOut[k] * sn;
        result[2 * k + 1] = uOut[k] * sn + vOut[k] * cs;
    }

    if (m_normalized && N == input.size()) {
        double scale = qSqrt(2.0 / N);
        for (int k = 0; k < N; ++k) result[k] *= scale;
    }
    return result;
}

/* ---- Transform matrix ---- */

QVector<QVector<double>> DCT4::transformMatrix(int N) const
{
    QVector<QVector<double>> mat(N, QVector<double>(N));
    double scale = m_normalized ? qSqrt(2.0 / N) : 1.0;
    for (int k = 0; k < N; ++k)
        for (int n = 0; n < N; ++n) {
            double angle = M_PI * (2 * n + 1) * (2 * k + 1) / (4.0 * N);
            mat[k][n] = scale * qCos(angle);
        }
    return mat;
}

/* ---- Forward ---- */

QVector<double> DCT4::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    int N = input.size();
    if (N == 0) return result;

    // Use fast recursion for power-of-2, half-shift DFT otherwise
    if ((N & (N - 1)) == 0)
        result = fastRecurse(input);
    else
        result = viaHalfShiftDFT(input);

    m_stats.totalTransforms++;
    m_stats.transformSize = N;
    m_stats.normalized = m_normalized;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, timer.elapsed());
    return result;
}

/* ---- Inverse (DCT-IV is self-inverse with normalization) ---- */

QVector<double> DCT4::inverse(const QVector<double>& input)
{
    // DCT-IV is an involution: applying it twice gives identity
    // When normalized, DCT4 * DCT4 = I
    // When unnormalized, need to divide by (2N)
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    int N = input.size();
    if (N == 0) return result;

    if ((N & (N - 1)) == 0)
        result = fastRecurse(input);
    else
        result = viaHalfShiftDFT(input);

    if (!m_normalized) {
        double scale = 1.0 / (2.0 * N);
        for (int i = 0; i < N; ++i) result[i] *= scale;
    }

    m_stats.totalTransforms++;
    m_stats.transformSize = N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void DCT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
