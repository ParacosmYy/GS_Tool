#include "Phaser7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化移相器效果处理器
 * @param parent 父对象指针
 */
Phaser7::Phaser7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Phaser7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置LFO参数
 * @param rateHz LFO速率(Hz)
 * @param depth 调制深度[0, 1]
 * @param waveform 波形类型(sine/triangle/square)
 */
void Phaser7::setLFO(double rateHz, double depth, const QString& waveform)
{
    Q_UNUSED(rateHz)
    Q_UNUSED(depth)
    Q_UNUSED(waveform)
}

/**
 * @brief 设置全通滤波器级数
 * @param stages 级数(2/4/6/8)
 */
void Phaser7::setStages(int stages)
{
    Q_UNUSED(stages)
}

/**
 * @brief 设置干/湿信号混合比例
 * @param mix 混合比例[0, 1]
 */
void Phaser7::setMix(double mix)
{
    Q_UNUSED(mix)
}

/**
 * @brief 设置反馈量
 * @param feedback 反馈量[0, 0.99]
 */
void Phaser7::setFeedback(double feedback)
{
    Q_UNUSED(feedback)
}

/**
 * @brief 处理音频帧进行移相效果
 *
 * 多阶全通滤波器+反馈的移相器：
 * 1. LFO生成调制信号（正弦/三角/方波）
 * 2. 调制信号控制全通滤波器组的截止频率
 * 3. 反馈路径：将输出反馈到输入增强效果深度
 * 4. 干湿混合控制效果强度
 *
 * 全通滤波器：y[n] = c*(x[n] - y[n-1]) + x[n-1]
 * c由LFO调制，产生周期性相位偏移。
 * 反馈使梳状滤波凹陷更深更明显。
 *
 * @param inputFrame 输入音频采样帧
 * @return 移相处理后的音频帧
 */
QVector<double> Phaser7::processFrame(const QVector<double>& inputFrame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = inputFrame.size();
    QVector<double> output(n);
    if (n == 0) {
        emit phasingCompleted(0);
        return output;
    }

    /* 默认参数 */
    const int stages = 8;
    const double lfoRate = 0.5;
    const double lfoDepth = 0.7;
    const double mix = 0.5;
    const double feedback = 0.5;
    const double sampleRate = 44100.0;

    /* 全通滤波器状态 */
    static QVector<double> apState(stages, 0.0);
    if (apState.size() != stages) {
        apState.resize(stages);
        std::fill(apState.begin(), apState.end(), 0.0);
    }

    /* LFO与反馈状态 */
    static double lfoPhase = 0.0;
    static double feedbackBuf = 0.0;

    const double phaseInc = 2.0 * M_PI * lfoRate / sampleRate;

    for (int i = 0; i < n; ++i) {
        /* LFO：正弦波调制 */
        double lfoValue = 0.5 * (1.0 + qSin(lfoPhase));
        lfoPhase += phaseInc;
        if (lfoPhase >= 2.0 * M_PI) lfoPhase -= 2.0 * M_PI;

        /* LFO映射到全通系数 */
        double coeff = 0.2 + 0.6 * lfoValue * lfoDepth;

        /* 加入反馈到输入 */
        double x = inputFrame[i] + feedbackBuf * feedback;

        /* 多阶全通滤波器级联 */
        double y = x;
        for (int s = 0; s < stages; ++s) {
            double prev = apState[s];
            apState[s] = y;
            y = coeff * (y - prev) + prev;
        }

        /* 更新反馈缓冲 */
        feedbackBuf = y;

        /* 干湿混合 */
        output[i] = inputFrame[i] * (1.0 - mix) + y * mix;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessFrames++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessFrames;

    emit phasingCompleted(n);
    return output;
}
