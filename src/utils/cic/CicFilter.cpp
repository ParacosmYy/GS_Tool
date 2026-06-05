/**
 * @file CicFilter.cpp
 * @brief CIC滤波器实现 — 积分梳状抽取/插值
 */

#include "utils/cic/CicFilter.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
CicFilter::CicFilter(QObject* parent)
    : QObject(parent)
    , m_order(1)
    , m_rate(1)
    , m_diffDelay(1)
    , m_timeSum(0.0)
{
}

/** @brief 配置CIC参数 */
void CicFilter::configure(int order, int rate, int diffDelay)
{
    m_order = qMax(1, order);
    m_rate = qMax(2, rate);
    m_diffDelay = qMax(1, diffDelay);
    reset();
}

/** @brief CIC抽取 */
QVector<double> CicFilter::decimate(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    reset();

    int outSize = input.size() / m_rate;
    QVector<double> output;
    output.reserve(outSize);

    /* 积分器阶段 */
    QVector<double> integState(m_order, 0.0);
    QVector<double> combState(m_order, 0.0);
    QVector<double> combDelay(m_order, 0.0);

    int outIdx = 0;
    for (int i = 0; i < input.size(); ++i) {
        double val = input[i];

        /* 级联积分器 */
        for (int s = 0; s < m_order; ++s) {
            integState[s] += val;
            val = integState[s];
        }

        /* 抽取 */
        if ((i % m_rate) == 0) {
            double y = val;

            /* 级联梳状器 */
            for (int s = 0; s < m_order; ++s) {
                double prev = combDelay[s];
                combDelay[s] = y;
                y = y - prev;
            }

            output.append(y);
            ++outIdx;
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.totalSamples += static_cast<quint64>(input.size());
    double total = static_cast<double>(m_stats.totalSamples);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit processingCompleted(input.size(), output.size());
    return output;
}

/** @brief CIC插值 */
QVector<double> CicFilter::interpolate(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    reset();

    QVector<double> output;
    output.reserve(input.size() * m_rate);

    QVector<double> combState(m_order, 0.0);
    QVector<double> integState(m_order, 0.0);

    for (int i = 0; i < input.size(); ++i) {
        double val = input[i];

        /* 梳状器(在零填充前) */
        for (int s = 0; s < m_order; ++s) {
            double prev = combState[s];
            combState[s] = val;
            val = val - prev;
        }

        /* 零填充插值 */
        for (int j = 0; j < m_rate; ++j) {
            double interp = (j == 0) ? val : 0.0;

            /* 级联积分器 */
            for (int s = 0; s < m_order; ++s) {
                integState[s] += interp;
                interp = integState[s];
            }

            output.append(interp);
        }
    }

    /* 增益补偿 */
    double gain = qPow(static_cast<double>(m_rate * m_diffDelay),
                       static_cast<double>(m_order));
    for (auto& v : output)
        v /= gain;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.totalSamples += static_cast<quint64>(input.size());
    double total = static_cast<double>(m_stats.totalSamples);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit processingCompleted(input.size(), output.size());
    return output;
}

/** @brief 频率响应 */
QVector<double> CicFilter::frequencyResponse(const QVector<double>& freqs) const
{
    QVector<double> response;
    response.reserve(freqs.size());

    double gain = qPow(static_cast<double>(m_rate * m_diffDelay),
                       static_cast<double>(m_order));

    for (double f : freqs) {
        double w = 2.0 * M_PI * f;
        double hVal = 1.0;

        /* |H(f)| = |sin(pi*R*M*f) / sin(pi*M*f)|^N */
        double num = qSin(M_PI * m_rate * m_diffDelay * f);
        double den = qSin(M_PI * m_diffDelay * f);

        if (qAbs(den) < 1e-15) {
            hVal = qPow(static_cast<double>(m_rate), static_cast<double>(m_order));
        } else {
            hVal = qPow(qAbs(num / den), static_cast<double>(m_order));
        }

        double dB = 20.0 * std::log10(qMax(hVal / gain, 1e-15));
        response.append(dB);
    }

    return response;
}

/** @brief 重置统计 */
void CicFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 重置滤波器状态 */
void CicFilter::reset()
{
    m_integrators.fill(0.0);
    m_combRegs.fill(0.0);
}
