/**
 * @file DelayLine3.cpp
 * @brief 数字延迟线效果器实现
 *
 * 实现基于循环缓冲区的数字延迟线，支持可调延迟时间、反馈增益和干湿比混合。
 * 广泛用于音频效果处理，如回声（Echo）、合唱（Chorus）、镶边（Flanger）等。
 *
 * 延迟输出公式: y[n] = (1-wet)*x[n] + wet*(x[n-d] + feedback*y[n-d])
 * 其中 d 为延迟采样数，feedback 为反馈系数，wet 为干湿比。
 *
 * 循环缓冲区工作原理:
 * - 写入指针 (m_writeIdx) 指向当前写入位置
 * - 读取位置 = 写入位置 - 延迟采样数（取模实现循环）
 * - 每次采样后写入指针前进一步
 *
 * 典型延迟时间参数（44.1kHz采样率下）:
 * - 回声效果: 50ms~500ms (2205~22050采样)
 * - 合唱效果: 5ms~30ms (220~1323采样)
 * - 镶边效果: 1ms~10ms (44~441采样)
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/dsp51/DelayLine3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 *
 * 默认延迟为4410采样（约100ms@44.1kHz），反馈0.3，湿信号0.5。
 * 缓冲区初始化为零，避免首次读取时出现噪声。
 *
 * @param parent 父QObject对象指针
 */
DelayLine3::DelayLine3(QObject* parent)
    : QObject(parent)
    , m_delay(4410)
    , m_feedback(0.3)
    , m_wet(0.5)
    , m_writeIdx(0)
    , m_timeSum(0.0)
{
    m_buffer.resize(m_delay, 0.0);
}

/**
 * @brief 设置延迟采样数
 *
 * 调整延迟线缓冲区大小。如果新延迟大于当前缓冲区，
 * 扩展部分填充零；如果小于，保留最近的数据。
 * 写入指针位置会自动调整以保证数据连续性。
 *
 * @param samples 延迟采样数，必须 >= 1
 */
void DelayLine3::setDelaySamples(int samples)
{
    int newDelay = qMax(1, samples);
    if (newDelay == m_delay) {
        return;
    }

    /* 创建新缓冲区并迁移数据 */
    QVector<double> newBuffer(newDelay, 0.0);
    int copyLen = qMin(m_buffer.size(), newDelay);

    /* 从旧缓冲区中按时间顺序复制最近的copyLen个采样 */
    for (int i = 0; i < copyLen; ++i) {
        int readIdx = (m_writeIdx - copyLen + i + m_buffer.size()) % m_buffer.size();
        newBuffer[i] = m_buffer[readIdx];
    }

    m_buffer = newBuffer;
    m_delay = newDelay;
    m_writeIdx = copyLen % newDelay;
}

/**
 * @brief 设置反馈增益
 *
 * 反馈系数控制延迟信号的衰减速率:
 * - 0.0: 无反馈，单次延迟
 * - 0.5: 每次反馈衰减一半
 * - 接近1.0: 延迟信号持续很长时间
 *
 * 注意: 反馈 >= 1.0 会导致信号发散，被钳位到0.99。
 *
 * @param fb 反馈系数，范围 [0.0, 1.0)，过大会导致信号发散
 */
void DelayLine3::setFeedback(double fb)
{
    m_feedback = qBound(0.0, fb, 0.99);
}

/**
 * @brief 设置干湿比混合参数
 *
 * 控制原始信号（干）和延迟信号（湿）的混合比例:
 * - 0.0: 全干信号（无延迟效果）
 * - 0.5: 干湿各半
 * - 1.0: 全湿信号（只有延迟信号）
 *
 * @param wet 湿信号比例，0.0为全干信号，1.0为全湿信号
 */
void DelayLine3::setMix(double wet)
{
    m_wet = qBound(0.0, wet, 1.0);
}

/**
 * @brief 处理输入信号，产生延迟效果
 *
 * 逐采样处理输入信号，使用循环缓冲区实现延迟:
 * 1. 从循环缓冲区读取延迟采样
 * 2. 将输入加上反馈信号写入缓冲区
 * 3. 按干湿比混合原始输入和延迟信号
 * 4. 推进循环写入指针
 *
 * 缓冲区读写示意图（延迟=3）:
 * [w-3] [w-2] [w-1] [w] [w+1]
 *   ^                 ^
 *   读位置            写位置
 *
 * @param input 输入采样数据
 * @return 经过延迟处理后的输出采样数据
 */
QVector<double> DelayLine3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    QVector<double> output(n);

    if (n == 0) {
        return output;
    }

    /* 确保缓冲区大小与延迟参数一致 */
    if (m_buffer.size() != m_delay) {
        m_buffer.resize(m_delay, 0.0);
    }

    /* 预计算干信号增益，避免每次循环内计算 */
    double dry = 1.0 - m_wet;
    int bufSize = m_buffer.size();

    for (int i = 0; i < n; ++i) {
        /* 计算读取位置（写入位置往前延迟m_delay个采样，循环取模） */
        int readIdx = (m_writeIdx - m_delay + bufSize) % bufSize;

        /* 从缓冲区读取延迟信号 */
        double delayed = m_buffer[readIdx];

        /* 写入当前输入 + 反馈信号到缓冲区 */
        m_buffer[m_writeIdx] = input[i] + m_feedback * delayed;

        /* 混合干湿信号: y = dry*x + wet*delayed */
        output[i] = dry * input[i] + m_wet * delayed;

        /* 推进写入指针（循环取模） */
        m_writeIdx = (m_writeIdx + 1) % bufSize;
    }

    /* 更新统计信息: 累加处理次数和采样数 */
    double elapsed = timer.elapsed();
    m_stats.totalProcessCalls++;
    m_stats.totalSamples += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    emit processingCompleted(n);
    return output;
}

/**
 * @brief 清空延迟线缓冲区
 *
 * 将缓冲区所有采样归零，重置写入指针到起始位置。
 * 通常在切换效果参数或重新开始处理时调用。
 */
void DelayLine3::clear()
{
    m_buffer.fill(0.0);
    m_writeIdx = 0;
}

/**
 * @brief 重置所有统计计数器
 *
 * 将处理调用次数、总采样数、平均处理时间等统计指标归零。
 * 不影响延迟参数和缓冲区内容。
 */
void DelayLine3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
