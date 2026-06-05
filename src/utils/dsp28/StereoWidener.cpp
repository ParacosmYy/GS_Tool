/**
 * @file StereoWidener.cpp
 * @brief 立体声展宽器实现 — M/S处理/交叉馈送/宽度控制
 */

#include "utils/dsp28/StereoWidener.h"

#include <QtMath>
#include <algorithm>

StereoWidener::StereoWidener(double sampleRate, QObject* parent)
    : QObject(parent), m_sampleRate(sampleRate), m_width(1.5),
      m_crossfeedAmount(0.2), m_centerFreq(150.0),
      m_lpStateL(0.0f), m_lpStateR(0.0f),
      m_cfStateL1(0.0f), m_cfStateR1(0.0f),
      m_cfStateL2(0.0f), m_cfStateR2(0.0f)
{
}

QVector<float> StereoWidener::process(const QVector<float>& input)
{
    m_timing.start();
    int frames = input.size() / 2;
    if (frames <= 0) return input;
    ++m_stats.totalFramesProcessed;
    m_stats.totalSamplesProcessed += frames * 2;

    QVector<float> output(input.size());
    for (int i = 0; i < frames; ++i) {
        float left = input[2 * i];
        float right = input[2 * i + 1];
        /* 低频保持居中: 提取低频中心 */
        float lpL = processLowpass(left, m_lpStateL);
        float lpR = processLowpass(right, m_lpStateR);
        float center = (lpL + lpR) * 0.5f;
        /* 去除低频中心后进行M/S展宽 */
        float hpL = left - lpL;
        float hpR = right - lpR;
        applyMidSide(hpL, hpR);
        applyCrossfeed(hpL, hpR);
        /* 恢复低频中心 */
        left = hpL + center;
        right = hpR + center;
        /* 峰值检测 */
        m_stats.peakLevelL = qMax(m_stats.peakLevelL, static_cast<double>(qAbs(left)));
        m_stats.peakLevelR = qMax(m_stats.peakLevelR, static_cast<double>(qAbs(right)));
        output[2 * i] = left;
        output[2 * i + 1] = right;
    }
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFramesProcessed > 0)
        ? m_timeSum / m_stats.totalFramesProcessed : 0.0;
    emit frameProcessed(frames, m_stats.avgProcessingTimeMs);
    return output;
}

void StereoWidener::processBuffers(QVector<float>& left, QVector<float>& right)
{
    m_timing.start();
    int n = qMin(left.size(), right.size());
    if (n <= 0) return;
    ++m_stats.totalFramesProcessed;
    m_stats.totalSamplesProcessed += n * 2;

    for (int i = 0; i < n; ++i) {
        float lpL = processLowpass(left[i], m_lpStateL);
        float lpR = processLowpass(right[i], m_lpStateR);
        float center = (lpL + lpR) * 0.5f;
        float hpL = left[i] - lpL;
        float hpR = right[i] - lpR;
        applyMidSide(hpL, hpR);
        applyCrossfeed(hpL, hpR);
        left[i] = hpL + center;
        right[i] = hpR + center;
        m_stats.peakLevelL = qMax(m_stats.peakLevelL, static_cast<double>(qAbs(left[i])));
        m_stats.peakLevelR = qMax(m_stats.peakLevelR, static_cast<double>(qAbs(right[i])));
    }
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFramesProcessed > 0)
        ? m_timeSum / m_stats.totalFramesProcessed : 0.0;
    emit frameProcessed(n, m_stats.avgProcessingTimeMs);
}

void StereoWidener::applyMidSide(float& left, float& right)
{
    /* M/S编码 */
    float mid = (left + right) * 0.5f;
    float side = (left - right) * 0.5f;
    /* 宽度控制: 缩放侧边信号 */
    float widthF = static_cast<float>(m_width);
    side *= widthF;
    /* M/S解码 */
    left = mid + side;
    right = mid - side;
}

void StereoWidener::applyCrossfeed(float& left, float& right)
{
    if (m_crossfeedAmount <= 0.0) return;
    float cf = static_cast<float>(m_crossfeedAmount);
    /* 一阶低通交叉馈送: 模拟扬声器串音 */
    float alpha = static_cast<float>(200.0 / m_sampleRate);
    alpha = qBound(0.0f, alpha, 1.0f);
    float cfL = alpha * right + (1.0f - alpha) * m_cfStateL1;
    float cfR = alpha * left + (1.0f - alpha) * m_cfStateR1;
    /* 二阶 */
    float cfL2 = alpha * cfL + (1.0f - alpha) * m_cfStateL2;
    float cfR2 = alpha * cfR + (1.0f - alpha) * m_cfStateR2;
    m_cfStateL1 = cfL;
    m_cfStateR1 = cfR;
    m_cfStateL2 = cfL2;
    m_cfStateR2 = cfR2;
    left = left * (1.0f - cf) + cfL2 * cf;
    right = right * (1.0f - cf) + cfR2 * cf;
}

float StereoWidener::processLowpass(float sample, float& state)
{
    float omega = static_cast<float>(2.0 * M_PI * m_centerFreq / m_sampleRate);
    float alpha = omega / (1.0f + omega);
    state = alpha * sample + (1.0f - alpha) * state;
    return state;
}

void StereoWidener::setWidth(double width)
{
    m_width = qBound(0.0, width, 3.0);
}

void StereoWidener::setCrossfeed(double amount)
{
    m_crossfeedAmount = qBound(0.0, amount, 1.0);
}

void StereoWidener::setCenterFreq(double freq)
{
    m_centerFreq = qBound(20.0, freq, m_sampleRate * 0.45);
}

void StereoWidener::reset()
{
    m_lpStateL = m_lpStateR = 0.0f;
    m_cfStateL1 = m_cfStateR1 = 0.0f;
    m_cfStateL2 = m_cfStateR2 = 0.0f;
}

void StereoWidener::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
