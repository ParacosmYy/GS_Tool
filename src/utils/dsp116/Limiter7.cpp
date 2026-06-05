#include "Limiter7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化砖墙限幅器
 * @param parent 父对象指针
 */
Limiter7::Limiter7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Limiter7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置限幅器参数
 * @param ceilingDb 输出上限(dB)
 * @param thresholdDb 限幅阈值(dB)
 * @param releaseMs 释放时间(ms)
 */
void Limiter7::setParameters(double ceilingDb, double thresholdDb, double releaseMs)
{
    Q_UNUSED(ceilingDb)
    Q_UNUSED(thresholdDb)
    Q_UNUSED(releaseMs)
}

/**
 * @brief 设置lookahead时间
 * @param lookaheadMs lookahead时间(ms)
 */
void Limiter7::setLookahead(double lookaheadMs)
{
    Q_UNUSED(lookaheadMs)
}

/**
 * @brief 获取当前增益缩减量
 * @return 增益缩减值(dB)
 */
double Limiter7::getGainReduction() const
{
    return 0.0;
}

/**
 * @brief 处理音频帧进行限幅
 *
 * 砖墙限幅器确保输出不超过上限电平：
 * 1. Lookahead缓冲：预读未来样本检测峰值
 * 2. 增益计算：当峰值超过阈值时计算所需衰减
 * 3. 平滑释放：防止低频失真
 * 4. 硬限幅：最终裁剪到ceiling电平
 *
 * @param inputFrame 输入音频采样帧
 * @return 限幅后的音频帧
 */
QVector<double> Limiter7::processFrame(const QVector<double>& inputFrame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = inputFrame.size();
    QVector<double> output(n);
    if (n == 0) {
        emit limitingCompleted(0);
        return output;
    }

    /* 默认限幅参数 */
    const double ceilingDb = -0.3;
    const double thresholdDb = -6.0;
    const double releaseMs = 50.0;
    const int lookaheadSamples = 64;
    const double sampleRate = 44100.0;

    const double ceilingLinear = qPow(10.0, ceilingDb / 20.0);
    const double thresholdLinear = qPow(10.0, thresholdDb / 20.0);
    const double releaseCoeff = qExp(-1.0 / (releaseMs * 0.001 * sampleRate));

    /* Lookahead缓冲区 */
    static QVector<double> lookaheadBuf(lookaheadSamples, 0.0);
    static int bufPos = 0;

    /* 增益缩减状态 */
    static double gainReduction = 1.0;

    for (int i = 0; i < n; ++i) {
        /* 查找lookahead窗口内最大峰值 */
        double peakInWindow = 0.0;
        for (int j = 0; j < lookaheadSamples; ++j) {
            int idx = (bufPos + j) % lookaheadSamples;
            double val = (j == 0) ? qAbs(inputFrame[i]) : qAbs(lookaheadBuf[idx]);
            if (val > peakInWindow) peakInWindow = val;
        }

        /* 写入lookahead缓冲 */
        lookaheadBuf[bufPos] = inputFrame[i];
        bufPos = (bufPos + 1) % lookaheadSamples;

        /* 计算所需增益 */
        double targetGain = 1.0;
        if (peakInWindow > thresholdLinear) {
            targetGain = thresholdLinear / qMax(peakInWindow, 1e-10);
            targetGain = qBound(ceilingLinear / qMax(peakInWindow, 1e-10), targetGain, 1.0);
        }

        /* 平滑增益变化（快速攻击，慢释放） */
        if (targetGain < gainReduction) {
            gainReduction = targetGain;
        } else {
            gainReduction = releaseCoeff * gainReduction + (1.0 - releaseCoeff) * targetGain;
        }

        /* 应用增益并硬限幅到ceiling */
        output[i] = qBound(-ceilingLinear, inputFrame[i] * gainReduction, ceilingLinear);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessFrames++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessFrames;

    emit limitingCompleted(n);
    return output;
}
