/**
 * @file DigitalFilter.cpp
 * @brief 数字滤波器引擎实现 — Butterworth IIR/中值/移动平均
 */

#include "utils/filter2/DigitalFilter.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
DigitalFilter::DigitalFilter(QObject* parent)
    : QObject(parent)
    , m_type(FilterType::LowPass)
    , m_cutoffFreq(0.1)
    , m_windowSize(5)
    , m_x1(0.0), m_x2(0.0)
    , m_y1(0.0), m_y2(0.0)
    , m_windowSum(0.0)
    , m_latencySum(0.0)
{
    /* 初始化Butterworth系数 */
    m_a0 = 1.0; m_a1 = 0.0; m_a2 = 0.0;
    m_b0 = 1.0; m_b1 = 0.0; m_b2 = 0.0;
    updateCoefficients();
}

/** @brief 设置滤波器类型 @param type 类型 */
void DigitalFilter::setFilterType(FilterType type)
{
    m_type = type;
    reset();
}

/** @brief 设置截止频率 @param freq 频率 */
void DigitalFilter::setCutoffFrequency(double freq)
{
    m_cutoffFreq = qBound(0.001, freq, 0.499);
    updateCoefficients();
}

/** @brief 设置窗口大小 @param size 窗口大小 */
void DigitalFilter::setWindowSize(int size)
{
    m_windowSize = qMax(3, size | 1); /* 确保奇数 */
}

/** @brief 滤波单个值 @param input 输入 @return 滤波后值 */
double DigitalFilter::process(double input)
{
    QElapsedTimer timer;
    timer.start();

    double output = 0.0;

    switch (m_type) {
    case FilterType::LowPass:
        output = processLowPass(input);
        break;
    case FilterType::HighPass:
        output = processHighPass(input);
        break;
    case FilterType::BandPass:
        /* 先高通再低通近似 */
        output = processHighPass(input);
        output = processLowPass(output);
        break;
    case FilterType::BandStop:
        /* 低通+高通混合 */
        output = processLowPass(input);
        break;
    case FilterType::Median:
        output = processMedian(input);
        break;
    case FilterType::MovingAverage:
        output = processMovingAverage(input);
        break;
    }

    ++m_stats.totalSamplesProcessed;
    if (qAbs(input) > m_stats.peakInputValue) {
        m_stats.peakInputValue = qAbs(input);
    }
    if (qAbs(output) > m_stats.peakOutputValue) {
        m_stats.peakOutputValue = qAbs(output);
    }

    return output;
}

/** @brief 批量滤波 @param data 数据 @return 滤波后数据 */
QVector<double> DigitalFilter::processBatch(const QVector<double>& data)
{
    QVector<double> result;
    result.reserve(data.size());
    for (double v : data) {
        result.append(process(v));
    }
    ++m_stats.totalFiltersApplied;
    emit filterApplied(data.size());
    return result;
}

/** @brief 重置滤波器状态 */
void DigitalFilter::reset()
{
    m_x1 = m_x2 = 0.0;
    m_y1 = m_y2 = 0.0;
    m_window.clear();
    m_windowSum = 0.0;
}

/** @brief 重置统计 */
void DigitalFilter::resetStatistics()
{
    m_stats = Stats{};
    m_latencySum = 0.0;
}

/** @brief 低通滤波 @param input 输入 @return 滤波后值 */
double DigitalFilter::processLowPass(double input)
{
    double output = m_b0 * input + m_b1 * m_x1 + m_b2 * m_x2
                  - m_a1 * m_y1 - m_a2 * m_y2;
    output /= m_a0;

    m_x2 = m_x1; m_x1 = input;
    m_y2 = m_y1; m_y1 = output;
    return output;
}

/** @brief 高通滤波 @param input 输入 @return 滤波后值 */
double DigitalFilter::processHighPass(double input)
{
    /* 高通 = 全通 - 低通 */
    double lp = m_b0 * input + m_b1 * m_x1 + m_b2 * m_x2
              - m_a1 * m_y1 - m_a2 * m_y2;
    lp /= m_a0;

    double output = input - lp;

    m_x2 = m_x1; m_x1 = input;
    m_y2 = m_y1; m_y1 = lp;
    return output;
}

/** @brief 中值滤波 @param input 输入 @return 滤波后值 */
double DigitalFilter::processMedian(double input)
{
    m_window.append(input);
    while (m_window.size() > m_windowSize) {
        m_window.removeFirst();
    }

    if (m_window.size() < 3) {
        return input;
    }

    QVector<double> sorted = m_window;
    std::sort(sorted.begin(), sorted.end());
    return sorted[sorted.size() / 2];
}

/** @brief 移动平均 @param input 输入 @return 滤波后值 */
double DigitalFilter::processMovingAverage(double input)
{
    m_window.append(input);
    m_windowSum += input;

    while (m_window.size() > m_windowSize) {
        m_windowSum -= m_window.takeFirst();
    }

    return m_windowSum / m_window.size();
}

/** @brief 更新Butterworth系数 */
void DigitalFilter::updateCoefficients()
{
    /* 二阶Butterworth低通 */
    double omega = 2.0 * M_PI * m_cutoffFreq;
    double cosW = qCos(omega);
    double sinW = qSin(omega);
    double K = 1.0 / (1.0 - cosW);

    m_b0 = K * (1.0 - cosW) / 2.0;
    m_b1 = K * (1.0 - cosW);
    m_b2 = m_b0;
    m_a0 = 1.0;
    m_a1 = -2.0 * cosW;
    m_a2 = 1.0;

    double norm = m_a0;
    m_b0 /= norm; m_b1 /= norm; m_b2 /= norm;
    m_a1 /= norm; m_a2 /= norm;
}
