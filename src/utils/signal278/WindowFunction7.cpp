/**
 * @file WindowFunction7.cpp
 * @brief WindowFunction7 实现
 *
 * 实现窗函数：Dolph-Chebyshev等旁瓣设计与可调alpha高斯窗的频谱分析。
 */

#include "utils/signal278/WindowFunction7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WindowFunction7::WindowFunction7(QObject *parent)
    : QObject(parent)
{
    m_coeffs = generate();
}

WindowFunction7::~WindowFunction7() = default;

/* ---- Configuration ---- */

void WindowFunction7::setLength(int n) { m_length = qBound(4, n, 131072); }
void WindowFunction7::setWindowType(WindowType type) { m_type = type; }
void WindowFunction7::setChebyshevSLL(double sllDb) { m_chebyshevSLL = qBound(-120.0, sllDb, -10.0); }
void WindowFunction7::setGaussianAlpha(double alpha) { m_gaussianAlpha = qBound(0.5, alpha, 20.0); }
void WindowFunction7::setKaiserBeta(double beta) { m_kaiserBeta = qBound(0.0, beta, 50.0); }

/* ---- Bessel I0(x) ---- */

double WindowFunction7::besselI0(double x) const
{
    double sum = 1.0;
    double term = 1.0;
    for (int k = 1; k < 50; ++k) {
        double halfX = x / 2.0;
        term *= (halfX / k) * (halfX / k);
        sum += term;
        if (term < 1e-15 * sum) break;
    }
    return sum;
}

/* ---- Chebyshev polynomial T_n(x) ---- */

double WindowFunction7::chebyshevPoly(int n, double x) const
{
    if (n == 0) return 1.0;
    if (n == 1) return x;
    double tPrev2 = 1.0, tPrev1 = x, tCur = 0.0;
    for (int i = 2; i <= n; ++i) {
        tCur = 2.0 * x * tPrev1 - tPrev2;
        tPrev2 = tPrev1;
        tPrev1 = tCur;
    }
    return tCur;
}

/* ---- Dolph-Chebyshev window ---- */

QVector<double> WindowFunction7::generateDolphChebyshev(int n, double sllDb) const
{
    QVector<double> w(n, 0.0);
    double sllLinear = qPow(10.0, sllDb / 20.0);
    double beta = qCosh(qAcosh(1.0 / sllLinear) / (n - 1));

    // Compute in frequency domain using IDFT approach
    // W[k] = Chebyshev polynomial evaluated at beta * cos(pi*k/N)
    int N = n;
    QVector<double> W(N);
    for (int k = 0; k < N; ++k) {
        double arg = beta * qCos(M_PI * k / N);
        if (qAbs(arg) <= 1.0)
            W[k] = chebyshevPoly(N - 1, arg);
        else
            W[k] = (arg > 0 ? 1.0 : -1.0) * qPow(beta * qSqrt(arg * arg - 1.0), N - 1);
    }
    // Normalize peak to 1
    double maxW = 0.0;
    for (double v : W) maxW = qMax(maxW, qAbs(v));
    if (maxW > 0) for (double& v : W) v /= maxW;

    // Inverse DFT to get time-domain window
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k) {
            sum += W[k] * qCos(2.0 * M_PI * k * i / N);
        }
        w[i] = sum / N;
    }

    // Normalize peak
    double maxVal = 0.0;
    for (double v : w) maxVal = qMax(maxVal, qAbs(v));
    if (maxVal > 0) for (double& v : w) v /= maxVal;

    return w;
}

/* ---- Gaussian window ---- */

QVector<double> WindowFunction7::generateGaussian(int n, double alpha) const
{
    QVector<double> w(n);
    double halfN = (n - 1) / 2.0;
    for (int i = 0; i < n; ++i) {
        double t = (i - halfN) / halfN;  // Normalized [-1, 1]
        w[i] = qExp(-0.5 * (alpha * t) * (alpha * t));
    }
    return w;
}

/* ---- Kaiser window ---- */

QVector<double> WindowFunction7::generateKaiser(int n, double beta) const
{
    QVector<double> w(n);
    double denom = besselI0(beta);
    for (int i = 0; i < n; ++i) {
        double t = 2.0 * i / (n - 1) - 1.0;  // [-1, 1]
        double arg = beta * qSqrt(1.0 - t * t);
        w[i] = besselI0(arg) / denom;
    }
    return w;
}

/* ---- Standard cosine window ---- */

QVector<double> WindowFunction7::generateCosine(int n, const QVector<double>& coeffs) const
{
    QVector<double> w(n);
    for (int i = 0; i < n; ++i) {
        double val = 0.0;
        for (int k = 0; k < coeffs.size(); ++k) {
            double sign = (k % 2 == 0) ? 1.0 : -1.0;
            val += sign * coeffs[k] * qCos(2.0 * M_PI * k * i / (n - 1));
        }
        w[i] = val;
    }
    return w;
}

/* ---- Generate window ---- */

QVector<double> WindowFunction7::generate()
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> w;

    switch (m_type) {
    case DolphChebyshev:
        w = generateDolphChebyshev(m_length, m_chebyshevSLL);
        break;
    case Gaussian:
        w = generateGaussian(m_length, m_gaussianAlpha);
        break;
    case Kaiser:
        w = generateKaiser(m_length, m_kaiserBeta);
        break;
    case Hann:
        w = generateCosine(m_length, {0.5, 0.5});
        break;
    case Hamming:
        w = generateCosine(m_length, {0.54, 0.46});
        break;
    case BlackmanHarris:
        w = generateCosine(m_length, {0.35875, 0.48829, 0.14128, 0.01168});
        break;
    case FlatTop:
        w = generateCosine(m_length, {0.21557895, 0.41663158, 0.277263158, 0.083578947, 0.006947368});
        break;
    case FlattenedCosine:
        w = generateCosine(m_length, {0.355768, 0.487396, 0.144232, 0.012604});
        break;
    }

    m_coeffs = w;

    double elapsed = timer.elapsed();
    m_stats.windowLength = m_length;
    m_stats.numWindows++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit windowGenerated(m_length, static_cast<int>(m_type), 0.0, elapsed);

    return w;
}

/* ---- Apply window to signal ---- */

QVector<double> WindowFunction7::apply(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(signal.size(), m_coeffs.size());
    QVector<double> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = signal[i] * m_coeffs[i];

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Compute window metrics ---- */

WindowFunction7::WindowMetrics WindowFunction7::computeMetrics()
{
    WindowMetrics m;
    int n = m_coeffs.size();
    if (n == 0) return m;

    // Coherent gain
    double sum = 0.0;
    for (double c : m_coeffs) sum += c;
    m.coherentGain = sum / n;

    // ENBW (Equivalent Noise Bandwidth)
    double sumSq = 0.0;
    for (double c : m_coeffs) sumSq += c * c;
    m.ENBW = (n * sumSq) / (sum * sum);

    // Scalloping loss (response at half-bin offset)
    double halfBinSum = 0.0;
    for (int i = 0; i < n; ++i) {
        double t = 2.0 * M_PI * (i - (n - 1) / 2.0) / n;
        halfBinSum += m_coeffs[i] * qCos(t * 0.5);
    }
    m.scallopingLoss = (sum > 0) ? -20.0 * qLn(qAbs(halfBinSum / sum)) / M_LN10 : 0.0;

    // Compute magnitude spectrum via DFT (centered)
    int fftSize = qMax(n, 512);
    QVector<double> mag(fftSize / 2, 0.0);
    for (int k = 0; k < fftSize / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = 2.0 * M_PI * k * i / fftSize;
            re += m_coeffs[i] * qCos(angle);
            im -= m_coeffs[i] * qSin(angle);
        }
        mag[k] = qSqrt(re * re + im * im);
    }

    // Normalize to dB
    double maxMag = 0.0;
    for (double v : mag) maxMag = qMax(maxMag, v);

    if (maxMag > 0) {
        QVector<double> magDb(fftSize / 2);
        for (int k = 0; k < fftSize / 2; ++k)
            magDb[k] = 20.0 * qLn(mag[k] / maxMag) / M_LN10;

        // Find 3dB and 6dB mainlobe widths
        int peakK = 0;
        for (int k = 0; k < fftSize / 2; ++k)
            if (mag[k] == maxMag) { peakK = k; break; }

        auto findWidth = [&](double dbThresh) -> double {
            int left = peakK, right = peakK;
            while (left > 0 && magDb[left] > dbThresh) left--;
            while (right < fftSize / 2 - 1 && magDb[right] > dbThresh) right++;
            return static_cast<double>(right - left);
        };
        m.mainLobeWidth3dB = findWidth(-3.0);
        m.mainLobeWidth6dB = findWidth(-6.0);

        // Peak sidelobe level
        double peakSLL = -200.0;
        // Find first null after mainlobe
        int nullK = peakK + 1;
        while (nullK < fftSize / 2 - 1 && magDb[nullK] > -40.0) nullK++;
        // Search for peak sidelobe
        for (int k = nullK; k < fftSize / 2; ++k) {
            if (magDb[k] > peakSLL) peakSLL = magDb[k];
        }
        m.sideLobeLevel = peakSLL;
    }

    return m;
}

/* ---- Get coefficients ---- */

QVector<double> WindowFunction7::coefficients() const { return m_coeffs; }

/* ---- Reset ---- */

void WindowFunction7::resetStatistics()
{
    m_coeffs.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
