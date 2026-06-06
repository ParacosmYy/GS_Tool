/**
 * @file WindowFunction.cpp
 * @brief WindowFunction 实现
 *
 * 实现窗函数生成：Hann/Hamming/Blackman/Kaiser/Bartlett/Flat-top、SCT分析。
 */

#include "utils/signal178/WindowFunction.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

WindowFunction::WindowFunction(QObject *parent)
    : QObject(parent)
{
}

WindowFunction::~WindowFunction() = default;

/* ---- Configuration ---- */

void WindowFunction::setWindowType(WindowType type) { m_type = type; }
void WindowFunction::setKaiserBeta(double beta) { m_kaiserBeta = qMax(0.0, beta); }

/* ---- Modified Bessel I0 ---- */

double WindowFunction::besselI0(double x)
{
    /* Series expansion for modified Bessel function I0(x) */
    double sum = 1.0;
    double term = 1.0;
    for (int k = 1; k <= 25; ++k) {
        term *= (x / (2.0 * k)) * (x / (2.0 * k));
        sum += term;
        if (term < 1e-15 * sum) break;
    }
    return sum;
}

/* ---- Window value at sample n ---- */

double WindowFunction::windowValue(WindowType type, int n, int N) const
{
    double nn = static_cast<double>(n);
    double NN = static_cast<double>(N - 1);

    switch (type) {
    case Hann:
        return 0.5 * (1.0 - qCos(2.0 * M_PI * nn / NN));
    case Hamming:
        return 0.54 - 0.46 * qCos(2.0 * M_PI * nn / NN);
    case Blackman:
        return 0.42 - 0.5 * qCos(2.0 * M_PI * nn / NN) +
               0.08 * qCos(4.0 * M_PI * nn / NN);
    case Kaiser: {
        double arg = m_kaiserBeta * qSqrt(1.0 - qPow((nn - NN / 2.0) / (NN / 2.0), 2));
        return besselI0(arg) / besselI0(m_kaiserBeta);
    }
    case Bartlett:
        return 1.0 - qAbs(2.0 * nn / NN - 1.0);
    case FlatTop:
        return 0.21557895 - 0.41663158 * qCos(2.0 * M_PI * nn / NN) +
               0.277263158 * qCos(4.0 * M_PI * nn / NN) -
               0.083578947 * qCos(6.0 * M_PI * nn / NN) +
               0.006947368 * qCos(8.0 * M_PI * nn / NN);
    case Rectangular:
        return 1.0;
    case BlackmanHarris:
        return 0.35875 - 0.48829 * qCos(2.0 * M_PI * nn / NN) +
               0.14128 * qCos(4.0 * M_PI * nn / NN) -
               0.01168 * qCos(6.0 * M_PI * nn / NN);
    }
    return 1.0;
}

/* ---- Generate ---- */

QVector<double> WindowFunction::generate(int N) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> w(N);
    for (int n = 0; n < N; ++n)
        w[n] = windowValue(m_type, n, N);

    m_stats.totalGenerated++;
    m_stats.lastSize = N;
    const_cast<double&>(m_timeSum) += timer.elapsed();

    emit const_cast<WindowFunction*>(this)->windowGenerated(N, m_type);
    return w;
}

/* ---- Apply ---- */

QVector<double> WindowFunction::apply(const QVector<double>& signal) const
{
    int N = signal.size();
    QVector<double> w = generate(N);
    QVector<double> result(N);
    for (int i = 0; i < N; ++i)
        result[i] = signal[i] * w[i];
    return result;
}

/* ---- Frequency response ---- */

QVector<double> WindowFunction::frequencyResponse(int N, int fftSize) const
{
    if (fftSize <= 0) fftSize = 4096;

    QVector<double> w = generate(N);

    /* Zero-pad and compute DFT magnitude */
    int M = fftSize;
    QVector<double> response(M / 2 + 1, 0.0);

    for (int k = 0; k <= M / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / M;
            re += w[n] * qCos(angle);
            im += w[n] * qSin(angle);
        }
        response[k] = qSqrt(re * re + im * im);
    }

    /* Normalize to dB */
    double peak = 0.0;
    for (int k = 0; k <= M / 2; ++k)
        peak = qMax(peak, response[k]);

    for (int k = 0; k <= M / 2; ++k)
        response[k] = 20.0 * qLn(qMax(1e-10, response[k] / peak)) / M_LN2;

    return response;
}

/* ---- SCT Analysis ---- */

WindowFunction::SCTAnalysis WindowFunction::analyze(int N) const
{
    SCTAnalysis sct;
    QVector<double> w = generate(N);

    /* Coherent gain */
    double sum = 0.0;
    for (int n = 0; n < N; ++n) sum += w[n];
    sct.coherentGain = sum / N;

    /* ENBW (Equivalent Noise Bandwidth) */
    double sumSq = 0.0;
    for (int n = 0; n < N; ++n) sumSq += w[n] * w[n];
    sct.enbw = N * sumSq / (sum * sum);

    /* Processing gain */
    sct.processingGain = 10.0 * qLn(sct.enbw > 0 ? N / sct.enbw : 1.0) / M_LN2;

    /* Frequency response for sidelobe analysis */
    int fftSize = qMax(4096, N * 4);
    QVector<double> resp = frequencyResponse(N, fftSize);

    /* Sidelobe level: max dB after mainlobe */
    /* Find mainlobe edge (first null) */
    int mainlobeBins = 1;
    bool passedNull = false;
    for (int k = 2; k < resp.size(); ++k) {
        if (resp[k] < resp[k - 1] && resp[k] < -40.0) {
            passedNull = true;
            break;
        }
        mainlobeBins++;
    }
    sct.mainlobeWidth = mainlobeBins;

    /* Max sidelobe level */
    double maxSidelobe = -200.0;
    for (int k = mainlobeBins + 1; k < resp.size(); ++k)
        maxSidelobe = qMax(maxSidelobe, resp[k]);
    sct.sidelobeLevel = maxSidelobe;

    /* Scallop loss: worst-case amplitude at half-bin offset */
    /* Approximate as response at bin 0.5 */
    int halfBin = qMax(1, fftSize / (2 * N));
    if (halfBin < resp.size()) {
        double halfResp = resp[halfBin];
        sct.scallopLoss = -halfResp;
    }

    return sct;
}

/* ---- Generate all window types ---- */

QVector<QPair<WindowFunction::WindowType, QVector<double>>>
WindowFunction::generateAll(int N) const
{
    QVector<QPair<WindowType, QVector<double>>> all;
    WindowType types[] = {Hann, Hamming, Blackman, Kaiser, Bartlett, FlatTop,
                          Rectangular, BlackmanHarris};
    for (WindowType t : types) {
        QVector<double> w(N);
        for (int n = 0; n < N; ++n)
            w[n] = windowValue(t, n, N);
        all.append({t, w});
    }
    return all;
}

/* ---- Reset ---- */

void WindowFunction::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
