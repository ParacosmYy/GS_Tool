/**
 * @file WindowFunction3.cpp
 * @brief WindowFunction3 实现
 *
 * 实现窗函数设计器：Dolph-Chebyshev、Kaiser、多类型窗生成、性能指标分析。
 */

#include "utils/signal208/WindowFunction3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WindowFunction3::WindowFunction3(QObject *parent) : QObject(parent) {}
WindowFunction3::~WindowFunction3() = default;

/* ---- Bessel I0 ---- */

double WindowFunction3::besselI0(double x)
{
    double sum = 1.0, term = 1.0;
    for (int k = 1; k < 30; ++k) {
        double halfX = x / 2.0;
        term *= (halfX / k) * (halfX / k);
        sum += term;
        if (term < 1e-15 * sum) break;
    }
    return sum;
}

/* ---- Chebyshev polynomial ---- */

double WindowFunction3::chebyshevPoly(int n, double x)
{
    if (n == 0) return 1.0;
    if (n == 1) return x;
    double t0 = 1.0, t1 = x, tn = 0.0;
    for (int i = 2; i <= n; ++i) {
        tn = 2.0 * x * t1 - t0;
        t0 = t1;
        t1 = tn;
    }
    return tn;
}

/* ---- DFT magnitude ---- */

double WindowFunction3::dftMagnitude(const QVector<double>& window, double freq)
{
    double re = 0.0, im = 0.0;
    int N = window.size();
    for (int n = 0; n < N; ++n) {
        double ang = -2.0 * M_PI * freq * n / N;
        re += window[n] * qCos(ang);
        im += window[n] * qSin(ang);
    }
    return qSqrt(re * re + im * im);
}

/* ---- Kaiser window ---- */

QVector<double> WindowFunction3::kaiserWindow(int size, double beta) const
{
    QVector<double> w(size);
    double denom = besselI0(beta);
    for (int n = 0; n < size; ++n) {
        double x = 2.0 * n / (size - 1) - 1.0;
        w[n] = besselI0(beta * qSqrt(qMax(0.0, 1.0 - x * x))) / denom;
    }
    return w;
}

/* ---- Dolph-Chebyshev window ---- */

QVector<double> WindowFunction3::dolphChebyshevWindow(int size, double sidelobeDb) const
{
    int N = size;
    if (N < 2) return QVector<double>(N, 1.0);

    double gamma = qPow(10.0, sidelobeDb / 20.0);
    double x0 = qCosh(qAcosh(gamma) / (N - 1));

    QVector<double> w(N);
    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = 0; k < N - 1; ++k) {
            double ang = M_PI * (n - (N - 1) / 2.0) * (k + 0.5) / N;
            double tn = chebyshevPoly(k, x0);
            sum += tn * qCos(ang);
        }
        w[n] = gamma * sum / N;
    }

    // Normalize peak to 1.0
    double peak = *std::max_element(w.begin(), w.end());
    if (peak > 0.0)
        for (auto& v : w) v /= peak;
    return w;
}

/* ---- Optimize Kaiser beta ---- */

double WindowFunction3::optimizeKaiserBeta(double targetSidelobeDb) const
{
    // Empirical relationship: beta ≈ 0.1102(A-8.7) for A > 50
    //                          beta ≈ 0.5842(A-21)^0.4 + 0.07886(A-21) for 21 < A <= 50
    double A = qFabs(targetSidelobeDb);
    double beta;
    if (A > 50.0)
        beta = 0.1102 * (A - 8.7);
    else if (A >= 21.0)
        beta = 0.5842 * qPow(A - 21.0, 0.4) + 0.07886 * (A - 21.0);
    else
        beta = 0.0;
    return beta;
}

/* ---- Kaiser sweep ---- */

QMap<double, WindowMetrics> WindowFunction3::kaiserSweep(int size, double betaMin,
                                                           double betaMax, double step) const
{
    QMap<double, WindowMetrics> results;
    for (double beta = betaMin; beta <= betaMax + 1e-9; beta += step) {
        QVector<double> w = kaiserWindow(size, beta);
        results[beta] = computeMetrics(w);
    }
    return results;
}

/* ---- Compute metrics ---- */

WindowFunction3::WindowMetrics WindowFunction3::computeMetrics(const QVector<double>& window) const
{
    WindowMetrics m;
    int N = window.size();
    if (N == 0) return m;

    // Coherent gain
    double sum = 0.0;
    for (double v : window) sum += v;
    m.coherentGain = sum / N;

    // ENBW
    double sumSq = 0.0;
    for (double v : window) sumSq += v * v;
    m.enbw = N * sumSq / (sum * sum);

    // Mainlobe width: find first zero of DFT
    double mainlobePeak = 0.0;
    for (double f = 0.0; f < 0.5; f += 0.001) {
        double mag = dftMagnitude(window, f);
        if (mag > mainlobePeak) mainlobePeak = mag;
    }
    double halfPeak = mainlobePeak * 0.5;
    m.mainlobeWidth = 0.0;
    for (double f = 0.01; f < 0.5; f += 0.01) {
        if (dftMagnitude(window, f) < halfPeak) {
            m.mainlobeWidth = 2.0 * f * N;
            break;
        }
    }

    // Sidelobe level: max of DFT beyond mainlobe
    double mainlobeEdge = m.mainlobeWidth / (2.0 * N);
    m.sidelobeLevel = -200.0;
    for (double f = mainlobeEdge + 0.01; f < 0.5; f += 0.01) {
        double db = 20.0 * qLn(dftMagnitude(window, f) / qMax(mainlobePeak, 1e-30)) / qLn(10.0);
        if (db > m.sidelobeLevel) m.sidelobeLevel = db;
    }

    // Scallop loss
    double halfBin = dftMagnitude(window, 0.5 / N);
    m.scallopLoss = 20.0 * qLn(halfBin / qMax(mainlobePeak, 1e-30)) / qLn(10.0);

    return m;
}

/* ---- Generate window by type ---- */

QVector<double> WindowFunction3::generate(WindowType type, int size) const
{
    QElapsedTimer timer;
    timer.start();
    QVector<double> w(size);

    switch (type) {
    case Rectangular:
        for (int i = 0; i < size; ++i) w[i] = 1.0;
        break;
    case Hann:
        for (int i = 0; i < size; ++i)
            w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (size - 1)));
        break;
    case Hamming:
        for (int i = 0; i < size; ++i)
            w[i] = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (size - 1));
        break;
    case Blackman:
        for (int i = 0; i < size; ++i)
            w[i] = 0.42 - 0.5 * qCos(2.0 * M_PI * i / (size - 1))
                   + 0.08 * qCos(4.0 * M_PI * i / (size - 1));
        break;
    case BlackmanHarris:
        for (int i = 0; i < size; ++i)
            w[i] = 0.35875 - 0.48829 * qCos(2.0 * M_PI * i / (size - 1))
                   + 0.14128 * qCos(4.0 * M_PI * i / (size - 1))
                   - 0.01168 * qCos(6.0 * M_PI * i / (size - 1));
        break;
    case Kaiser:
        w = kaiserWindow(size, optimizeKaiserBeta(60.0));
        break;
    case DolphChebyshev:
        w = dolphChebyshevWindow(size, 60.0);
        break;
    case FlatTop:
        for (int i = 0; i < size; ++i)
            w[i] = 0.21557895 - 0.41663158 * qCos(2.0 * M_PI * i / (size - 1))
                   + 0.27726316 * qCos(4.0 * M_PI * i / (size - 1))
                   - 0.08357895 * qCos(6.0 * M_PI * i / (size - 1))
                   + 0.00694737 * qCos(8.0 * M_PI * i / (size - 1));
        break;
    case Nuttall:
        for (int i = 0; i < size; ++i)
            w[i] = 0.3635819 - 0.4891775 * qCos(2.0 * M_PI * i / (size - 1))
                   + 0.1365995 * qCos(4.0 * M_PI * i / (size - 1))
                   - 0.0106411 * qCos(6.0 * M_PI * i / (size - 1));
        break;
    case Gaussian: {
        double sigma = 0.4;
        for (int i = 0; i < size; ++i) {
            double x = (i - (size - 1) / 2.0) / (sigma * (size - 1) / 2.0);
            w[i] = qExp(-0.5 * x * x);
        }
        break;
    }
    }

    const_cast<WindowFunction3*>(this)->m_stats.totalOps++;
    const_cast<WindowFunction3*>(this)->m_stats.windowSize = size;
    const_cast<WindowFunction3*>(this)->m_timeSum += timer.elapsed();
    const_cast<WindowFunction3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    const_cast<WindowFunction3*>(this)->emit windowGenerated(
        QString::number(type), size, timer.elapsed());
    return w;
}

/* ---- Apply window ---- */

QVector<double> WindowFunction3::apply(const QVector<double>& signal,
                                         const QVector<double>& window) const
{
    int n = qMin(signal.size(), window.size());
    QVector<double> out(n);
    for (int i = 0; i < n; ++i)
        out[i] = signal[i] * window[i];
    return out;
}

/* ---- Reset ---- */

void WindowFunction3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
