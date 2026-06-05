/**
 * @file BiquadFilter.cpp
 * @brief 双二阶IIR滤波器实现
 */

#include "BiquadFilter.h"
#include <QElapsedTimer>
#include <cmath>

BiquadFilter::BiquadFilter(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void BiquadFilter::design(FilterType type, double freq, double sampleRate,
                           double Q, double gainDB)
{
    QElapsedTimer timer;
    timer.start();

    m_type = type;
    m_freq = freq;
    m_Q = Q;
    m_gainDB = gainDB;

    double w0 = 2.0 * M_PI * freq / sampleRate;
    double alpha = std::sin(w0) / (2.0 * Q);
    double A = std::pow(10.0, gainDB / 40.0);

    computeCookbook(type, w0, alpha, A);

    m_stats.totalFiltersDesigned++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalFiltersDesigned + m_stats.totalSamplesProcessed;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit filterDesigned(static_cast<int>(type), freq);
}

void BiquadFilter::computeCookbook(FilterType type, double w0, double alpha,
                                    double A)
{
    double cosw0 = std::cos(w0);

    switch (type) {
    case LowPass:
        m_b0 = (1.0 - cosw0) / 2.0;
        m_b1 = 1.0 - cosw0;
        m_b2 = (1.0 - cosw0) / 2.0;
        m_a1 = -2.0 * cosw0;
        m_a2 = 1.0 - alpha;
        break;

    case HighPass:
        m_b0 = (1.0 + cosw0) / 2.0;
        m_b1 = -(1.0 + cosw0);
        m_b2 = (1.0 + cosw0) / 2.0;
        m_a1 = -2.0 * cosw0;
        m_a2 = 1.0 - alpha;
        break;

    case BandPass:
        m_b0 = alpha;
        m_b1 = 0.0;
        m_b2 = -alpha;
        m_a1 = -2.0 * cosw0;
        m_a2 = 1.0 - alpha;
        break;

    case Notch:
        m_b0 = 1.0;
        m_b1 = -2.0 * cosw0;
        m_b2 = 1.0;
        m_a1 = -2.0 * cosw0;
        m_a2 = 1.0 - alpha;
        break;

    case AllPass:
        m_b0 = 1.0 - alpha;
        m_b1 = -2.0 * cosw0;
        m_b2 = 1.0 + alpha;
        m_a1 = -2.0 * cosw0;
        m_a2 = 1.0 - alpha;
        break;

    case Peaking:
        m_b0 = 1.0 + alpha * A;
        m_b1 = -2.0 * cosw0;
        m_b2 = 1.0 - alpha * A;
        m_a1 = -2.0 * cosw0;
        m_a2 = 1.0 - alpha / A;
        break;

    case LowShelf: {
        double sqA = std::sqrt(A);
        m_b0 = A * ((A + 1.0) - (A - 1.0) * cosw0 + 2.0 * sqA * alpha);
        m_b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosw0);
        m_b2 = A * ((A + 1.0) - (A - 1.0) * cosw0 - 2.0 * sqA * alpha);
        m_a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosw0);
        m_a2 = (A + 1.0) + (A - 1.0) * cosw0 - 2.0 * sqA * alpha;
        break;
    }

    case HighShelf: {
        double sqA = std::sqrt(A);
        m_b0 = A * ((A + 1.0) + (A - 1.0) * cosw0 + 2.0 * sqA * alpha);
        m_b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosw0);
        m_b2 = A * ((A + 1.0) + (A - 1.0) * cosw0 - 2.0 * sqA * alpha);
        m_a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosw0);
        m_a2 = (A + 1.0) - (A - 1.0) * cosw0 - 2.0 * sqA * alpha;
        break;
    }
    }

    /* 归一化: a0 = 1 */
    double a0 = 1.0 + alpha;
    if (type == LowShelf || type == HighShelf) {
        double sqA = std::sqrt(A);
        a0 = (A + 1.0) + (A - 1.0) * cosw0 + 2.0 * sqA * alpha;
    }
    m_b0 /= a0;
    m_b1 /= a0;
    m_b2 /= a0;
    m_a1 /= a0;
    m_a2 /= a0;
}

double BiquadFilter::process(double input)
{
    /* 直接I型转置(Direct Form I Transposed) */
    double output = m_b0 * input + m_z1;
    m_z1 = m_b1 * input - m_a1 * output + m_z2;
    m_z2 = m_b2 * input - m_a2 * output;

    m_stats.totalSamplesProcessed++;
    return output;
}

QVector<double> BiquadFilter::processBuffer(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i)
        output[i] = process(input[i]);

    m_timeSum += timer.elapsed();
    int total = m_stats.totalFiltersDesigned + m_stats.totalSamplesProcessed;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return output;
}

QPair<QVector<double>, QVector<double>> BiquadFilter::frequencyResponse(
    const QVector<double>& freqs, double sampleRate) const
{
    QVector<double> mag(freqs.size());
    QVector<double> phase(freqs.size());

    for (int i = 0; i < freqs.size(); ++i) {
        double w = 2.0 * M_PI * freqs[i] / sampleRate;
        double cosw = std::cos(w);
        double sinw = std::sin(w);

        /* H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2) */
        /* z = e^(jw) */
        double numReal = m_b0 + m_b1 * cosw + m_b2 * std::cos(2.0 * w);
        double numImag = -(m_b1 * sinw + m_b2 * std::sin(2.0 * w));
        double denReal = 1.0 + m_a1 * cosw + m_a2 * std::cos(2.0 * w);
        double denImag = -(m_a1 * sinw + m_a2 * std::sin(2.0 * w));

        double numMag = std::sqrt(numReal * numReal + numImag * numImag);
        double denMag = std::sqrt(denReal * denReal + denImag * denImag);
        mag[i] = denMag > 1e-20 ? numMag / denMag : 0.0;

        double numPhase = std::atan2(numImag, numReal);
        double denPhase = std::atan2(denImag, denReal);
        phase[i] = numPhase - denPhase;
    }

    return {mag, phase};
}

QVector<double> BiquadFilter::coefficients() const
{
    return {m_b0, m_b1, m_b2, m_a1, m_a2};
}

void BiquadFilter::reset()
{
    m_z1 = 0.0;
    m_z2 = 0.0;
}

QString BiquadFilter::filterTypeName() const
{
    switch (m_type) {
    case LowPass: return tr("低通");
    case HighPass: return tr("高通");
    case BandPass: return tr("带通");
    case Notch: return tr("陷波");
    case AllPass: return tr("全通");
    case Peaking: return tr("峰值");
    case LowShelf: return tr("低架");
    case HighShelf: return tr("高架");
    }
    return tr("未知");
}

void BiquadFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
