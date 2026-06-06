/**
 * @file DST4.cpp
 * @brief DST4 实现
 *
 * 实现DST-IV：反对称扩展方法、快速递推算法、正交归一化。
 */

#include "utils/fft186/DST4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DST4::DST4(QObject *parent) : QObject(parent) {}
DST4::~DST4() = default;

/* ---- Configuration ---- */

void DST4::setNormalized(bool enabled) { m_normalized = enabled; }

/* ---- Helpers ---- */

int DST4::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/* ---- FFT ---- */

void DST4::fft(QVector<double>& re, QVector<double>& im, bool inv) const
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

/* ---- Antisymmetric extension method ---- */

QVector<double> DST4::viaAntisymmetric(const QVector<double>& input) const
{
    int N = input.size();
    if (N == 0) return {};

    // DST-IV via antisymmetric extension: extend to 4N with sign flips
    int N4 = 4 * N;
    QVector<double> re(N4, 0.0), im(N4, 0.0);

    // Antisymmetric extension: x, -x_rev, -x, x_rev
    for (int k = 0; k < N; ++k) {
        re[2 * k + 1] = input[k];
        re[2 * N + 2 * k + 1] = -input[k];
    }

    fft(re, im, false);

    // Extract DST-IV coefficients
    QVector<double> result(N);
    for (int k = 0; k < N; ++k) {
        double angle = M_PI * (k + 0.5) * 0.5 / N;
        double cs = qCos(angle), sn = qSin(angle);
        int idx = 2 * k + 1;
        result[k] = 2.0 * (re[idx] * cs + im[idx] * sn);
    }

    if (m_normalized) {
        double scale = qSqrt(2.0 / N);
        for (int k = 0; k < N; ++k) result[k] *= scale;
    }
    return result;
}

/* ---- Fast recursive DST-IV ---- */

QVector<double> DST4::fastRecurse(const QVector<double>& input) const
{
    int N = input.size();
    if (N == 0) return {};
    if (N == 1) {
        QVector<double> r(1);
        // DST-IV single element: sin(pi/2) * input
        r[0] = m_normalized ? input[0] * qSqrt(2.0) : input[0];
        return r;
    }

    // DST-IV recursion: split into two half-size DST-IV
    int N2 = N / 2;
    QVector<double> u(N2), v(N2);

    // Pre-twiddle rotation
    for (int k = 0; k < N2; ++k) {
        double angle = M_PI * (2 * k + 1) * 0.5 / (2 * N2);
        double cs = qCos(angle), sn = qSin(angle);
        double a = input[k];
        double b = input[N - 1 - k];
        // DST-IV uses different sign pattern from DCT-IV
        u[k] = a * cs + b * sn;
        v[k] = a * sn - b * cs;
    }

    auto uOut = fastRecurse(u);
    auto vOut = fastRecurse(v);

    // Post-twiddle and interleave
    QVector<double> result(N);
    for (int k = 0; k < N2; ++k) {
        double angle = M_PI * (2 * k + 1) * 0.5 / N;
        double cs = qCos(angle), sn = qSin(angle);
        result[2 * k] = uOut[k] * cs + vOut[k] * sn;
        result[2 * k + 1] = uOut[k] * sn - vOut[k] * cs;
    }

    if (m_normalized && N == input.size()) {
        double scale = qSqrt(2.0 / N);
        for (int k = 0; k < N; ++k) result[k] *= scale;
    }
    return result;
}

/* ---- Transform matrix ---- */

QVector<QVector<double>> DST4::transformMatrix(int N) const
{
    QVector<QVector<double>> mat(N, QVector<double>(N));
    double scale = m_normalized ? qSqrt(2.0 / N) : 1.0;
    for (int k = 0; k < N; ++k)
        for (int n = 0; n < N; ++n) {
            double angle = M_PI * (2 * n + 1) * (2 * k + 1) / (4.0 * N);
            mat[k][n] = scale * qSin(angle);
        }
    return mat;
}

/* ---- Forward ---- */

QVector<double> DST4::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    int N = input.size();
    if (N == 0) return result;

    if ((N & (N - 1)) == 0)
        result = fastRecurse(input);
    else
        result = viaAntisymmetric(input);

    m_stats.totalTransforms++;
    m_stats.transformSize = N;
    m_stats.normalized = m_normalized;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, timer.elapsed());
    return result;
}

/* ---- Inverse (DST-IV is self-inverse with normalization) ---- */

QVector<double> DST4::inverse(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    int N = input.size();
    if (N == 0) return result;

    if ((N & (N - 1)) == 0)
        result = fastRecurse(input);
    else
        result = viaAntisymmetric(input);

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

void DST4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
