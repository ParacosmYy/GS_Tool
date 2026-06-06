/**
 * @file Resampler2.cpp
 * @brief Resampler2 实现
 *
 * 实现采样率转换：多相FIR抗混叠、Farrow结构任意比率插值。
 */

#include "utils/signal180/Resampler2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Resampler2::Resampler2(QObject *parent) : QObject(parent) {}
Resampler2::~Resampler2() = default;

/* ---- Configuration ---- */

void Resampler2::setInputRate(int rate) { m_inputRate = qMax(1, rate); }
void Resampler2::setOutputRate(int rate) { m_outputRate = qMax(1, rate); }
void Resampler2::setFilterTaps(int taps) { m_filterTaps = qMax(4, taps); }
void Resampler2::setInterpolation(Interpolation type) { m_interp = type; }

/* ---- Design lowpass FIR via windowed sinc ---- */

QVector<double> Resampler2::designFilter(double cutoff, int taps) const
{
    QVector<double> h(taps);
    int mid = taps / 2;
    for (int n = 0; n < taps; ++n) {
        double x = n - mid;
        double sinc = (qAbs(x) < 1e-10) ? 1.0 : qSin(M_PI * cutoff * x) / (M_PI * x);
        /* Hann window */
        double win = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (taps - 1)));
        h[n] = sinc * win;
    }
    /* Normalize */
    double sum = 0.0;
    for (double v : h) sum += v;
    if (sum > 1e-12)
        for (int i = 0; i < taps; ++i) h[i] /= sum;
    return h;
}

/* ---- Polyphase FIR filter for rational ratio ---- */

QVector<double> Resampler2::polyphaseFilter(const QVector<double>& input,
                                               int upFactor, int downFactor) const
{
    /* Cutoff = 1 / max(upFactor, downFactor) for anti-aliasing */
    double cutoff = 1.0 / qMax(upFactor, downFactor);
    auto h = designFilter(cutoff, m_filterTaps);

    int n = input.size();
    /* Upsample: insert (upFactor-1) zeros between samples */
    int upLen = n * upFactor;
    QVector<double> upsampled(upLen, 0.0);
    for (int i = 0; i < n; ++i)
        upsampled[i * upFactor] = input[i];

    /* Apply FIR filter */
    int outLen = (upLen + downFactor - 1) / downFactor;
    QVector<double> output(outLen);

    for (int i = 0; i < outLen; ++i) {
        int idx = i * downFactor;
        double sum = 0.0;
        for (int k = 0; k < h.size() && idx - k >= 0; ++k)
            sum += upsampled[idx - k] * h[k];
        output[i] = sum * upFactor; /* Gain compensation */
    }
    return output;
}

/* ---- Farrow cubic interpolation coefficient ---- */

double Resampler2::farrowCoeff(const QVector<double>& y, double mu) const
{
    /* 4-point cubic: y[0], y[1], y[2], y[3], fractional offset mu in [0,1) */
    if (y.size() < 4) return y.value(1, 0.0);
    double a0 = y[1];
    double a1 = 0.5 * (y[2] - y[0]);
    double a2 = y[0] - 2.5 * y[1] + 2.0 * y[2] - 0.5 * y[3];
    double a3 = 0.5 * (-y[0] + 3.0 * y[1] - 3.0 * y[2] + y[3]);
    return a0 + a1 * mu + a2 * mu * mu + a3 * mu * mu * mu;
}

/* ---- Farrow structure interpolation ---- */

QVector<double> Resampler2::farrowInterpolate(const QVector<double>& input,
                                                 double ratio) const
{
    int n = input.size();
    if (n < 4) return input;

    int outLen = qFloor(n * ratio);
    QVector<double> output(outLen);

    for (int i = 0; i < outLen; ++i) {
        double t = i / ratio;
        int idx = qFloor(t);
        double mu = t - idx;

        /* Clamp indices */
        int i0 = qMax(0, idx - 1);
        int i1 = qMax(0, qMin(n - 1, idx));
        int i2 = qMax(0, qMin(n - 1, idx + 1));
        int i3 = qMax(0, qMin(n - 1, idx + 2));

        QVector<double> y = {input[i0], input[i1], input[i2], input[i3]};
        output[i] = farrowCoeff(y, mu);
    }
    return output;
}

/* ---- Main process ---- */

QVector<double> Resampler2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0 || m_inputRate <= 0 || m_outputRate <= 0) return {};

    double ratio = static_cast<double>(m_outputRate) / m_inputRate;
    QVector<double> output;

    if (m_interp == Farrow) {
        /* Farrow structure: direct arbitrary ratio */
        output = farrowInterpolate(input, ratio);
    } else {
        /* Polyphase approach: find rational approximation */
        int up = m_outputRate;
        int down = m_inputRate;

        /* GCD to reduce fraction */
        int a = up, b = down;
        while (b > 0) { int t = b; b = a % b; a = t; }
        up /= a;
        down /= a;

        output = polyphaseFilter(input, up, down);

        /* Additional interpolation for residual fractional ratio */
        if (m_interp == Cubic && output.size() > 0) {
            int target = qFloor(n * ratio);
            if (qAbs(output.size() - target) > 1) {
                double adjRatio = static_cast<double>(target) / output.size();
                output = farrowInterpolate(output, adjRatio);
            }
        }
    }

    m_stats.totalResamples++;
    m_stats.inputRate = m_inputRate;
    m_stats.outputRate = m_outputRate;
    m_stats.inputSamples = n;
    m_stats.outputSamples = output.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalResamples;

    emit resampleCompleted(n, output.size());
    return output;
}

/* ---- Reset ---- */

void Resampler2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
