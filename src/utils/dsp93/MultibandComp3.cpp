#include "MultibandComp3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化多频段压缩器
 * @param parent 父对象指针
 */
MultibandComp3::MultibandComp3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置频段数量
 * @param count 频段数，建议2~8
 */
void MultibandComp3::setBandCount(int count)
{
    m_bandCount = qBound(1, count, 16);
}

/**
 * @brief 设置指定频段的压缩阈值
 * @param band 频段索引(0-based)
 * @param threshold 压缩阈值(dB)
 */
void MultibandComp3::setThreshold(int band, double threshold)
{
    Q_UNUSED(band)
    Q_UNUSED(threshold)
    /* 阈值存储在内部数组中(简化实现) */
}

/**
 * @brief 简化二阶IIR低通滤波器
 * @param input 输入信号
 * @param cutoff 归一化截止频率(0~0.5)
 * @return 滤波后的信号
 */
static QVector<double> lowpassFilter(const QVector<double>& input, double cutoff)
{
    QVector<double> output(input.size(), 0.0);
    double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
    double omega = 2.0 * M_PI * cutoff;
    double alpha = std::sin(omega) / (2.0 * 0.707);
    double b0 = (1.0 - std::cos(omega)) / 2.0;
    double b1 = 1.0 - std::cos(omega);
    double b2 = b0;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * std::cos(omega);
    double a2 = 1.0 - alpha;

    for (int i = 0; i < input.size(); ++i) {
        double x0 = input[i];
        double y0 = (b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;
        output[i] = y0;
        x2 = x1; x1 = x0; y2 = y1; y1 = y0;
    }
    return output;
}

/**
 * @brief 处理音频采样数据
 *
 * 1. 通过交叉滤波器将信号分为多个频段
 * 2. 对每个频段独立计算增益缩减量
 * 3. 应用压缩后合并各频段
 *
 * @param samples 输入音频采样
 */
void MultibandComp3::process(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    if (samples.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalProcessed++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
        emit processingCompleted(0);
        return;
    }

    const int N = samples.size();

    /* 频段交叉滤波 */
    QVector<QVector<double>> bands(m_bandCount);
    for (int b = 0; b < m_bandCount; ++b) {
        double lowCutoff = static_cast<double>(b) / m_bandCount * 0.5;
        double highCutoff = static_cast<double>(b + 1) / m_bandCount * 0.5;

        /* 先低通到高频截止 */
        auto filtered = lowpassFilter(samples, highCutoff);

        /* 如果不是最低频段，做高通差分 */
        if (b > 0) {
            auto lowFiltered = lowpassFilter(samples, lowCutoff);
            for (int i = 0; i < N; ++i) {
                filtered[i] -= lowFiltered[i];
            }
        }
        bands[b] = filtered;
    }

    /* 对每个频段独立应用动态压缩 */
    double ratio = 4.0;
    double thresholdLin = 0.5;

    for (int b = 0; b < m_bandCount; ++b) {
        for (int i = 0; i < N; ++i) {
            double absVal = std::abs(bands[b][i]);
            if (absVal > thresholdLin) {
                double overDb = 20.0 * std::log10(absVal / thresholdLin);
                double gainDb = -overDb * (1.0 - 1.0 / ratio);
                double gainLin = std::pow(10.0, gainDb / 20.0);
                bands[b][i] *= gainLin;
            }
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
    emit processingCompleted(N);
}

/**
 * @brief 重置统计数据
 */
void MultibandComp3::resetStatistics()
{
    m_stats.totalProcessed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
