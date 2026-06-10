/**
 * @file WindowFunction6.cpp
 * @brief WindowFunction6 实现
 *
 * 实现窗函数：Kaiser参数化旁瓣衰减平顶加权精确振幅测量。
 */

#include "utils/signal264/WindowFunction6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WindowFunction6::WindowFunction6(QObject *parent)
    : QObject(parent) {}
WindowFunction6::~WindowFunction6() = default;

/* ---- Bessel I0 for Kaiser window ---- */

double WindowFunction6::besselI0(double x) const
{
    // Series expansion for modified Bessel function I0(x)
    double sum = 1.0;
    double term = 1.0;
    for (int k = 1; k <= 30; ++k) {
        term *= (x / (2.0 * k)) * (x / (2.0 * k));
        sum += term;
        if (term < 1e-15 * sum) break;
    }
    return sum;
}

/* ---- Kaiser helpers ---- */

double WindowFunction6::kaiserBetaFromAttenuation(double attenuationDb) const
{
    // Kaiser's formula for beta from desired sidelobe attenuation
    if (attenuationDb > 50.0)
        return 0.1102 * (attenuationDb - 8.7);
    else if (attenuationDb >= 21.0)
        return 0.5842 * qPow(attenuationDb - 21.0, 0.4) + 0.07886 * (attenuationDb - 21.0);
    return 0.0;
}

double WindowFunction6::kaiserCoefficient(int n, int N, double beta) const
{
    double alpha = (N - 1) / 2.0;
    double ratio = (n - alpha) / alpha;
    double arg = beta * qSqrt(qMax(0.0, 1.0 - ratio * ratio));
    return besselI0(arg) / besselI0(beta);
}

/* ---- Flat-top coefficient ---- */

double WindowFunction6::flatTopCoefficient(int n, int N) const
{
    // Flat-top window: maximally flat passband for amplitude accuracy
    // w[n] = a0 - a1*cos(2*pi*n/N-1) + a2*cos(4*pi*n/N-1) - a3*cos(6*pi*n/N-1) + a4*cos(8*pi*n/N-1)
    const double a0 = 0.21557895;
    const double a1 = 0.41663158;
    const double a2 = 0.277263158;
    const double a3 = 0.083578947;
    const double a4 = 0.006947368;

    double x = 2.0 * M_PI * n / (N - 1);
    return a0 - a1 * qCos(x) + a2 * qCos(2.0 * x) - a3 * qCos(3.0 * x) + a4 * qCos(4.0 * x);
}

/* ---- Individual window generators ---- */

QVector<double> WindowFunction6::generateRectangular(int n) const
{
    return QVector<double>(n, 1.0);
}

QVector<double> WindowFunction6::generateHann(int n) const
{
    QVector<double> w(n);
    for (int i = 0; i < n; ++i)
        w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
    return w;
}

QVector<double> WindowFunction6::generateHamming(int n) const
{
    QVector<double> w(n);
    for (int i = 0; i < n; ++i)
        w[i] = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (n - 1));
    return w;
}

QVector<double> WindowFunction6::generateBlackman(int n) const
{
    QVector<double> w(n);
    for (int i = 0; i < n; ++i) {
        double x = 2.0 * M_PI * i / (n - 1);
        w[i] = 0.42 - 0.5 * qCos(x) + 0.08 * qCos(2.0 * x);
    }
    return w;
}

QVector<double> WindowFunction6::generateBlackmanHarris(int n) const
{
    QVector<double> w(n);
    for (int i = 0; i < n; ++i) {
        double x = 2.0 * M_PI * i / (n - 1);
        w[i] = 0.35875 - 0.48829 * qCos(x) + 0.14128 * qCos(2.0 * x) - 0.01168 * qCos(3.0 * x);
    }
    return w;
}

QVector<double> WindowFunction6::generateNuttall(int n) const
{
    QVector<double> w(n);
    for (int i = 0; i < n; ++i) {
        double x = 2.0 * M_PI * i / (n - 1);
        w[i] = 0.3635819 - 0.4891775 * qCos(x) + 0.1365995 * qCos(2.0 * x) - 0.0106411 * qCos(3.0 * x);
    }
    return w;
}

QVector<double> WindowFunction6::generateGaussian(int n, double sigma) const
{
    QVector<double> w(n);
    if (sigma <= 0.0) sigma = 0.4;
    for (int i = 0; i < n; ++i) {
        double x = (i - (n - 1) / 2.0) / (sigma * (n - 1) / 2.0);
        w[i] = qExp(-0.5 * x * x);
    }
    return w;
}

/* ---- Generate window ---- */

QVector<double> WindowFunction6::generate(WindowType type, int size, double parameter)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> window;
    switch (type) {
    case Rectangular:   window = generateRectangular(size); break;
    case Hann:          window = generateHann(size); break;
    case Hamming:       window = generateHamming(size); break;
    case Blackman:      window = generateBlackman(size); break;
    case BlackmanHarris: window = generateBlackmanHarris(size); break;
    case Kaiser:
        window.resize(size);
        for (int i = 0; i < size; ++i)
            window[i] = kaiserCoefficient(i, size, parameter > 0 ? parameter : 8.0);
        break;
    case FlatTop:
        window.resize(size);
        for (int i = 0; i < size; ++i)
            window[i] = flatTopCoefficient(i, size);
        break;
    case Nuttall:       window = generateNuttall(size); break;
    case Gaussian:      window = generateGaussian(size, parameter > 0 ? parameter : 0.4); break;
    }

    double elapsed = timer.elapsed();
    m_stats.windowSize = size;
    m_stats.windowType = static_cast<int>(type);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit windowGenerated(size, static_cast<int>(type), elapsed);
    return window;
}

/* ---- Apply window to signal ---- */

QVector<double> WindowFunction6::apply(const QVector<double>& signal, WindowType type,
                                          double parameter)
{
    if (signal.isEmpty()) return {};
    QVector<double> w = generate(type, signal.size(), parameter);
    QVector<double> result(signal.size());
    for (int i = 0; i < signal.size(); ++i)
        result[i] = signal[i] * w[i];
    return result;
}

/* ---- Compute metrics ---- */

WindowFunction6::WindowMetrics WindowFunction6::computeMetrics(const QVector<double>& window) const
{
    WindowMetrics m;
    int n = window.size();
    if (n == 0) return m;

    // Coherent gain: average of window values
    double sum = 0.0;
    for (double v : window) sum += v;
    m.coherentGain = sum / n;

    // Compute FFT-based metrics (simplified)
    // Equivalent noise bandwidth
    double sumSq = 0.0;
    for (double v : window) sumSq += v * v;
    m.noiseBandwidth = n * sumSq / (sum * sum);

    // 3dB bandwidth (approximate)
    m.threeDbBandwidth = 1.0;  // Simplified

    // Scalloping loss (worst-case amplitude error)
    m.scallopingLoss = 0.0;

    // Sidelobe level (simplified estimate from window type)
    m.sidelobeLevelDb = -20.0 * qLog10(qMax(m.noiseBandwidth, 1e-10));

    return m;
}

/* ---- Reset ---- */

void WindowFunction6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
