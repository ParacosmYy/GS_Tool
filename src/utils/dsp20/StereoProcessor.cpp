/**
 * @file StereoProcessor.cpp
 * @brief 立体声处理器实现 — Mid/Side编解码/宽度/声像/哈斯效应/相关性
 */

#include "utils/dsp20/StereoProcessor.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
StereoProcessor::StereoProcessor(QObject* parent)
    : QObject(parent)
    , m_width(1.0)
    , m_pan(0.0)
    , m_haasDelay(0)
    , m_sampleRate(44100.0)
    , m_smoothCorrelation(0.0)
    , m_attackCoeff(0.0)
    , m_releaseCoeff(0.0)
    , m_delayWritePos(0)
    , m_timeSum(0.0)
{
    /* 默认Attack/Release: 10ms / 50ms */
    double attackMs = 10.0;
    double releaseMs = 50.0;
    m_attackCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * attackMs * 0.001));
    m_releaseCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * releaseMs * 0.001));
}

/** @brief 设置立体声宽度 @param width 宽度[0.0,2.0] */
void StereoProcessor::setWidth(double width)
{
    m_width = qBound(0.0, width, 2.0);
}

/** @brief 设置平衡声像 @param pan 声像[-1.0,1.0] */
void StereoProcessor::setPan(double pan)
{
    m_pan = qBound(-1.0, pan, 1.0);
}

/** @brief 设置哈斯效应延迟 @param delaySamples 延迟采样数 */
void StereoProcessor::setHaasDelay(int delaySamples)
{
    m_haasDelay = qMax(0, delaySamples);
    m_delayLine.resize(m_haasDelay + 1, 0.0);
    m_delayWritePos = 0;
}

/** @brief 设置采样率 @param sampleRate 采样率 */
void StereoProcessor::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
    m_attackCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * 10.0 * 0.001));
    m_releaseCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * 50.0 * 0.001));
}

/** @brief 处理立体声帧 @param left 左声道 @param right 右声道 @return 处理结果 */
StereoProcessor::StereoFrame StereoProcessor::process(
    const QVector<double>& left, const QVector<double>& right)
{
    QElapsedTimer timer;
    timer.start();

    StereoFrame frame;
    int n = qMin(left.size(), right.size());
    if (n == 0) return frame;

    /* 第一步: Mid/Side编码 */
    auto msPair = encodeMidSide(left, right);
    frame.mid = msPair.first;
    frame.side = msPair.second;

    /* 第二步: 立体声宽度控制 — 调整Side分量 */
    frame.side.resize(n);
    for (int i = 0; i < n; ++i) {
        frame.side[i] *= m_width;
    }

    /* 第三步: Mid/Side解码回L/R */
    auto lrPair = decodeMidSide(frame.mid, frame.side);
    QVector<double>& outL = frame.left;
    QVector<double>& outR = frame.right;
    outL = lrPair.first;
    outR = lrPair.second;

    /* 第四步: 平衡声像控制 */
    /* 使用恒定功率声像: left = cos(theta), right = sin(theta) */
    double panAngle = (m_pan + 1.0) * 0.25 * M_PI; /* [-1,1] -> [0, pi/2] */
    double panL = qCos(panAngle);
    double panR = qSin(panAngle);
    for (int i = 0; i < n; ++i) {
        outL[i] *= panL;
        outR[i] *= panR;
    }

    /* 第五步: 哈斯效应 — 给一侧加微小延迟 */
    if (m_haasDelay > 0 && m_delayLine.size() > 0) {
        for (int i = 0; i < n; ++i) {
            double delayed = m_delayLine[m_delayWritePos];
            m_delayLine[m_delayWritePos] = outR[i];
            outR[i] = delayed;
            m_delayWritePos = (m_delayWritePos + 1) % m_delayLine.size();
        }
    }

    /* 第六步: 计算相关性表 */
    m_corrMeter.currentCorrelation = computeCorrelation(outL, outR);
    m_corrMeter.rmsLeft = computeRms(outL);
    m_corrMeter.rmsRight = computeRms(outR);
    if (qAbs(m_corrMeter.currentCorrelation) > qAbs(m_corrMeter.peakCorrelation)) {
        m_corrMeter.peakCorrelation = m_corrMeter.currentCorrelation;
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalFramesProcessed;
    m_stats.totalSamplesProcessed += static_cast<quint64>(n);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFramesProcessed);

    /* 平滑相关系数 */
    double coeff = (m_corrMeter.currentCorrelation > m_smoothCorrelation)
                   ? m_attackCoeff : m_releaseCoeff;
    m_smoothCorrelation += coeff * (m_corrMeter.currentCorrelation - m_smoothCorrelation);
    m_stats.avgCorrelation = m_smoothCorrelation;

    emit frameProcessed(n, m_corrMeter.currentCorrelation);
    return frame;
}

/** @brief 编码Mid/Side @param left 左声道 @param right 右声道 @return (mid, side) */
QPair<QVector<double>, QVector<double>> StereoProcessor::encodeMidSide(
    const QVector<double>& left, const QVector<double>& right)
{
    int n = qMin(left.size(), right.size());
    QVector<double> mid(n), side(n);

    for (int i = 0; i < n; ++i) {
        mid[i] = (left[i] + right[i]) * 0.5;
        side[i] = (left[i] - right[i]) * 0.5;
    }
    return {mid, side};
}

/** @brief 解码Mid/Side @param mid Mid分量 @param side Side分量 @return (left, right) */
QPair<QVector<double>, QVector<double>> StereoProcessor::decodeMidSide(
    const QVector<double>& mid, const QVector<double>& side)
{
    int n = qMin(mid.size(), side.size());
    QVector<double> left(n), right(n);

    for (int i = 0; i < n; ++i) {
        left[i] = mid[i] + side[i];
        right[i] = mid[i] - side[i];
    }
    return {left, right};
}

/** @brief 获取相关性表 @return 相关性表 */
StereoProcessor::CorrelationMeter StereoProcessor::correlationMeter() const
{
    return m_corrMeter;
}

/** @brief 重置统计 */
void StereoProcessor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_smoothCorrelation = 0.0;
    m_corrMeter = CorrelationMeter{};
    m_delayLine.fill(0.0);
    m_delayWritePos = 0;
}

/** @brief 计算两声道相关性 @param left 左声道 @param right 右声道 @return 相关系数[-1,1] */
double StereoProcessor::computeCorrelation(const QVector<double>& left,
                                            const QVector<double>& right) const
{
    int n = qMin(left.size(), right.size());
    if (n < 2) return 0.0;

    double sumL = 0.0, sumR = 0.0;
    for (int i = 0; i < n; ++i) {
        sumL += left[i];
        sumR += right[i];
    }
    double meanL = sumL / n;
    double meanR = sumR / n;

    double num = 0.0, denL = 0.0, denR = 0.0;
    for (int i = 0; i < n; ++i) {
        double dl = left[i] - meanL;
        double dr = right[i] - meanR;
        num += dl * dr;
        denL += dl * dl;
        denR += dr * dr;
    }

    double den = qSqrt(denL * denR);
    if (den < 1e-14) return 0.0;
    return qBound(-1.0, num / den, 1.0);
}

/** @brief 计算RMS @param data 数据 @return RMS值 */
double StereoProcessor::computeRms(const QVector<double>& data) const
{
    if (data.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double v : data) {
        sum += v * v;
    }
    return qSqrt(sum / data.size());
}
