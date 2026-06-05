#include "Flanger3.h"
#include <QElapsedTimer>
#include <cmath>

/**
 * @brief 构造函数，初始化Flanger效果器
 * @param parent 父QObject对象指针
 *
 * 默认参数：延迟2ms，速率0.5Hz，反馈0.7，产生经典Flanger效果。
 */
Flanger3::Flanger3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 处理音频缓冲区，应用Flanger效果
 *
 * Flanger通过短延迟线的周期性调制产生梳状滤波效果：
 * 1. 延迟时间在[0, delayMs]范围内正弦变化
 * 2. 延迟信号与原始信号混合产生相位干涉（梳状滤波）
 * 3. 反馈回路增强效果强度
 *
 * 产生的效果是空灵的"喷气式飞机"声音，常用于
 * 吉他、鼓声和人声的空间效果处理。
 *
 * @param input 输入音频采样序列
 * @return 处理后的音频采样序列
 */
QVector<double> Flanger3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    if (n == 0) return {};

    QVector<double> output(n);

    QVector<double> output(n);

    /// 延迟线参数配置
    const double sampleRate = 44100.0;
    const int maxDelaySamples = static_cast<int>(m_delay * sampleRate / 1000.0 * 2);
    const int bufferSize = qMax(256, maxDelaySamples * 2);

    /// 延迟缓冲区（静态保持跨调用连续性）
    static QVector<double> delayBuffer(bufferSize, 0.0);
    static int writePos = 0;
    static double phase = 0.0;

    /// 处理每个采样点
    for (int i = 0; i < n; ++i) {
        /// 计算当前延迟量（正弦LFO调制）
        double lfoPhase = 2.0 * M_PI * m_rate * phase;
        double modDelay = m_delay * (1.0 + std::sin(lfoPhase)) * 0.5;
        int delaySamples = static_cast<int>(modDelay * sampleRate / 1000.0);
        delaySamples = qBound(1, delaySamples, bufferSize - 1);

        /// 从延迟线读取（线性插值提高音质）
        double frac = modDelay * sampleRate / 1000.0 - delaySamples;
        int readPos1 = (writePos - delaySamples + bufferSize) % bufferSize;
        int readPos2 = (readPos1 - 1 + bufferSize) % bufferSize;
        double delayed = delayBuffer[readPos1] * (1.0 - frac) + delayBuffer[readPos2] * frac;

        /// 混合原始信号和延迟信号（梳状滤波效果）
        output[i] = input[i] * 0.7 + delayed * 0.3;

        /// 更新延迟线（带反馈回路增强效果）
        delayBuffer[writePos] = input[i] + delayed * m_feedback;
        writePos = (writePos + 1) % bufferSize;

        /// 推进LFO相位（周期性重置避免累积误差）
        phase += 1.0 / sampleRate;
        if (phase > 1.0 / m_rate) phase -= 1.0 / m_rate;
    }

    /// 更新统计信息
    m_stats.totalSamplesProcessed += n;
    m_stats.totalBuffersApplied++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBuffersApplied;

    double currentDelay = m_delay * (1.0 + std::sin(2.0 * M_PI * m_rate * phase)) * 0.5;
    emit effectApplied(n, currentDelay);
    return output;
}

/**
 * @brief 设置Flanger效果参数
 *
 * 延迟时间控制梳状滤波的间距，延迟越短齿距越大。
 * LFO速率控制效果的"旋转"速度。
 * 反馈系数控制效果的持续性和强度，过高的反馈可能导致自激。
 *
 * @param delayMs 最大延迟时间(ms)，典型1~5ms
 * @param rateHz LFO调制速率(Hz)，典型0.1~3.0Hz
 * @param feedback 反馈系数(0.0~0.95)，越高效果越强
 */
void Flanger3::setParameters(double delayMs, double rateHz, double feedback)
{
    m_delay = qBound(0.5, delayMs, 10.0);
    m_rate = qBound(0.05, rateHz, 5.0);
    m_feedback = qBound(0.0, feedback, 0.95);
}

/**
 * @brief 获取当前统计数据
 *
 * 返回包含处理采样总数、应用缓冲区数和平均处理耗时的统计信息，
 * 可用于实时性能监控和延迟分析。
 *
 * @return 包含采样处理数、缓冲区数和平均耗时的Stats结构
 */
Flanger3::Stats Flanger3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 *
 * 将所有计数器归零并清除累计时间，用于新一轮的性能统计。
 * 不影响当前的Flanger参数设置（延迟/速率/反馈）。
 * 重置后首次process()调用的耗时将作为新的基准。
 */
void Flanger3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
