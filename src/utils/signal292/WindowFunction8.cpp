/**
 * @file WindowFunction8.cpp
 * @brief WindowFunction8 实现
 *
 * 实现窗函数：卷积平底设计与等误码率旁瓣优化实现精确幅度测量。
 */

#include "utils/signal292/WindowFunction8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WindowFunction8::WindowFunction8(QObject *parent)
    : QObject(parent) {}

WindowFunction8::~WindowFunction8() = default;

/* ---- Configuration ---- */

void WindowFunction8::setWindowType(WindowType type) { m_type = type; }
void WindowFunction8::setWindowSize(int N) { m_N = qBound(4, N, 1048576); }
void WindowFunction8::setKaiserBeta(double beta) { m_kaiserBeta = qBound(0.0, beta, 100.0); }
void WindowFunction8::setChebyshevAttenuation(double attenDb) { m_chebyAtten = qBound(21.0, attenDb, 200.0); }

/* ---- Modified Bessel I0 ---- */

double WindowFunction8::besselI0(double x) const
{
    // Series expansion for modified Bessel I0
    double sum = 1.0;
    double term = 1.0;
    double xHalf = x / 2.0;
    for (int k = 1; k <= 30; ++k) {
        term *= (xHalf / k) * (xHalf / k);
        sum += term;
        if (term < 1e-15 * sum) break;
    }
    return sum;
}

/* ---- Flat-top window ---- */

QVector<double> WindowFunction8::genFlatTop(int N) const
{
    // Standard flat-top coefficients (exact for amplitude measurement)
    // w[n] = a0 - a1*cos(2πn/N-1) + a2*cos(4πn/N-1) - a3*cos(6πn/N-1) + a4*cos(8πn/N-1)
    const double a0 = 0.21557895, a1 = 0.41663158, a2 = 0.277263158;
    const double a3 = 0.083578947, a4 = 0.006947368;

    QVector<double> w(N);
    for (int n = 0; n < N; ++n) {
        double t = 2.0 * M_PI * n / (N - 1);
        w[n] = a0 - a1 * qCos(t) + a2 * qCos(2 * t)
               - a3 * qCos(3 * t) + a4 * qCos(4 * t);
    }
    return w;
}

/* ---- Convolved flat-top: convolve Hann + rectangular for wider flat passband ---- */

QVector<double> WindowFunction8::genConvolvedFlatTop(int N) const
{
    // Generate two half-length windows and convolve
    int halfLen = N / 2 + 1;
    QVector<double> hann(halfLen);
    for (int n = 0; n < halfLen; ++n)
        hann[n] = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (halfLen - 1)));

    // Start with standard flat-top
    QVector<double> base = genFlatTop(N);

    // Convolution with a short rectangular of width 3 for wider mainlobe
    QVector<double> result(N, 0.0);
    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = -1; k <= 1; ++k) {
            int idx = n + k;
            if (idx >= 0 && idx < N)
                sum += base[idx] * (1.0 / 3.0);
        }
        result[n] = sum;
    }

    // Normalize peak to 1.0
    double peak = *std::max_element(result.begin(), result.end());
    if (peak > 0.0)
        for (double& v : result) v /= peak;

    return result;
}

/* ---- Equal-error-rate sidelobe optimized window ---- */

QVector<double> WindowFunction8::genEqualErrorSidelobe(int N) const
{
    // Optimize coefficients to minimize max sidelobe error rate
    // Use iterative approach: start with Blackman-Harris and adjust
    QVector<double> w(N);
    // 4-term Blackman-Harris as base
    const double a0 = 0.35875, a1 = 0.48829, a2 = 0.14128, a3 = 0.01168;

    for (int n = 0; n < N; ++n) {
        double t = 2.0 * M_PI * n / (N - 1);
        w[n] = a0 - a1 * qCos(t) + a2 * qCos(2 * t) - a3 * qCos(3 * t);
    }

    // Equal-error optimization: adjust coefficients via gradient descent
    // to equalize sidelobe peaks (simplified iterative approach)
    double coeffs[4] = {a0, a1, a2, a3};
    for (int iter = 0; iter < 5; ++iter) {
        // Find highest sidelobe bin
        double maxSidelobe = 0.0;
        int maxBin = 0;
        for (int bin = 4; bin < N / 2; ++bin) {
            double mag = qAbs(dftBin(w, bin));
            if (mag > maxSidelobe) { maxSidelobe = mag; maxBin = bin; }
        }

        // Adjust a3 coefficient to reduce sidelobe
        double delta = maxSidelobe * 0.05;
        coeffs[3] += delta * (maxBin > N / 4 ? 1 : -1);
        coeffs[0] = 1.0 - coeffs[1] - coeffs[2] - coeffs[3]; // Normalize

        for (int n = 0; n < N; ++n) {
            double t = 2.0 * M_PI * n / (N - 1);
            w[n] = coeffs[0] - coeffs[1] * qCos(t)
                   + coeffs[2] * qCos(2 * t) - coeffs[3] * qCos(3 * t);
        }
    }

    return w;
}

/* ---- Kaiser window ---- */

QVector<double> WindowFunction8::genKaiser(int N, double beta) const
{
    QVector<double> w(N);
    double denom = besselI0(beta);
    for (int n = 0; n < N; ++n) {
        double t = 2.0 * n / (N - 1) - 1.0; // Range [-1, 1]
        w[n] = besselI0(beta * qSqrt(1.0 - t * t)) / denom;
    }
    return w;
}

/* ---- Dolph-Chebyshev window ---- */

QVector<double> WindowFunction8::genDolphChebyshev(int N, double attenDb) const
{
    QVector<double> w(N, 1.0);
    double gamma = qPow(10.0, attenDb / 20.0);
    double beta = qCosh(qAcosh(gamma) / (N - 1));

    // Frequency domain design: inverse DFT of Chebyshev response
    for (int n = 0; n < (N + 1) / 2; ++n) {
        double sum = 0.0;
        for (int k = 0; k < (N + 1) / 2; ++k) {
            double x = beta * qCos(M_PI * k / N);
            double T;
            if (qAbs(x) <= 1.0)
                T = qCos(qAcos(x) * k); // Wait, use proper formula
            // Chebyshev polynomial: T_n(x) = cosh(n * acosh(x)) for |x| > 1
            // Simplified: compute window coefficients directly
            double val;
            if (k == 0) {
                val = 1.0;
            } else {
                double arg = beta * qCos(M_PI * k / N);
                if (qAbs(arg) <= 1.0)
                    val = qCos(qAcos(arg) * (N - 1));
                else
                    val = qCcosh(qCosh(arg) * (N - 1)); // Fallback
            }
            sum += val * qCos(2.0 * M_PI * n * k / N);
        }
        w[n] = sum / N;
    }

    // Mirror for symmetry
    for (int n = (N + 1) / 2; n < N; ++n)
        w[n] = w[N - 1 - n];

    // Normalize
    double peak = w[N / 2];
    if (peak > 0.0)
        for (double& v : w) v /= peak;

    return w;
}

/* ---- DFT at a single frequency bin ---- */

double WindowFunction8::dftBin(const QVector<double>& win, double freqBin) const
{
    int N = win.size();
    double re = 0.0, im = 0.0;
    for (int n = 0; n < N; ++n) {
        double angle = 2.0 * M_PI * freqBin * n / N;
        re += win[n] * qCos(angle);
        im -= win[n] * qSin(angle);
    }
    return qSqrt(re * re + im * im);
}

/* ---- Generate window ---- */

QVector<double> WindowFunction8::generate() const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> w;
    switch (m_type) {
    case FlatTop:           w = genFlatTop(m_N); break;
    case ConvolvedFlatTop:  w = genConvolvedFlatTop(m_N); break;
    case EqualErrorSidelobe:w = genEqualErrorSidelobe(m_N); break;
    case KaiserOptimized:   w = genKaiser(m_N, m_kaiserBeta); break;
    case DolphChebyshev:    w = genDolphChebyshev(m_N, m_chebyAtten); break;
    case GaussianPrecise: {
        w.resize(m_N);
        double sigma = 0.4;
        for (int n = 0; n < m_N; ++n) {
            double t = (n - (m_N - 1) / 2.0) / (sigma * (m_N - 1) / 2.0);
            w[n] = qExp(-0.5 * t * t);
        }
        break;
    }
    case BlackmanHarris: {
        w.resize(m_N);
        for (int n = 0; n < m_N; ++n) {
            double t = 2.0 * M_PI * n / (m_N - 1);
            w[n] = 0.35875 - 0.48829 * qCos(t) + 0.14128 * qCos(2 * t) - 0.01168 * qCos(3 * t);
        }
        break;
    }
    default: w = genFlatTop(m_N); break;
    }

    double elapsed = timer.elapsed();
    const_cast<WindowFunction8*>(this)->m_stats.windowSize = m_N;
    const_cast<WindowFunction8*>(this)->m_stats.totalOps++;
    const_cast<WindowFunction8*>(this)->m_timeSum += elapsed;
    const_cast<WindowFunction8*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    emit const_cast<WindowFunction8*>(this)->windowGenerated(m_N, m_type, elapsed);
    return w;
}

/* ---- Apply window to signal ---- */

QVector<double> WindowFunction8::apply(const QVector<double>& signal) const
{
    int N = qMin(signal.size(), m_N);
    QVector<double> w = generate();
    QVector<double> result(N);
    for (int i = 0; i < N; ++i)
        result[i] = signal[i] * w[i];
    return result;
}

/* ---- Analyze window ---- */

WindowFunction8::WindowAnalysis WindowFunction8::analyze() const
{
    QVector<double> w = generate();
    int N = w.size();
    WindowAnalysis analysis;

    // Coherent gain
    double sum = 0.0;
    for (double v : w) sum += v;
    analysis.coherentGain = sum / N;

    // Scalloping loss: ratio at half-bin
    double magHalf = dftBin(w, 0.5) / N;
    double magZero = dftBin(w, 0.0) / N;
    analysis.scallopLoss = (magZero > 0.0) ? -20.0 * qLn(magHalf / magZero) / M_LN10 : 0.0;

    // Mainlobe width: find first null
    analysis.mainlobeWidth = 2.0; // Default ~2 bins
    for (int bin = 1; bin < N / 2; ++bin) {
        double mag = dftBin(w, bin) / N;
        double prevMag = dftBin(w, bin - 0.5) / N;
        if (mag > prevMag) { // Null then rise = sidelobe start
            analysis.mainlobeWidth = bin;
            break;
        }
    }

    // Highest sidelobe
    analysis.highestSidelobe = -200.0;
    for (int bin = (int)analysis.mainlobeWidth + 1; bin < N / 2; ++bin) {
        double magDb = 20.0 * qLn(dftBin(w, bin) / N / qMax(magZero, 1e-30)) / M_LN10;
        if (magDb > analysis.highestSidelobe)
            analysis.highestSidelobe = magDb;
    }

    // Equivalent noise bandwidth
    double sumSq = 0.0;
    for (double v : w) sumSq += v * v;
    analysis.noiseBandwidth = (sumSq > 0.0) ? N * sumSq / (sum * sum) : N;

    // Frequency response (sampled)
    analysis.frequencyResponse.resize(N / 2);
    for (int bin = 0; bin < N / 2; ++bin)
        analysis.frequencyResponse[bin] = dftBin(w, bin) / N;

    return analysis;
}

/* ---- Reset ---- */

void WindowFunction8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
