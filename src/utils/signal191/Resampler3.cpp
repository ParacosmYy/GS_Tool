/**
 * @file Resampler3.cpp
 * @brief Resampler3 实现
 *
 * 实现重采样器：多相抗混叠滤波器组、Sinc插值、线性/三次回退、分数倍转换。
 */

#include "utils/signal191/Resampler3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Resampler3::Resampler3(QObject *parent) : QObject(parent)
{
    m_historySize = m_filterTaps;
    m_history.resize(m_historySize, 0.0);
    buildPolyPhase();
}

Resampler3::~Resampler3() = default;

/* ---- Configuration ---- */

void Resampler3::setRatio(double ratio)
{
    m_ratio = qMax(0.01, ratio);
    buildPolyPhase();
}

void Resampler3::setMode(Mode mode) { m_mode = mode; }
void Resampler3::setFilterTaps(int taps) { m_filterTaps = qMax(8, taps); buildPolyPhase(); }
void Resampler3::setSincLobes(int lobes) { m_sincLobes = qMax(1, lobes); buildPolyPhase(); }

/* ---- Sinc function ---- */

double Resampler3::sinc(double x) const
{
    if (qAbs(x) < 1e-10) return 1.0;
    double px = M_PI * x;
    return qSin(px) / px;
}

/* ---- Blackman window ---- */

double Resampler3::blackman(int n, int N) const
{
    double x = static_cast<double>(n) / (N - 1);
    return 0.42 - 0.5 * qCos(2.0 * M_PI * x) + 0.08 * qCos(4.0 * M_PI * x);
}

/* ---- Build polyphase filter bank ---- */

void Resampler3::buildPolyPhase()
{
    // Number of phases = filterTaps for fine resolution
    m_numPhases = m_filterTaps;
    int halfLen = m_sincLobes * m_filterTaps;
    int filterLen = 2 * halfLen + 1;

    // Design lowpass filter cutoff
    double cutoff = qMin(1.0, 1.0 / m_ratio);

    // Build prototype filter (windowed sinc)
    QVector<double> proto(filterLen);
    double sum = 0.0;
    for (int i = 0; i < filterLen; ++i) {
        double n = i - halfLen;
        proto[i] = cutoff * sinc(cutoff * n) * blackman(i, filterLen);
        sum += proto[i];
    }
    // Normalize
    for (auto& v : proto) v /= sum;

    // Decompose into polyphase components
    m_polyPhase.resize(m_numPhases);
    for (int p = 0; p < m_numPhases; ++p) {
        int tapsPerPhase = (filterLen + m_numPhases - 1) / m_numPhases;
        m_polyPhase[p].resize(tapsPerPhase, 0.0);
        for (int t = 0; t < tapsPerPhase; ++t) {
            int idx = p + t * m_numPhases;
            if (idx < filterLen) m_polyPhase[p][t] = proto[idx];
        }
    }

    m_historySize = halfLen + 1;
    m_history.resize(m_historySize, 0.0);
}

/* ---- Linear interpolation ---- */

double Resampler3::interpLinear(double frac, double y0, double y1) const
{
    return y0 + frac * (y1 - y0);
}

/* ---- Cubic (Hermite) interpolation ---- */

double Resampler3::interpCubic(double frac, double y0, double y1,
                               double y2, double y3) const
{
    double a = -0.5 * y0 + 1.5 * y1 - 1.5 * y2 + 0.5 * y3;
    double b = y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3;
    double c = -0.5 * y0 + 0.5 * y2;
    double d = y1;
    return a * frac * frac * frac + b * frac * frac + c * frac + d;
}

/* ---- Process block ---- */

QVector<double> Resampler3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return {};

    int nIn = input.size();
    int nOut = static_cast<int>(qCeil(nIn * m_ratio));
    QVector<double> output(nOut);

    int histLen = m_historySize;

    for (int i = 0; i < nOut; ++i) {
        // Compute position in input
        double inPos = m_phaseAccum;
        int inIdx = static_cast<int>(inPos);
        double frac = inPos - inIdx;

        if (m_mode == Linear) {
            int i0 = qBound(0, inIdx, nIn - 1);
            int i1 = qBound(0, inIdx + 1, nIn - 1);
            output[i] = interpLinear(frac, input[i0], input[i1]);

        } else if (m_mode == Cubic) {
            int i0 = qBound(0, inIdx - 1, nIn - 1);
            int i1 = qBound(0, inIdx, nIn - 1);
            int i2 = qBound(0, inIdx + 1, nIn - 1);
            int i3 = qBound(0, inIdx + 2, nIn - 1);
            output[i] = interpCubic(frac, input[i0], input[i1], input[i2], input[i3]);

        } else {
            // Sinc / polyphase interpolation
            int phaseIdx = static_cast<int>(frac * m_numPhases);
            phaseIdx = qBound(0, phaseIdx, m_numPhases - 1);
            const auto& phase = m_polyPhase[phaseIdx];
            double sum = 0.0;
            for (int t = 0; t < phase.size(); ++t) {
                int si = inIdx - static_cast<int>(phase.size()) / 2 + t;
                double s = (si >= 0 && si < nIn) ? input[si] : 0.0;
                sum += phase[t] * s;
            }
            output[i] = sum;
        }

        m_phaseAccum += 1.0 / m_ratio;
    }

    // Keep fractional part for streaming
    m_phaseAccum -= nIn;

    m_stats.totalSamplesIn += nIn;
    m_stats.totalSamplesOut += nOut;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSamplesOut > 0)
        ? m_timeSum * 1000.0 / m_stats.totalSamplesOut : 0.0;

    emit resamplingCompleted(nIn, nOut, timer.elapsed());
    return output;
}

/* ---- Reset state ---- */

void Resampler3::reset()
{
    m_history.fill(0.0);
    m_phaseAccum = 0.0;
}

/* ---- Reset statistics ---- */

void Resampler3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    reset();
}
