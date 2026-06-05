/**
 * @file DelayLine.cpp
 * @brief 分数延迟线实现 — Lagrange/全通/线性插值 + 环形缓冲区
 */

#include "utils/dsp15/DelayLine.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

DelayLine::DelayLine(int maxDelay, QObject* parent)
    : QObject(parent)
    , m_maxDelay(qMax(1, maxDelay))
    , m_writePos(0)
    , m_allpassY1(0.0)
    , m_allpassX1(0.0)
{
    m_buffer.assign(m_maxDelay + 16, 0.0);
    m_params.delay = 0.0;
}

DelayLine::~DelayLine() = default;

// ═══════════════════════════════════════════════════════════
// 参数控制
// ═══════════════════════════════════════════════════════════

void DelayLine::setParameters(const Parameters& params)
{
    m_params = params;
    m_params.delay = std::max(0.0, std::min(m_params.delay,
                                             static_cast<double>(m_maxDelay)));
    m_params.feedback = std::max(0.0, std::min(m_params.feedback, 0.999));
}

DelayLine::Parameters DelayLine::parameters() const
{
    return m_params;
}

void DelayLine::setDelay(double delay)
{
    QElapsedTimer timer;
    timer.start();

    delay = std::max(0.0, std::min(delay, static_cast<double>(m_maxDelay)));
    if (std::abs(delay - m_params.delay) < 1e-10) return;

    m_params.delay = delay;

    m_stats.totalDelayChanges++;
    m_timeSum += static_cast<double>(timer.elapsed());
    quint64 total = m_stats.totalSamplesProcessed + m_stats.totalDelayChanges;
    m_stats.avgProcessingTimeMs =
        (total > 0) ? m_timeSum / static_cast<double>(total) : 0.0;

    emit delayChanged(delay);
}

void DelayLine::setFeedback(double feedback)
{
    m_params.feedback = std::max(0.0, std::min(feedback, 0.999));
}

// ═══════════════════════════════════════════════════════════
// 信号处理
// ═══════════════════════════════════════════════════════════

double DelayLine::processOne(double input)
{
    QElapsedTimer timer;
    timer.start();

    /* 读取延迟输出(在写入新值之前) */
    double delayed = readInterpolated(m_params.delay);

    /* 计算反馈信号 */
    double feedbackSignal = delayed * m_params.feedback;

    /* 写入: 输入 + 反馈 */
    m_buffer[m_writePos] = input + feedbackSignal;
    m_writePos = (m_writePos + 1) % static_cast<int>(m_buffer.size());

    /* 输出: dry * 输入 + wet * (前馈 * 延迟 + 延迟) */
    double output = m_params.dryMix * input
                    + m_params.wetMix * (delayed + m_params.feedforward * delayed);

    m_stats.totalSamplesProcessed++;
    m_timeSum += static_cast<double>(timer.elapsed());
    quint64 total = m_stats.totalSamplesProcessed + m_stats.totalDelayChanges;
    m_stats.avgProcessingTimeMs =
        (total > 0) ? m_timeSum / static_cast<double>(total) : 0.0;

    return output;
}

QVector<double> DelayLine::process(const QVector<double>& input)
{
    QVector<double> output;
    output.reserve(input.size());
    for (double sample : input) {
        output.append(processOne(sample));
    }
    return output;
}

void DelayLine::reset()
{
    std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
    m_writePos = 0;
    m_allpassY1 = 0.0;
    m_allpassX1 = 0.0;
}

// ═══════════════════════════════════════════════════════════
// 查询
// ═══════════════════════════════════════════════════════════

double DelayLine::tapOutput() const
{
    if (m_params.delay < 1.0) return 0.0;
    int intDelay = static_cast<int>(m_params.delay);
    int readPos = m_writePos - intDelay - 1;
    if (readPos < 0) readPos += static_cast<int>(m_buffer.size());
    return m_buffer[readPos];
}

double DelayLine::tapAt(int offset) const
{
    int pos = m_writePos - offset - 1;
    while (pos < 0) pos += static_cast<int>(m_buffer.size());
    pos = pos % static_cast<int>(m_buffer.size());
    return m_buffer[pos];
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

DelayLine::Stats DelayLine::stats() const
{
    return m_stats;
}

void DelayLine::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ═══════════════════════════════════════════════════════════
// 内部实现 — 插值
// ═══════════════════════════════════════════════════════════

double DelayLine::readInterpolated(double delaySamples)
{
    if (delaySamples < 0.5) return 0.0;

    int intDelay = static_cast<int>(std::floor(delaySamples));
    double frac = delaySamples - static_cast<double>(intDelay);

    int bufSize = static_cast<int>(m_buffer.size());

    switch (m_params.mode) {
    case Lagrange:
        return interpolateLagrange(frac);
    case Allpass:
        return interpolateAllpass(frac);
    case Linear:
        return interpolateLinear(frac);
    default:
        return interpolateLinear(frac);
    }
}

double DelayLine::interpolateLagrange(double frac)
{
    int order = m_params.lagrangeOrder;
    int half = order / 2;
    int bufSize = static_cast<int>(m_buffer.size());
    int intDelay = static_cast<int>(std::floor(m_params.delay));

    double result = 0.0;

    for (int j = 0; j <= order; ++j) {
        /* Lagrange基函数 L_j(frac) */
        double lj = 1.0;
        for (int k = 0; k <= order; ++k) {
            if (k == j) continue;
            /* 自变量: frac - k */
            double denom = static_cast<double>(j) - static_cast<double>(k);
            if (std::abs(denom) < 1e-12) denom = 1e-12;
            lj *= (frac - static_cast<double>(k - half)) / denom;
        }

        /* 从环形缓冲区读取采样 */
        int readOffset = intDelay - j + half;
        int readPos = m_writePos - readOffset - 1;
        while (readPos < 0) readPos += bufSize;
        readPos = readPos % bufSize;

        result += lj * m_buffer[readPos];
    }

    return result;
}

double DelayLine::interpolateAllpass(double frac)
{
    /* Thiran一阶全通插值 */
    /* 系数 a = (1 - frac) / (1 + frac) */
    double a = (1.0 - frac) / (1.0 + frac);

    int intDelay = static_cast<int>(std::floor(m_params.delay));
    int bufSize = static_cast<int>(m_buffer.size());

    int readPos = m_writePos - intDelay - 1;
    while (readPos < 0) readPos += bufSize;
    readPos = readPos % bufSize;

    double x_n = m_buffer[readPos];

    /* 全通: y[n] = a * x[n] + x[n-1] - a * y[n-1] */
    double y_n = a * x_n + m_allpassX1 - a * m_allpassY1;

    m_allpassX1 = x_n;
    m_allpassY1 = y_n;

    return y_n;
}

double DelayLine::interpolateLinear(double frac)
{
    int intDelay = static_cast<int>(std::floor(m_params.delay));
    int bufSize = static_cast<int>(m_buffer.size());

    int readPos0 = m_writePos - intDelay - 1;
    while (readPos0 < 0) readPos0 += bufSize;
    readPos0 = readPos0 % bufSize;

    int readPos1 = readPos0 - 1;
    if (readPos1 < 0) readPos1 += bufSize;

    double y0 = m_buffer[readPos0];
    double y1 = m_buffer[readPos1];

    return y0 + frac * (y1 - y0);
}
