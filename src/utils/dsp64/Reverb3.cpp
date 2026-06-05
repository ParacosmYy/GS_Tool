/**
 * @file Reverb3.cpp
 * @brief 混响音效处理器实现（第3版）
 *
 * 使用梳状滤波器（Comb Filter）和全通滤波器（All-pass Filter）
 * 实现Schroeder混响算法。支持房间大小、衰减时间、阻尼、
 * 预延迟和干湿比等参数调节。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/dsp64/Reverb3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化混响处理器
 * @param parent 父QObject对象指针
 */
Reverb3::Reverb3(QObject* parent)
    : QObject(parent)
{
    /* 默认初始化延迟缓冲区 */
    int bufSize = static_cast<int>(44100.0 * 0.1);
    m_delayBuf.resize(qMax(bufSize, 1), 0.0);
    m_delayPos = 0;
}

/**
 * @brief 设置虚拟房间大小
 * @param size 房间大小参数（0.0~1.0），影响混响尾部长度
 */
void Reverb3::setRoomSize(double size)
{
    m_roomSize = qBound(0.0, size, 1.0);
}

/**
 * @brief 设置混响衰减时间
 * @param ms 衰减到60dB以下的时间（毫秒）
 */
void Reverb3::setDecayTime(double ms)
{
    m_decay = qMax(10.0, ms);
}

/**
 * @brief 设置高频阻尼系数
 * @param damp 阻尼系数（0.0~1.0），越大高频衰减越快
 */
void Reverb3::setDamping(double damp)
{
    m_damping = qBound(0.0, damp, 1.0);
}

/**
 * @brief 设置预延迟时间
 * @param ms 早期反射与直达声之间的延迟（毫秒）
 */
void Reverb3::setPreDelay(double ms)
{
    m_preDelay = qMax(0.0, ms);
}

/**
 * @brief 设置干湿混合比例
 * @param mix 湿信号比例（0.0=全干, 1.0=全湿）
 */
void Reverb3::setWetDry(double mix)
{
    m_wetDry = qBound(0.0, mix, 1.0);
}

/**
 * @brief 处理输入音频信号，添加混响效果
 *
 * 使用四个并行梳状滤波器和两个级联全通滤波器
 * 生成自然的混响效果。
 *
 * @param input 输入音频采样序列
 * @return 添加混响效果后的音频序列
 */
QVector<double> Reverb3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    if (input.isEmpty()) {
        emit processingCompleted(0, 0.0);
        return output;
    }

    int n = input.size();
    output.resize(n);

    /* 梳状滤波器延迟长度（基于房间大小） */
    static const int combDelays[] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
    static const int allpassDelays[] = {556, 441, 341, 225};
    const int nCombs = 8;
    const int nAllpass = 4;

    /* 缩放延迟长度 */
    double scaleFactor = m_roomSize * 1.0 + 0.5;

    /* 分配延迟线 */
    QVector<QVector<double>> combBufs(nCombs);
    QVector<int> combPos(nCombs, 0);
    QVector<double> combFeedback(nCombs, 0.0);

    for (int i = 0; i < nCombs; ++i) {
        int len = static_cast<int>(combDelays[i] * scaleFactor);
        len = qMax(len, 1);
        combBufs[i].resize(len, 0.0);
    }

    QVector<QVector<double>> apBufs(nAllpass);
    QVector<int> apPos(nAllpass, 0);
    for (int i = 0; i < nAllpass; ++i) {
        int len = static_cast<int>(allpassDelays[i] * scaleFactor);
        len = qMax(len, 1);
        apBufs[i].resize(len, 0.0);
    }

    /* 计算反馈系数 */
    double fbCoeff = qPow(0.001, 1.0 / (m_decay * 0.001 * 44100.0));
    double damp1 = m_damping;
    double damp2 = 1.0 - damp1;

    /* 预延迟缓冲 */
    int preDelaySamples = static_cast<int>(m_preDelay * 0.001 * 44100.0);
    QVector<double> preDelayBuf(qMax(preDelaySamples, 1), 0.0);
    int preDelayPos = 0;

    /* 逐样本处理 */
    for (int s = 0; s < n; ++s) {
        double inSample = input[s];

        /* 预延迟 */
        double delayed = preDelayBuf[preDelayPos];
        preDelayBuf[preDelayPos] = inSample;
        preDelayPos = (preDelayPos + 1) % preDelayBuf.size();

        /* 梳状滤波器并行处理 */
        double combSum = 0.0;
        for (int i = 0; i < nCombs; ++i) {
            int len = combBufs[i].size();
            double readVal = combBufs[i][combPos[i]];

            /* 阻尼反馈 */
            double filtered = readVal * damp2 + combFeedback[i] * damp1;
            combFeedback[i] = filtered;

            combBufs[i][combPos[i]] = delayed + filtered * fbCoeff;
            combPos[i] = (combPos[i] + 1) % len;

            combSum += readVal;
        }
        combSum /= nCombs;

        /* 全通滤波器级联 */
        double apOut = combSum;
        for (int i = 0; i < nAllpass; ++i) {
            int len = apBufs[i].size();
            double apRead = apBufs[i][apPos[i]];
            double apNew = apOut + apRead * 0.5;
            apBufs[i][apPos[i]] = apOut - apRead * 0.5;
            apOut = apRead + apNew * 0.5;
            apPos[i] = (apPos[i] + 1) % len;
        }

        /* 干湿混合 */
        double wet = apOut * m_wetDry;
        double dry = inSample * (1.0 - m_wetDry);
        output[s] = dry + wet;
    }

    /* 计算尾部电平 */
    m_tailLevel = 0.0;
    if (!output.isEmpty()) {
        double sumSq = 0.0;
        int tailStart = qMax(0, n - 100);
        for (int i = tailStart; i < n; ++i) {
            sumSq += output[i] * output[i];
        }
        m_tailLevel = qSqrt(sumSq / (n - tailStart));
    }

    /* 更新统计 */
    m_stats.totalProcessings++;
    m_stats.totalSamples += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(n, m_tailLevel);
    return output;
}

/**
 * @brief 获取当前统计信息
 * @return 处理统计结构
 */
Reverb3::Stats Reverb3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void Reverb3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
