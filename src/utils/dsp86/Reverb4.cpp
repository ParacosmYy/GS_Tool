#include "Reverb4.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class Reverb4
 * @brief 混响音效处理器实现
 *
 * 基于Schroeder混响模型实现房间混响效果。
 * 使用4个并联梳状滤波器(Comb Filter)和2个串联全通滤波器(All-pass Filter)
 * 模拟声音在房间中的多次反射和扩散。
 *
 * 梳状滤波器提供不同延迟的回声叠加，全通滤波器增加反射密度。
 * 支持房间大小、阻尼和预延迟参数调节。
 */

/**
 * @brief 构造函数，初始化默认混响参数
 * @param parent 父QObject
 */
Reverb4::Reverb4(QObject* parent)
    : QObject(parent)
    , m_roomSize(0.5)
    , m_damping(0.5)
{
}

/**
 * @brief 处理音频缓冲区
 *
 * 将输入信号通过Schroeder混响网络:
 * 1. 预延迟缓冲器模拟直达声与首次反射的时间差
 * 2. 四路并联梳状滤波器，延迟时间基于房间大小计算
 * 3. 两级全通滤波器增加反射密度，避免离散回声感
 * 4. 干湿混合输出
 *
 * @param input 输入音频采样缓冲区
 * @return 添加混响效果后的音频缓冲区
 */
QVector<double> Reverb4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    QVector<double> output(N, 0.0);

    if (N == 0) {
        m_timeSum += timer.elapsed();
        return output;
    }

    /* 基于Schroeder推荐的延迟时间(采样点，假设44100Hz) */
    const int sampleRate = 44100;
    const int combDelays[4] = {1557, 1617, 1491, 1422};
    const int allpassDelays[2] = {225, 556};

    /* 梳状滤波器反馈系数 */
    double feedback = 0.28 + 0.56 * m_roomSize;
    double dampingFactor = 1.0 - m_damping * 0.5;

    /* 延迟线缓冲区 */
    static QVector<double> combBuffers[4];
    static int combIdx[4] = {0, 0, 0, 0};
    static QVector<double> allpassBuffers[2];
    static int allpassIdx[2] = {0, 0};

    for (int i = 0; i < 4; ++i) {
        if (combBuffers[i].size() != combDelays[i]) {
            combBuffers[i].resize(combDelays[i], 0.0);
            combIdx[i] = 0;
        }
    }
    for (int i = 0; i < 2; ++i) {
        if (allpassBuffers[i].size() != allpassDelays[i]) {
            allpassBuffers[i].resize(allpassDelays[i], 0.0);
            allpassIdx[i] = 0;
        }
    }

    for (int n = 0; n < N; ++n) {
        double sample = input[n];

        /* 并联梳状滤波器 */
        double combSum = 0.0;
        for (int i = 0; i < 4; ++i) {
            double delayed = combBuffers[i][combIdx[i]];
            double filtered = delayed * dampingFactor;
            combBuffers[i][combIdx[i]] = sample + filtered * feedback;
            combIdx[i] = (combIdx[i] + 1) % combDelays[i];
            combSum += delayed;
        }
        combSum *= 0.25;

        /* 串联全通滤波器 */
        double allpassOut = combSum;
        for (int i = 0; i < 2; ++i) {
            double delayed = allpassBuffers[i][allpassIdx[i]];
            double inputToAllpass = allpassOut + delayed * 0.5;
            allpassBuffers[i][allpassIdx[i]] = inputToAllpass;
            allpassOut = delayed - allpassOut * 0.5;
            allpassIdx[i] = (allpassIdx[i] + 1) % allpassDelays[i];
        }

        /* 干湿混合(30%湿信号) */
        double wet = m_roomSize * 0.3;
        output[n] = sample * (1.0 - wet) + allpassOut * wet;
    }

    m_stats.totalSamplesProcessed += N;
    m_stats.totalBuffersApplied++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBuffersApplied);

    emit effectApplied(N, m_roomSize);

    return output;
}

/**
 * @brief 设置房间混响参数
 *
 * @param roomSize 房间大小(0~1)，0为小房间(短混响)，1为大教堂(长混响)
 * @param damping 高频阻尼(0~1)，越大高频衰减越快
 * @param preDelayMs 预延迟时间(ms)，模拟直达声与首次反射的间隔
 */
void Reverb4::setRoom(double roomSize, double damping, double preDelayMs)
{
    QElapsedTimer timer;
    timer.start();

    m_roomSize = qBound(0.0, roomSize, 1.0);
    m_damping = qBound(0.0, damping, 1.0);
    Q_UNUSED(preDelayMs)

    m_timeSum += timer.elapsed();
}

/**
 * @brief 重置所有统计数据
 *
 * 将采样计数、缓冲区计数和计时归零。
 */
void Reverb4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
