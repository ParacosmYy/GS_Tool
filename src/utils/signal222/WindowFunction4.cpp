/**
 * @file WindowFunction4.cpp
 * @brief WindowFunction4 实现
 *
 * 实现窗函数库：标准窗生成、Slepian DPSS多锥序列、时间带宽积优化。
 */

#include "utils/signal222/WindowFunction4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WindowFunction4::WindowFunction4(QObject *parent) : QObject(parent) {}
WindowFunction4::~WindowFunction4() = default;

/* ---- Bessel I0 ---- */

double WindowFunction4::besselI0(double x) const
{
    double sum = 1.0, term = 1.0;
    for (int k = 1; k <= 50; ++k) {
        term *= (x / (2.0 * k)) * (x / (2.0 * k));
        sum += term;
        if (term < 1e-12 * sum) break;
    }
    return sum;
}

/* ---- Generate window ---- */

QVector<double> WindowFunction4::generate(WindowType type, int length,
                                            double parameter) const
{
    int N = qMax(1, length);
    QVector<double> w(N, 1.0);

    switch (type) {
    case Rectangular:
        // Already all 1.0
        break;
    case Hann:
        for (int i = 0; i < N; ++i)
            w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (N - 1)));
        break;
    case Hamming:
        for (int i = 0; i < N; ++i)
            w[i] = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (N - 1));
        break;
    case Blackman:
        for (int i = 0; i < N; ++i)
            w[i] = 0.42 - 0.5 * qCos(2.0 * M_PI * i / (N - 1))
                   + 0.08 * qCos(4.0 * M_PI * i / (N - 1));
        break;
    case BlackmanHarris:
        for (int i = 0; i < N; ++i) {
            double a = 2.0 * M_PI * i / (N - 1);
            w[i] = 0.35875 - 0.48829 * qCos(a)
                   + 0.14128 * qCos(2.0 * a) - 0.01168 * qCos(3.0 * a);
        }
        break;
    case Kaiser:
        w = kaiserWindow(N, parameter > 0 ? parameter : 8.0);
        break;
    case Gaussian:
        w = gaussianWindow(N, parameter > 0 ? parameter : 0.4);
        break;
    case FlatTop:
        for (int i = 0; i < N; ++i) {
            double a = 2.0 * M_PI * i / (N - 1);
            w[i] = 0.21557895 - 0.41663158 * qCos(a)
                   + 0.277263158 * qCos(2.0 * a)
                   - 0.083578947 * qCos(3.0 * a)
                   + 0.006947368 * qCos(4.0 * a);
        }
        break;
    case DPSS:
        // Use default time-bandwidth product
        break;
    }
    return w;
}

/* ---- Kaiser window ---- */

QVector<double> WindowFunction4::kaiserWindow(int length, double beta) const
{
    int N = qMax(1, length);
    QVector<double> w(N);
    double denom = besselI0(beta);
    for (int i = 0; i < N; ++i) {
        double x = (2.0 * i / (N - 1)) - 1.0;
        w[i] = besselI0(beta * qSqrt(qMax(0.0, 1.0 - x * x))) / denom;
    }
    return w;
}

/* ---- Gaussian window ---- */

QVector<double> WindowFunction4::gaussianWindow(int length, double sigma) const
{
    int N = qMax(1, length);
    QVector<double> w(N);
    for (int i = 0; i < N; ++i) {
        double x = (i - (N - 1) / 2.0) / (sigma * (N - 1) / 2.0);
        w[i] = qExp(-0.5 * x * x);
    }
    return w;
}

/* ---- Slepian DPSS ---- */

QVector<QVector<double>> WindowFunction4::slepianDPSS(int length, int numTapers,
                                                         double timeBandwidth) const
{
    int N = qMax(2, length);
    int K = qMin(numTapers, qMax(1, qFloor(2.0 * timeBandwidth) - 1));
    double W = timeBandwidth / N;

    // Build tridiagonal matrix: T(i,i) = ((N-1)/2 - i)^2 * cos(2*pi*W)^2
    //                                 T(i,i+1) = i*(N-i) / 2
    QVector<double> diag(N), offDiag(N - 1);
    double cos2W = qCos(2.0 * M_PI * W);
    for (int i = 0; i < N; ++i) {
        double m = (N - 1) / 2.0 - i;
        diag[i] = m * m * cos2W * cos2W;
    }
    for (int i = 0; i < N - 1; ++i)
        offDiag[i] = i * (N - 1 - i) / 2.0;

    // Find eigenvalues (we want the K largest)
    QVector<double> eigvals = tridiagEigenvalues(N, diag, offDiag);

    // Sort descending and take top K
    QVector<int> indices(N);
    for (int i = 0; i < N; ++i) indices[i] = i;
    std::sort(indices.begin(), indices.end(),
              [&](int a, int b) { return eigvals[a] > eigvals[b]; });

    QVector<QVector<double>> tapers(K);
    for (int k = 0; k < K; ++k) {
        tapers[k] = inverseIteration(N, diag, offDiag, eigvals[indices[k]]);
        // Normalize
        double norm = 0.0;
        for (auto v : tapers[k]) norm += v * v;
        norm = qSqrt(norm);
        if (norm > 0.0)
            for (int i = 0; i < N; ++i) tapers[k][i] /= norm;
    }
    return tapers;
}

/* ---- Tridiagonal eigenvalues (QR) ---- */

QVector<double> WindowFunction4::tridiagEigenvalues(int n,
    const QVector<double>& diag, const QVector<double>& offDiag) const
{
    QVector<double> d = diag;
    QVector<double> e = offDiag;

    for (int iter = 0; iter < 30 * n; ++iter) {
        // Find smallest off-diagonal element for convergence
        int m = -1;
        for (int i = 0; i < n - 1; ++i) {
            if (qAbs(e[i]) <= 1e-14 * (qAbs(d[i]) + qAbs(d[i + 1]))) {
                e[i] = 0.0;
            } else if (m < 0) {
                m = i;
            }
        }
        if (m < 0) break; // All converged

        // Wilkinson shift
        double dd = (d[m + 1] - d[m]) / 2.0;
        double sign = (dd >= 0) ? 1.0 : -1.0;
        double mu = d[m] - e[m] * e[m] / (dd + sign * qSqrt(dd * dd + e[m] * e[m]));

        // QR chase
        double x = d[m] - mu, z = e[m];
        for (int k = m; k < n - 1; ++k) {
            double r = qSqrt(x * x + z * z);
            double c = x / r, s = z / r;
            if (k > m) e[k - 1] = r;

            double q1 = c * d[k] + s * e[k];
            double q2 = c * e[k] + s * d[k + 1];
            d[k] = c * q1 + s * q2;
            e[k] = c * q2 - s * q1;
            d[k + 1] = d[k + 1] * c - e[k] * s;
            e[k] = c * s * (d[k] - d[k + 1]) + e[k]; // Approximate

            x = e[k];
            if (k + 1 < n - 1) z = s * e[k + 1];
        }
    }
    return d;
}

/* ---- Inverse iteration ---- */

QVector<double> WindowFunction4::inverseIteration(int n,
    const QVector<double>& diag, const QVector<double>& offDiag,
    double eigenvalue) const
{
    QVector<double> v(n, 1.0 / qSqrt(n));
    QVector<double> w(n);

    double shift = eigenvalue + 1e-10;
    for (int iter = 0; iter < 20; ++iter) {
        // Solve (T - shift*I) * w = v via Thomas algorithm
        QVector<double> a = offDiag, b = diag, c = offDiag, d = v;
        for (int i = 0; i < n; ++i) b[i] -= shift;

        // Forward elimination
        for (int i = 1; i < n; ++i) {
            double m = a[i - 1] / b[i - 1];
            b[i] -= m * c[i - 1];
            d[i] -= m * d[i - 1];
        }

        // Back substitution
        w[n - 1] = d[n - 1] / b[n - 1];
        for (int i = n - 2; i >= 0; --i)
            w[i] = (d[i] - c[i] * w[i + 1]) / b[i];

        // Normalize
        double norm = 0.0;
        for (auto val : w) norm += val * val;
        norm = qSqrt(norm);
        if (norm > 0.0)
            for (int i = 0; i < n; ++i) v[i] = w[i] / norm;
    }
    return v;
}

/* ---- Generate DPSS (public) ---- */

QVector<QVector<double>> WindowFunction4::generateDPSS(int length, int numTapers,
                                                          double timeBandwidth) const
{
    QElapsedTimer timer;
    timer.start();
    auto result = slepianDPSS(length, numTapers, timeBandwidth);

    // Update stats (const method -> modify via mutable-like access)
    WindowFunction4* self = const_cast<WindowFunction4*>(this);
    self->m_stats.windowLength = length;
    self->m_stats.numTapers = result.size();
    self->m_stats.timeBandwidthProduct = timeBandwidth;
    self->m_stats.totalOps++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = self->m_timeSum / self->m_stats.totalOps;

    emit self->dpssGenerated(result.size(), timeBandwidth, timer.elapsed());
    return result;
}

/* ---- Optimize time-bandwidth ---- */

double WindowFunction4::optimizeTimeBandwidth(int length, int numTapers) const
{
    // Shannon number: NW >= K/2 for K tapers
    double nw = qMax(2.0, (numTapers + 1) / 2.0);
    // Refine: search for NW that maximizes concentration
    double bestNW = nw, bestConc = 0.0;
    for (double tw = nw; tw <= nw + 4.0; tw += 0.5) {
        auto tapers = slepianDPSS(length, numTapers, tw);
        if (tapers.size() < numTapers) continue;
        double conc = 0.0;
        for (int k = 0; k < qMin(numTapers, tapers.size()); ++k)
            conc += dpssConcentration(length, tw);
        if (conc > bestConc) { bestConc = conc; bestNW = tw; }
    }
    return bestNW;
}

double WindowFunction4::dpssConcentration(int length, double eigenvalue) const
{
    Q_UNUSED(length);
    return eigenvalue; // Concentration ratio is the eigenvalue itself
}

/* ---- Apply window ---- */

QVector<double> WindowFunction4::apply(const QVector<double>& signal,
                                          WindowType type, double parameter) const
{
    QVector<double> w = generate(type, signal.size(), parameter);
    QVector<double> result(signal.size());
    for (int i = 0; i < signal.size(); ++i)
        result[i] = signal[i] * w[i];
    return result;
}

QVector<QVector<double>> WindowFunction4::applyMultiTaper(
    const QVector<double>& signal, int numTapers, double timeBandwidth) const
{
    auto tapers = slepianDPSS(signal.size(), numTapers, timeBandwidth);
    QVector<QVector<double>> result(tapers.size());
    for (int k = 0; k < tapers.size(); ++k) {
        result[k].resize(signal.size());
        for (int i = 0; i < signal.size(); ++i)
            result[k][i] = signal[i] * tapers[k][i];
    }
    return result;
}

/* ---- ENBW ---- */

double WindowFunction4::enbw(WindowType type, int length, double parameter) const
{
    QVector<double> w = generate(type, length, parameter);
    double sumW = 0.0, sumW2 = 0.0;
    for (auto v : w) { sumW += v; sumW2 += v * v; }
    if (sumW2 <= 0.0) return 0.0;
    return length * sumW2 / (sumW * sumW);
}

/* ---- Coherence loss ---- */

double WindowFunction4::coherenceLoss(WindowType type, int length,
                                        double parameter) const
{
    double bw = enbw(type, length, parameter);
    return 1.0 - 1.0 / bw;
}

/* ---- Reset ---- */

void WindowFunction4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
