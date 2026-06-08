/**
 * @file WindowFunction4.cpp
 * @brief WindowFunction4 实现
 *
 * 实现窗函数：Dolph-Chebyshev与Slepian DPSS窗及频谱泄漏分析。
 */

#include "utils/signal236/WindowFunction4.h"

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
    double sum = 1.0;
    double term = 1.0;
    for (int k = 1; k < 50; ++k) {
        double half = x / 2.0;
        term *= (half * half) / (k * k);
        sum += term;
        if (term < 1e-16 * sum) break;
    }
    return sum;
}

/* ---- Chebyshev polynomial ---- */

double WindowFunction4::chebyshevPoly(int n, double x) const
{
    if (n == 0) return 1.0;
    if (n == 1) return x;
    double t0 = 1.0, t1 = x, t2 = 0.0;
    for (int i = 2; i <= n; ++i) {
        t2 = 2.0 * x * t1 - t0;
        t0 = t1;
        t1 = t2;
    }
    return t1;
}

/* ---- Solve tridiagonal system ---- */

QVector<double> WindowFunction4::solveTridiagonal(int n, const QVector<double>& diag,
                                                     const QVector<double>& offDiag) const
{
    // Solve for eigenvector of tridiagonal Toeplitz matrix
    // Using inverse power method approximation
    QVector<double> v(n, 1.0 / qSqrt(n));

    // Power iteration for dominant eigenvector
    for (int iter = 0; iter < 50; ++iter) {
        QVector<double> w(n, 0.0);
        w[0] = diag[0] * v[0] + offDiag[0] * v[1];
        for (int i = 1; i < n - 1; ++i)
            w[i] = offDiag[i - 1] * v[i - 1] + diag[i] * v[i] + offDiag[i] * v[i + 1];
        w[n - 1] = offDiag[n - 2] * v[n - 2] + diag[n - 1] * v[n - 1];

        // Normalize
        double norm = 0.0;
        for (int i = 0; i < n; ++i) norm += w[i] * w[i];
        norm = qSqrt(norm);
        if (norm < 1e-16) break;
        for (int i = 0; i < n; ++i) v[i] = w[i] / norm;
    }

    // Ensure positive first element
    if (v[0] < 0)
        for (int i = 0; i < n; ++i) v[i] = -v[i];

    return v;
}

/* ---- Dolph-Chebyshev window ---- */

QVector<double> WindowFunction4::dolphChebyshev(int size, double sidelobeAttenuationDb)
{
    QElapsedTimer timer;
    timer.start();

    if (size < 2) return QVector<double>(1, 1.0);

    double beta = qPow(10.0, sidelobeAttenuationDb / 20.0);
    int N = size - 1;
    double x0 = qCosh(qAcosh(beta) / N);

    QVector<double> w(size);
    // Compute using DFT-based method
    for (int n = 0; n < size; ++n) {
        double sum = 0.0;
        for (int k = 0; k <= N; ++k) {
            double x = x0 * qCos(M_PI * k / size);
            double tN = chebyshevPoly(N, x);
            sum += tN * qCos(2.0 * M_PI * k * (n - N / 2.0) / size);
        }
        w[n] = sum / size;
    }

    // Normalize to peak = 1.0
    double peak = *std::max_element(w.begin(), w.end(),
                  [](double a, double b) { return qAbs(a) < qAbs(b); });
    if (qAbs(peak) > 1e-16)
        for (int i = 0; i < size; ++i) w[i] /= peak;

    m_stats.windowType = DolphChebyshev;
    m_stats.windowSize = size;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return w;
}

/* ---- Slepian DPSS window ---- */

QVector<double> WindowFunction4::slepianDPSS(int size, double timeBandwidth)
{
    QElapsedTimer timer;
    timer.start();

    if (size < 2) return QVector<double>(1, 1.0);

    int N = size;
    double W = timeBandwidth / (2.0 * N);

    // Build tridiagonal matrix for DPSS eigenproblem
    // diag[i] = ((N-1)/2 - i)^2 * cos^2(pi*W)
    // offDiag[i] = i*(N-i) / 2
    QVector<double> diag(N), offDiag(N - 1);
    double cosW = qCos(M_PI * W);
    for (int i = 0; i < N; ++i)
        diag[i] = qPow((N - 1.0) / 2.0 - i, 2) * cosW * cosW;
    for (int i = 0; i < N - 1; ++i)
        offDiag[i] = i * (N - 1 - i) / 2.0;

    QVector<double> dpss = solveTridiagonal(N, diag, offDiag);

    m_stats.windowType = SlepianDPSS;
    m_stats.windowSize = size;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return dpss;
}

/* ---- Generate window ---- */

QVector<double> WindowFunction4::generate(WindowType type, int size)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> w(size);

    switch (type) {
    case Hann:
        for (int n = 0; n < size; ++n)
            w[n] = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (size - 1)));
        break;
    case Hamming:
        for (int n = 0; n < size; ++n)
            w[n] = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (size - 1));
        break;
    case Blackman:
        for (int n = 0; n < size; ++n)
            w[n] = 0.42 - 0.5 * qCos(2.0 * M_PI * n / (size - 1))
                   + 0.08 * qCos(4.0 * M_PI * n / (size - 1));
        break;
    case DolphChebyshev:
        w = dolphChebyshev(size);
        break;
    case SlepianDPSS:
        w = slepianDPSS(size);
        break;
    case Kaiser: {
        double beta = 8.0;
        double denom = besselI0(beta);
        for (int n = 0; n < size; ++n) {
            double arg = beta * qSqrt(1.0 - qPow(2.0 * n / (size - 1) - 1.0, 2));
            w[n] = besselI0(arg) / denom;
        }
        break;
    }
    }

    m_stats.windowType = type;
    m_stats.windowSize = size;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    auto metrics = analyzeLeakage(w);
    emit windowGenerated(type, size, metrics.enbw);
    return w;
}

/* ---- Apply window ---- */

QVector<double> WindowFunction4::apply(const QVector<double>& signal, WindowType type)
{
    QVector<double> w = generate(type, signal.size());
    QVector<double> output(signal.size());
    for (int i = 0; i < signal.size(); ++i)
        output[i] = signal[i] * w[i];
    return output;
}

/* ---- Frequency response ---- */

QVector<double> WindowFunction4::frequencyResponse(const QVector<double>& window,
                                                      int fftSize) const
{
    QVector<double> response(fftSize / 2 + 1);
    for (int k = 0; k <= fftSize / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < window.size(); ++n) {
            double angle = 2.0 * M_PI * k * n / fftSize;
            re += window[n] * qCos(angle);
            im -= window[n] * qSin(angle);
        }
        response[k] = qSqrt(re * re + im * im);
    }
    // Normalize to dB
    double peak = *std::max_element(response.begin(), response.end());
    for (int k = 0; k < response.size(); ++k) {
        if (response[k] > 0 && peak > 0)
            response[k] = 20.0 * qLn(response[k] / peak) / M_LN10;
        else
            response[k] = -200.0;
    }
    return response;
}

/* ---- Analyze leakage ---- */

WindowFunction4::LeakageMetrics WindowFunction4::analyzeLeakage(
    const QVector<double>& window) const
{
    LeakageMetrics m;
    int N = window.size();
    if (N == 0) return m;

    // Coherent gain
    double sum = 0.0;
    for (int i = 0; i < N; ++i) sum += window[i];
    m.coherenceGain = sum / N;

    // ENBW: sum(w^2) / (sum(w)/N)^2
    double sumSq = 0.0;
    for (int i = 0; i < N; ++i) sumSq += window[i] * window[i];
    m.enbw = N * sumSq / (sum * sum);

    // Peak sidelobe level: compute DFT and find highest sidelobe
    int fftSize = qMax(N, 256);
    QVector<double> re(fftSize, 0.0), im(fftSize, 0.0);
    for (int n = 0; n < N; ++n) {
        re[n] = window[n];
    }
    // Simple DFT at various frequencies
    double mainLobe = 0.0;
    for (int n = 0; n < N; ++n) mainLobe += window[n];

    // Find sidelobe via frequency response
    double maxSidelobe = 0.0;
    for (int k = 2; k < fftSize / 2; ++k) {
        double rR = 0.0, rI = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = 2.0 * M_PI * k * n / fftSize;
            rR += window[n] * qCos(angle);
            rI -= window[n] * qSin(angle);
        }
        double mag = qSqrt(rR * rR + rI * rI);
        if (k > 2 && mag > maxSidelobe) maxSidelobe = mag;
    }
    m.sidelobeLevel = (maxSidelobe > 0 && mainLobe > 0)
        ? 20.0 * qLn(maxSidelobe / qAbs(mainLobe)) / M_LN10 : -200.0;

    // 3dB bandwidth (approximate)
    m.threeDbBw = m.enbw * 0.886;  // Empirical approximation

    // Scalloping loss
    double halfBin = 0.0;
    for (int n = 0; n < N; ++n)
        halfBin += window[n] * qCos(M_PI * n / N);
    m.scallopLoss = (mainLobe > 0 && halfBin > 0)
        ? -20.0 * qLn(qAbs(halfBin / mainLobe)) / M_LN10 : 0.0;

    emit analysisCompleted(m.sidelobeLevel, m.scallopLoss);
    return m;
}

/* ---- Reset ---- */

void WindowFunction4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
