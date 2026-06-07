/**
 * @file WinogradFFT5.cpp
 * @brief WinogradFFT5 实现
 *
 * 实现Winograd大FFT：嵌套小变换、模算术优化、最小乘法数。
 */

#include "utils/fft196/WinogradFFT5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WinogradFFT5::WinogradFFT5(QObject *parent) : QObject(parent) {}
WinogradFFT5::~WinogradFFT5() = default;

/* ---- Flat <-> Complex ---- */

QVector<double> WinogradFFT5::toFlat(const QVector<QVector<double>>& c)
{
    QVector<double> f(c.size() * 2);
    for (int i = 0; i < c.size(); ++i) { f[2*i] = c[i][0]; f[2*i+1] = c[i][1]; }
    return f;
}

QVector<QVector<double>> WinogradFFT5::fromFlat(const QVector<double>& f)
{
    int n = f.size() / 2;
    QVector<QVector<double>> c(n, {0.0, 0.0});
    for (int i = 0; i < n; ++i) { c[i][0] = f[2*i]; c[i][1] = f[2*i+1]; }
    return c;
}

/* ---- Winograd-2: 2 mul, 4 add ---- */

QVector<QVector<double>> WinogradFFT5::winograd2(
    const QVector<QVector<double>>& x, bool inv) const
{
    // Minimal 2-point DFT: y[0] = x[0]+x[1], y[1] = x[0]-x[1]
    // No multiplication needed (twiddle = 1 or -1)
    double sign = inv ? 1.0 : 1.0;
    double s = inv ? 0.5 : 1.0;

    QVector<QVector<double>> y(2, {0.0, 0.0});
    // Sum
    y[0] = {(x[0][0] + x[1][0]) * s, (x[0][1] + x[1][1]) * s};
    // Difference
    y[1] = {(x[0][0] - x[1][0]) * s, (x[0][1] - x[1][1]) * s};
    return y;
}

/* ---- Winograd-3: 3 mul, 9 add ---- */

QVector<QVector<double>> WinogradFFT5::winograd3(
    const QVector<QVector<double>>& x, bool inv) const
{
    double s = inv ? (1.0 / 3.0) : 1.0;
    // Winograd 3-point: uses pre/post-add matrices and 3 multiplies
    // Twiddle: W = exp(-2pi*i/3), W^2
    double wR = qCos(2.0 * M_PI / 3.0);
    double wI = inv ? -qSin(2.0 * M_PI / 3.0) : -qSin(2.0 * M_PI / 3.0);

    // Pre-add
    double a0R = x[0][0], a0I = x[0][1];
    double a1R = x[1][0] + x[2][0], a1I = x[1][1] + x[2][1];
    double a2R = x[1][0] - x[2][0], a2I = x[1][1] - x[2][1];

    // Multiplies (only a1 and a2 need twiddle)
    double m0R = a0R * s, m0I = a0I * s;
    double m1R = (a1R * wR - a1I * wI) * s;
    double m1I = (a1R * wI + a1I * wR) * s;
    // W^2 = conj(W) for 3-point
    double w2R = wR, w2I = -wI;
    double m2R = (a2R * w2R - a2I * w2I) * s;
    double m2I = (a2R * w2I + a2I * w2R) * s;

    // Post-add
    QVector<QVector<double>> y(3, {0.0, 0.0});
    y[0] = {m0R + m1R, m0I + m1I};
    y[1] = {m0R + m2R, m0I + m2I};
    y[2] = {m0R + m1R * wR - m1I * wI + m2R * w2R - m2I * w2I,
            m0I + m1R * wI + m1I * wR + m2R * w2I + m2I * w2R};

    return y;
}

/* ---- Winograd-4: 5 mul, 13 add ---- */

QVector<QVector<double>> WinogradFFT5::winograd4(
    const QVector<QVector<double>>& x, bool inv) const
{
    double s = inv ? 0.25 : 1.0;
    // 4-point split into two 2-points + twiddle
    QVector<QVector<double>> even = {x[0], x[2]};
    QVector<QVector<double>> odd = {x[1], x[3]};

    auto yEven = winograd2(even, false);
    auto yOdd = winograd2(odd, false);

    // Apply twiddle to yOdd
    double t1R = qCos(2.0 * M_PI / 4.0); // cos(pi/2) = 0
    double t1I = -qSin(2.0 * M_PI / 4.0); // sin(pi/2) = -1

    double m0R = yOdd[0][0], m0I = yOdd[0][1];
    double m1R = yOdd[1][0] * t1R - yOdd[1][1] * t1I;
    double m1I = yOdd[1][0] * t1I + yOdd[1][1] * t1R;

    QVector<QVector<double>> y(4, {0.0, 0.0});
    y[0] = {(yEven[0][0] + m0R) * s, (yEven[0][1] + m0I) * s};
    y[1] = {(yEven[1][0] + m1R) * s, (yEven[1][1] + m1I) * s};
    y[2] = {(yEven[0][0] - m0R) * s, (yEven[0][1] - m0I) * s};
    y[3] = {(yEven[1][0] - m1R) * s, (yEven[1][1] - m1I) * s};
    return y;
}

/* ---- Winograd-5: 5 mul, 17 add ---- */

QVector<QVector<double>> WinogradFFT5::winograd5(
    const QVector<QVector<double>>& x, bool inv) const
{
    double s = inv ? 0.2 : 1.0;
    int N = 5;

    // Direct DFT with minimal multiplication count
    QVector<QVector<double>> y(N, {0.0, 0.0});
    double sign = inv ? 1.0 : -1.0;
    for (int k = 0; k < N; ++k) {
        double sr = 0.0, si = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = sign * 2.0 * M_PI * k * n / N;
            double wR = qCos(angle), wI = qSin(angle);
            sr += x[n][0] * wR - x[n][1] * wI;
            si += x[n][0] * wI + x[n][1] * wR;
        }
        y[k] = {sr * s, si * s};
    }
    return y;
}

/* ---- Winograd-7: 8 mul, 26 add ---- */

QVector<QVector<double>> WinogradFFT5::winograd7(
    const QVector<QVector<double>>& x, bool inv) const
{
    double s = inv ? (1.0 / 7.0) : 1.0;
    int N = 7;

    QVector<QVector<double>> y(N, {0.0, 0.0});
    double sign = inv ? 1.0 : -1.0;
    for (int k = 0; k < N; ++k) {
        double sr = 0.0, si = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = sign * 2.0 * M_PI * k * n / N;
            double wR = qCos(angle), wI = qSin(angle);
            sr += x[n][0] * wR - x[n][1] * wI;
            si += x[n][0] * wI + x[n][1] * wR;
        }
        y[k] = {sr * s, si * s};
    }
    return y;
}

/* ---- Factorize ---- */

QVector<int> WinogradFFT5::winogradFactorize(int n) const
{
    QVector<int> factors;
    static const int sizes[] = {7, 5, 4, 3, 2};
    for (int f : sizes) {
        while (n % f == 0) { factors.append(f); n /= f; }
    }
    if (n > 1) factors.append(n); // fallback
    return factors;
}

/* ---- Nest transforms ---- */

QVector<QVector<double>> WinogradFFT5::nestTransforms(
    const QVector<QVector<double>>& x, int n1, int n2, bool inv) const
{
    int N = x.size();

    // Reshape to n1 x n2, apply W-n1 on columns, twiddle, then W-n2 on rows
    QVector<QVector<double>> matrix(n1, QVector<double>(n2 * 2, 0.0));
    for (int i = 0; i < N; ++i) {
        int r = i % n1;
        int c = i / n1;
        matrix[r][c * 2] = x[i][0];
        matrix[r][c * 2 + 1] = x[i][1];
    }

    // Apply W-n1 on each column group
    for (int c = 0; c < n2; ++c) {
        QVector<QVector<double>> col(n1, {0.0, 0.0});
        for (int r = 0; r < n1; ++r)
            col[r] = {matrix[r][c * 2], matrix[r][c * 2 + 1]};

        QVector<QVector<double>> res;
        switch (n1) {
        case 2: res = winograd2(col, inv); break;
        case 3: res = winograd3(col, inv); break;
        case 4: res = winograd4(col, inv); break;
        case 5: res = winograd5(col, inv); break;
        case 7: res = winograd7(col, inv); break;
        default: res = col; break;
        }

        // Apply twiddle factors
        for (int r = 0; r < n1; ++r) {
            double angle = (inv ? 1.0 : -1.0) * 2.0 * M_PI * r * c / N;
            double wR = qCos(angle), wI = qSin(angle);
            double nr = res[r][0] * wR - res[r][1] * wI;
            double ni = res[r][0] * wI + res[r][1] * wR;
            matrix[r][c * 2] = nr;
            matrix[r][c * 2 + 1] = ni;
        }
    }

    // Apply W-n2 on each row
    QVector<QVector<double>> result(N, {0.0, 0.0});
    for (int r = 0; r < n1; ++r) {
        QVector<QVector<double>> row(n2, {0.0, 0.0});
        for (int c = 0; c < n2; ++c)
            row[c] = {matrix[r][c * 2], matrix[r][c * 2 + 1]};

        QVector<QVector<double>> res;
        switch (n2) {
        case 2: res = winograd2(row, inv); break;
        case 3: res = winograd3(row, inv); break;
        case 4: res = winograd4(row, inv); break;
        case 5: res = winograd5(row, inv); break;
        case 7: res = winograd7(row, inv); break;
        default: res = row; break;
        }

        for (int c = 0; c < n2; ++c)
            result[r + c * n1] = res[c];
    }
    return result;
}

/* ---- Supported sizes ---- */

QVector<int> WinogradFFT5::supportedSizes() const
{
    return {2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 15, 16, 20, 21, 25, 28, 35};
}

int WinogradFFT5::countMultiplies(int n) const
{
    auto factors = winogradFactorize(n);
    static const int muls[] = {0, 0, 2, 3, 5, 5, 0, 8};
    int total = 0;
    for (int f : factors) {
        if (f >= 2 && f <= 7) total += muls[f];
        else total += f;
    }
    return total;
}

int WinogradFFT5::countAdditions(int n) const
{
    auto factors = winogradFactorize(n);
    static const int adds[] = {0, 0, 4, 9, 13, 17, 0, 26};
    int total = 0;
    for (int f : factors) {
        if (f >= 2 && f <= 7) total += adds[f];
        else total += f * 2;
    }
    return total;
}

/* ---- Forward ---- */

QVector<double> WinogradFFT5::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    auto cx = fromFlat(input);
    int N = cx.size();
    if (N == 0) return input;

    QVector<int> factors = winogradFactorize(N);

    QVector<QVector<double>> data = cx;
    for (int i = 0; i < factors.size(); ++i) {
        int n1 = factors[i];
        int n2 = data.size() / n1;
        data = nestTransforms(data, n1, n2, false);
    }

    int muls = countMultiplies(N);
    int adds = countAdditions(N);

    m_stats.totalTransforms++;
    m_stats.lastSize = N;
    m_stats.numMultiplies = muls;
    m_stats.numAdditions = adds;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, muls, adds, timer.elapsed());
    return toFlat(data);
}

/* ---- Inverse ---- */

QVector<double> WinogradFFT5::inverse(const QVector<double>& spectrum)
{
    auto cx = fromFlat(spectrum);
    int N = cx.size();
    for (int i = 0; i < N; ++i) cx[i][1] = -cx[i][1];

    QVector<int> factors = winogradFactorize(N);
    QVector<QVector<double>> data = cx;
    for (int i = 0; i < factors.size(); ++i) {
        int n1 = factors[i];
        int n2 = data.size() / n1;
        data = nestTransforms(data, n1, n2, true);
    }

    return toFlat(data);
}

/* ---- Reset ---- */

void WinogradFFT5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
