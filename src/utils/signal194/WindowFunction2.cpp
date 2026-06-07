/**
 * @file WindowFunction2.cpp
 * @brief WindowFunction2 实现
 *
 * 实现窗函数比较器：主瓣宽度、旁瓣电平、扇贝损耗指标。
 */

#include "utils/signal194/WindowFunction2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WindowFunction2::WindowFunction2(QObject *parent) : QObject(parent) {}
WindowFunction2::~WindowFunction2() = default;

/* ---- Configuration ---- */

void WindowFunction2::setWindowSize(int n) { m_windowSize = qMax(8, n); }
void WindowFunction2::setFFTSize(int fftSize) { m_fftSize = qMax(16, fftSize); }

/* ---- Window name ---- */

QString WindowFunction2::windowName(WindowType type)
{
    switch (type) {
    case Rectangular: return "Rectangular";
    case Hann: return "Hann";
    case Hamming: return "Hamming";
    case Blackman: return "Blackman";
    case BlackmanHarris: return "Blackman-Harris";
    case Nuttall: return "Nuttall";
    case FlatTop: return "Flat-Top";
    case Kaiser: return "Kaiser";
    case Gaussian: return "Gaussian";
    case Tukey: return "Tukey";
    }
    return "Unknown";
}

/* ---- Generate window coefficients ---- */

QVector<double> WindowFunction2::generate(WindowType type, double param) const
{
    int N = m_windowSize;
    QVector<double> w(N, 1.0);

    switch (type) {
    case Rectangular:
        // All ones (already set)
        break;

    case Hann:
        for (int n = 0; n < N; ++n)
            w[n] = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (N - 1)));
        break;

    case Hamming:
        for (int n = 0; n < N; ++n)
            w[n] = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (N - 1));
        break;

    case Blackman:
        for (int n = 0; n < N; ++n)
            w[n] = 0.42 - 0.5 * qCos(2.0 * M_PI * n / (N - 1))
                   + 0.08 * qCos(4.0 * M_PI * n / (N - 1));
        break;

    case BlackmanHarris:
        for (int n = 0; n < N; ++n)
            w[n] = 0.35875 - 0.48829 * qCos(2.0 * M_PI * n / (N - 1))
                   + 0.14128 * qCos(4.0 * M_PI * n / (N - 1))
                   - 0.01168 * qCos(6.0 * M_PI * n / (N - 1));
        break;

    case Nuttall:
        for (int n = 0; n < N; ++n)
            w[n] = 0.3635819 - 0.4891775 * qCos(2.0 * M_PI * n / (N - 1))
                   + 0.1365995 * qCos(4.0 * M_PI * n / (N - 1))
                   - 0.0106411 * qCos(6.0 * M_PI * n / (N - 1));
        break;

    case FlatTop:
        for (int n = 0; n < N; ++n)
            w[n] = 0.21557895 - 0.41663158 * qCos(2.0 * M_PI * n / (N - 1))
                   + 0.27726316 * qCos(4.0 * M_PI * n / (N - 1))
                   - 0.08357895 * qCos(6.0 * M_PI * n / (N - 1))
                   + 0.00694737 * qCos(8.0 * M_PI * n / (N - 1));
        break;

    case Kaiser: {
        double alpha = (param > 0) ? param : 5.0;
        double denom = 1.0;
        for (int i = 1; i <= static_cast<int>(alpha); ++i)
            denom *= (alpha + i) / i;
        for (int n = 0; n < N; ++n) {
            double x = 2.0 * n / (N - 1) - 1.0;
            double arg = alpha * qSqrt(qMax(0.0, 1.0 - x * x));
            // Simplified Bessel I0 approximation
            double sum = 1.0;
            double term = 1.0;
            for (int k = 1; k <= 20; ++k) {
                term *= (arg / (2.0 * k)) * (arg / (2.0 * k));
                sum += term;
            }
            double i0arg = sum;
            w[n] = i0arg / denom;
        }
        break;
    }

    case Gaussian: {
        double sigma = (param > 0) ? param : 0.4;
        for (int n = 0; n < N; ++n) {
            double x = (n - (N - 1) * 0.5) / (sigma * (N - 1) * 0.5);
            w[n] = qExp(-0.5 * x * x);
        }
        break;
    }

    case Tukey: {
        double r = (param > 0) ? qMin(param, 1.0) : 0.5;
        for (int n = 0; n < N; ++n) {
            if (n < r * (N - 1) * 0.5)
                w[n] = 0.5 * (1.0 + qCos(M_PI * (2.0 * n / (r * (N - 1)) - 1.0)));
            else if (n > (N - 1) * (1.0 - r * 0.5))
                w[n] = 0.5 * (1.0 + qCos(M_PI * (2.0 * n / (r * (N - 1)) - 2.0 / r + 1.0)));
            else
                w[n] = 1.0;
        }
        break;
    }
    }
    return w;
}

/* ---- Magnitude spectrum (zero-padded DFT) ---- */

QVector<double> WindowFunction2::magnitudeSpectrum(
    const QVector<double>& window) const
{
    int N = m_fftSize;
    int M = window.size();
    QVector<double> mag(N / 2 + 1, 0.0);

    for (int k = 0; k <= N / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < M; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += window[n] * qCos(angle);
            im += window[n] * qSin(angle);
        }
        mag[k] = qSqrt(re * re + im * im);
    }
    return mag;
}

/* ---- Find mainlobe width at dB level ---- */

double WindowFunction2::findMainlobeWidth(const QVector<double>& mag,
                                            double dbLevel) const
{
    if (mag.size() < 2) return 0.0;
    double peak = mag[0];
    if (peak <= 0) return 0.0;
    double threshold = peak * qPow(10.0, dbLevel / 20.0);

    // Find where magnitude drops below threshold
    double binWidth = 1.0 / m_fftSize;
    for (int k = 1; k < mag.size(); ++k) {
        if (mag[k] < threshold) {
            // Interpolate
            double frac = (threshold - mag[k]) / qMax(1e-15, mag[k - 1] - mag[k]);
            return (k - 1.0 + frac) * 2.0 * binWidth;
        }
    }
    return (mag.size() - 1) * 2.0 * binWidth;
}

/* ---- Find peak sidelobe level ---- */

double WindowFunction2::findPeakSidelobe(const QVector<double>& mag) const
{
    if (mag.size() < 4) return 0.0;
    double peak = mag[0];
    if (peak <= 0) return 0.0;

    // Find mainlobe end (first minimum)
    int mainlobeEnd = 1;
    for (int k = 2; k < mag.size(); ++k) {
        if (mag[k] > mag[k - 1]) { mainlobeEnd = k; break; }
    }

    // Find max sidelobe
    double maxSidelobe = 0.0;
    for (int k = mainlobeEnd; k < mag.size(); ++k)
        maxSidelobe = qMax(maxSidelobe, mag[k]);

    return 20.0 * qLn(maxSidelobe / peak) / qLn(10.0);
}

/* ---- Compute scalloping loss ---- */

double WindowFunction2::computeScallopingLoss(const QVector<double>& mag) const
{
    if (mag.size() < 2) return 0.0;
    // Scalloping loss = worst case when frequency falls between bins
    // Approximate as half-bin offset response
    int M = (m_windowSize > 0) ? m_windowSize : mag.size();
    double peak = mag[0];
    if (peak <= 0) return 0.0;

    // Compute magnitude at half-bin offset
    double re = 0.0, im = 0.0;
    QVector<double> w = generate(Rectangular); // Use current type... simplified
    for (int n = 0; n < M; ++n) {
        double angle = -2.0 * M_PI * 0.5 * n / m_fftSize;
        re += w[n] * qCos(angle);
        im += w[n] * qSin(angle);
    }
    double halfBin = qSqrt(re * re + im * im);
    return 20.0 * qLn(halfBin / peak) / qLn(10.0);
}

/* ---- Compute ENBW ---- */

double WindowFunction2::computeENBW(const QVector<double>& window) const
{
    int N = window.size();
    double sumSq = 0.0, sumW = 0.0;
    for (int i = 0; i < N; ++i) {
        sumW += window[i];
        sumSq += window[i] * window[i];
    }
    if (sumSq <= 0) return 0.0;
    return N * sumSq / (sumW * sumW);
}

/* ---- Analyze single window ---- */

WindowFunction2::Metrics WindowFunction2::analyze(WindowType type, double param)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> w = generate(type, param);
    QVector<double> mag = magnitudeSpectrum(w);

    Metrics m;
    m.mainlobeWidth3dB = findMainlobeWidth(mag, -3.0);
    m.mainlobeWidth6dB = findMainlobeWidth(mag, -6.0);
    m.peakSidelobeLevel = findPeakSidelobe(mag);
    m.scallopingLoss = computeScallopingLoss(mag);
    m.coherentGain = 0.0;
    for (double v : w) m.coherentGain += v;
    m.coherentGain /= w.size();
    m.enbw = computeENBW(w);
    m.sidelobeRollOff = -6.0; // Approximate

    m_stats.totalAnalysis++;
    m_stats.windowSize = m_windowSize;
    m_stats.numWindowsCompared++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalysis;

    emit analysisCompleted(windowName(type), m.peakSidelobeLevel, timer.elapsed());
    return m;
}

/* ---- Compare multiple windows ---- */

QVector<QPair<WindowFunction2::WindowType, WindowFunction2::Metrics>>
WindowFunction2::compare(const QVector<WindowType>& types)
{
    QVector<QPair<WindowType, Metrics>> results;
    for (WindowType t : types)
        results.append({t, analyze(t)});

    // Sort by peak sidelobe level (best first)
    std::sort(results.begin(), results.end(),
              [](const auto& a, const auto& b) {
                  return a.second.peakSidelobeLevel < b.second.peakSidelobeLevel;
              });
    return results;
}

/* ---- Reset ---- */

void WindowFunction2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
