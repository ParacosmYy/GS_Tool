/**
 * @file WindowFunction5.cpp
 * @brief WindowFunction5 实现
 *
 * 实现窗函数：Kaiser-Bessel与Gauss可调参数窗及旁瓣电平比较度量。
 */

#include "utils/signal250/WindowFunction5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WindowFunction5::WindowFunction5(QObject *parent) : QObject(parent) {}
WindowFunction5::~WindowFunction5() = default;

/* ---- Modified Bessel I0 for Kaiser window ---- */

double WindowFunction5::besselI0(double x) const
{
    double sum = 1.0;
    double term = 1.0;
    double xHalf = x / 2.0;
    for (int k = 1; k < 50; ++k) {
        double t = xHalf / k;
        term *= t * t;
        sum += term;
        if (term < 1e-15 * sum) break;
    }
    return sum;
}

/* ---- ENBW ---- */

double WindowFunction5::computeENBW(const QVector<double>& w) const
{
    if (w.isEmpty()) return 0.0;
    double sumW = 0.0, sumW2 = 0.0;
    for (double v : w) { sumW += v; sumW2 += v * v; }
    if (sumW < 1e-15 || sumW2 < 1e-15) return 0.0;
    return sumW2 * w.size() / (sumW * sumW);
}

/* ---- Coherent gain ---- */

double WindowFunction5::computeCoherentGain(const QVector<double>& w) const
{
    if (w.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double v : w) sum += v;
    return sum / w.size();
}

/* ---- Estimate side-lobe level ---- */

double WindowFunction5::estimateSideLobeDb(const QVector<double>& w) const
{
    int N = w.size();
    if (N < 4) return 0.0;

    // Compute magnitude spectrum via DFT at sufficient resolution
    int fftSize = N * 4;
    double maxMainLobe = 0.0, maxSideLobe = 0.0;
    int mainLobeEnd = 0;

    // Sample the spectrum around DC to find main lobe
    for (int k = 0; k < fftSize / 2; ++k) {
        double freq = static_cast<double>(k) / fftSize;
        double real = 0.0, imag = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * freq * n;
            real += w[n] * qCos(angle);
            imag += w[n] * qSin(angle);
        }
        double mag = qSqrt(real * real + imag * imag);
        if (k == 0 || mag > maxMainLobe) {
            maxMainLobe = mag;
            mainLobeEnd = k;
        }
    }

    // Find main lobe boundary (first null)
    for (int k = 1; k < fftSize / 2; ++k) {
        double freq = static_cast<double>(k) / fftSize;
        double real = 0.0, imag = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * freq * n;
            real += w[n] * qCos(angle);
            imag += w[n] * qSin(angle);
        }
        double mag = qSqrt(real * real + imag * imag);
        if (mag > maxMainLobe * 0.5) mainLobeEnd = k;
        else break;
    }

    // Find max side-lobe after main lobe
    for (int k = mainLobeEnd + 1; k < fftSize / 2; ++k) {
        double freq = static_cast<double>(k) / fftSize;
        double real = 0.0, imag = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * freq * n;
            real += w[n] * qCos(angle);
            imag += w[n] * qSin(angle);
        }
        double mag = qSqrt(real * real + imag * imag);
        if (mag > maxSideLobe) maxSideLobe = mag;
    }

    if (maxMainLobe < 1e-15) return -200.0;
    double ratio = maxSideLobe / maxMainLobe;
    return (ratio > 1e-15) ? 20.0 * qLn(ratio) / qLn(10.0) : -200.0;
}

/* ---- Scalloping loss ---- */

double WindowFunction5::computeScallopingLoss(const QVector<double>& w) const
{
    if (w.isEmpty()) return 0.0;
    // Compute response at half-bin frequency
    int N = w.size();
    double real = 0.0, imag = 0.0;
    for (int n = 0; n < N; ++n) {
        double angle = -M_PI * n / N;
        real += w[n] * qCos(angle);
        imag += w[n] * qSin(angle);
    }
    double atHalf = qSqrt(real * real + imag * imag);
    double atDC = 0.0;
    for (double v : w) atDC += v;
    if (atDC < 1e-15) return 0.0;
    return 20.0 * qLn(atHalf / atDC) / qLn(10.0);
}

/* ---- Window generators ---- */

QVector<double> WindowFunction5::genHann(int size) const
{
    QVector<double> w(size);
    for (int n = 0; n < size; ++n)
        w[n] = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (size - 1)));
    return w;
}

QVector<double> WindowFunction5::genHamming(int size) const
{
    QVector<double> w(size);
    for (int n = 0; n < size; ++n)
        w[n] = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (size - 1));
    return w;
}

QVector<double> WindowFunction5::genBlackman(int size) const
{
    QVector<double> w(size);
    for (int n = 0; n < size; ++n)
        w[n] = 0.42 - 0.5 * qCos(2.0 * M_PI * n / (size - 1))
             + 0.08 * qCos(4.0 * M_PI * n / (size - 1));
    return w;
}

QVector<double> WindowFunction5::genBlackmanHarris(int size) const
{
    QVector<double> w(size);
    for (int n = 0; n < size; ++n) {
        double d = 2.0 * M_PI * n / (size - 1);
        w[n] = 0.35875 - 0.48829 * qCos(d) + 0.14128 * qCos(2.0 * d)
             - 0.01168 * qCos(3.0 * d);
    }
    return w;
}

QVector<double> WindowFunction5::genFlatTop(int size) const
{
    QVector<double> w(size);
    for (int n = 0; n < size; ++n) {
        double d = 2.0 * M_PI * n / (size - 1);
        w[n] = 0.21557895 - 0.41663158 * qCos(d) + 0.277263158 * qCos(2.0 * d)
             - 0.083578947 * qCos(3.0 * d) + 0.006947368 * qCos(4.0 * d);
    }
    return w;
}

QVector<double> WindowFunction5::genNuttall(int size) const
{
    QVector<double> w(size);
    for (int n = 0; n < size; ++n) {
        double d = 2.0 * M_PI * n / (size - 1);
        w[n] = 0.3635819 - 0.4891775 * qCos(d) + 0.1365995 * qCos(2.0 * d)
             - 0.0106411 * qCos(3.0 * d);
    }
    return w;
}

QVector<double> WindowFunction5::genTukey(int size, double alpha) const
{
    QVector<double> w(size, 1.0);
    int ramp = qRound(alpha * (size - 1) / 2.0);
    for (int n = 0; n < ramp; ++n) {
        double val = 0.5 * (1.0 + qCos(M_PI * (n / ramp - 1.0)));
        w[n] = val;
        w[size - 1 - n] = val;
    }
    return w;
}

/* ---- Generate window by type ---- */

QVector<double> WindowFunction5::generate(WindowType type, int size)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> w;
    switch (type) {
        case Rectangular: w = QVector<double>(size, 1.0); break;
        case Hann: w = genHann(size); break;
        case Hamming: w = genHamming(size); break;
        case Blackman: w = genBlackman(size); break;
        case BlackmanHarris: w = genBlackmanHarris(size); break;
        case KaiserBessel: w = kaiser(size, 8.0); break;
        case Gaussian: w = gaussian(size, 0.4); break;
        case FlatTop: w = genFlatTop(size); break;
        case Nuttall: w = genNuttall(size); break;
        case Tukey: w = genTukey(size, 0.25); break;
    }

    m_stats.windowSize = size;
    m_stats.numWindows++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    double sll = estimateSideLobeDb(w);
    emit windowGenerated(size, static_cast<int>(type), sll, timer.elapsed());
    return w;
}

/* ---- Kaiser window ---- */

QVector<double> WindowFunction5::kaiser(int size, double beta)
{
    QVector<double> w(size);
    double denom = besselI0(beta);
    int N1 = size - 1;
    for (int n = 0; n < size; ++n) {
        double arg = beta * qSqrt(1.0 - qPow(2.0 * n / N1 - 1.0, 2));
        w[n] = besselI0(arg) / denom;
    }
    return w;
}

/* ---- Gaussian window ---- */

QVector<double> WindowFunction5::gaussian(int size, double sigma)
{
    QVector<double> w(size);
    int N1 = size - 1;
    for (int n = 0; n < size; ++n) {
        double t = (n - N1 / 2.0) / (sigma * N1 / 2.0);
        w[n] = qExp(-0.5 * t * t);
    }
    return w;
}

/* ---- Apply window to signal ---- */

QVector<double> WindowFunction5::apply(const QVector<double>& window,
                                         const QVector<double>& signal) const
{
    int n = qMin(window.size(), signal.size());
    QVector<double> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = window[i] * signal[i];
    return result;
}

/* ---- Compute metrics ---- */

WindowFunction5::WindowMetrics WindowFunction5::computeMetrics(
    const QVector<double>& window) const
{
    WindowMetrics m;
    m.coherentGain = computeCoherentGain(window);
    m.sideLobeLevelDb = estimateSideLobeDb(window);
    m.enbw = computeENBW(window);
    m.scallopingLossDb = computeScallopingLoss(window);
    m.processingGain = 10.0 * qLn(window.size()) / qLn(10.0) - m.enbw;
    // Main lobe width approximation
    m.mainLobeWidth = 4.0 / window.size(); // bins (approximate for common windows)
    return m;
}

/* ---- Compare side-lobes ---- */

QVector<QPair<WindowFunction5::WindowType, double>>
WindowFunction5::compareSideLobes(int size) const
{
    QVector<QPair<WindowType, double>> results;
    WindowType types[] = {Rectangular, Hann, Hamming, Blackman, BlackmanHarris,
                          Nuttall, FlatTop};
    for (auto t : types) {
        // Use const_cast to call generate in a const context
        auto w = const_cast<WindowFunction5*>(this)->generate(t, size);
        double sll = estimateSideLobeDb(w);
        results.append({t, sll});
    }
    std::sort(results.begin(), results.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    return results;
}

/* ---- Reset ---- */

void WindowFunction5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
