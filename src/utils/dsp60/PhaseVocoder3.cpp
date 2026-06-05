/**
 * @file PhaseVocoder3.cpp
 * @brief 相位声码器实现 (相位展开 + 时间拉伸 + 窗函数)
 *
 * 实现基于STFT的相位声码器，用于高质量的时间拉伸:
 * 1. 对输入信号进行分帧STFT分析
 * 2. 在频域中进行相位展开，保持相位一致性
 * 3. 根据拉伸因子调整帧间距
 * 4. 使用重叠相加(overlap-add)进行信号重建
 * 支持可配置的窗函数大小、跳跃大小和拉伸因子。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/dsp60/PhaseVocoder3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化相位声码器
 * @param parent 父QObject指针
 */
PhaseVocoder3::PhaseVocoder3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置分析窗大小
 * @param n 窗函数长度 (默认 2048)，建议为2的幂
 */
void PhaseVocoder3::setWindowSize(int n)
{
    m_winSize = qMax(64, n);
}

/**
 * @brief 设置分析跳跃大小
 * @param hop 分析帧移 (默认 512)，通常为窗大小的 1/4
 */
void PhaseVocoder3::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

/**
 * @brief 设置时间拉伸因子
 * @param factor 拉伸因子 (默认 1.5)
 *              factor > 1.0: 变慢 (拉伸)
 *              factor < 1.0: 变快 (压缩)
 *              factor = 1.0: 不变
 */
void PhaseVocoder3::setStretchFactor(double factor)
{
    m_factor = qBound(0.25, factor, 4.0);
}

/**
 * @brief 处理输入信号，进行时间拉伸
 *
 * 处理流程:
 * 1. 生成汉宁窗函数
 * 2. 对输入信号进行分帧STFT
 * 3. 在频域中进行相位展开
 * 4. 以不同的综合跳跃大小进行重叠相加
 * 5. 返回拉伸后的信号
 *
 * @param input 输入音频信号
 * @return 时间拉伸后的音频信号
 */
QVector<double> PhaseVocoder3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int inLen = input.size();
    QVector<double> output;

    if (inLen == 0 || m_winSize <= 0 || m_hopSize <= 0) {
        m_stats.totalProcessings++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalProcessings > 0)
            ? m_timeSum / m_stats.totalProcessings : 0.0;
        emit processingCompleted(0, 0);
        return output;
    }

    /* 生成汉宁窗 */
    QVector<double> window = generateWindow(m_winSize);

    /* 计算分析帧数 */
    int numFrames = qMax(1, static_cast<int>(qCeil(
        static_cast<double>(inLen - m_winSize) / m_hopSize)) + 1);

    /* 综合跳跃 = 分析跳跃 * 拉伸因子 */
    int synthHop = qMax(1, static_cast<int>(qRound(m_hopSize * m_factor)));

    /* 输出缓冲区 */
    int outLen = (numFrames - 1) * synthHop + m_winSize;
    QVector<double> outBuf(outLen, 0.0);
    QVector<double> winSum(outLen, 0.0);

    /* 保存上一帧的相位用于相位展开 */
    int fftBins = m_winSize / 2 + 1;
    QVector<double> prevPhase(fftBins, 0.0);
    QVector<double> prevPhaseOut(fftBins, 0.0);

    /* 期望的相位增量 (分析跳跃对应的相位变化) */
    double omegaBase = 2.0 * M_PI * m_hopSize / m_winSize;

    for (int frame = 0; frame < numFrames; ++frame) {
        int startSample = frame * m_hopSize;

        /* 提取当前帧并加窗 */
        QVector<double> frameData(m_winSize, 0.0);
        for (int i = 0; i < m_winSize; ++i) {
            int idx = startSample + i;
            frameData[i] = (idx < inLen) ? input[idx] * window[i] : 0.0;
        }

        /* 计算STFT (DFT) */
        QVector<double> magnitude(fftBins, 0.0);
        QVector<double> phase(fftBins, 0.0);

        for (int k = 0; k < fftBins; ++k) {
            double re = 0.0;
            double im = 0.0;
            for (int n = 0; n < m_winSize; ++n) {
                double angle = -2.0 * M_PI * k * n / m_winSize;
                re += frameData[n] * qCos(angle);
                im += frameData[n] * qSin(angle);
            }
            magnitude[k] = qSqrt(re * re + im * im);
            phase[k] = qAtan2(im, re);
        }

        /* 相位展开 */
        QVector<double> newPhase(fftBins, 0.0);
        for (int k = 0; k < fftBins; ++k) {
            /* 计算相位差 */
            double dPhi = phase[k] - prevPhase[k];

            /* 减去期望的相位增量 */
            double expected = omegaBase * k;
            double deviation = dPhi - expected;

            /* 将偏差缠绕到 [-pi, pi] */
            deviation = deviation - 2.0 * M_PI * qRound(deviation / (2.0 * M_PI));

            /* 计算真实频率 */
            double trueFreq = expected + deviation;

            /* 计算输出相位 */
            newPhase[k] = prevPhaseOut[k] + trueFreq * m_factor;
        }

        prevPhase = phase;
        prevPhaseOut = newPhase;

        /* 逆DFT重建时域帧 */
        QVector<double> synthFrame(m_winSize, 0.0);
        for (int n = 0; n < m_winSize; ++n) {
            double val = 0.0;
            /* 直流分量 */
            val += magnitude[0] * qCos(newPhase[0]);
            /* 正频率分量 (对称) */
            for (int k = 1; k < fftBins - 1; ++k) {
                double angle = 2.0 * M_PI * k * n / m_winSize + newPhase[k];
                val += 2.0 * magnitude[k] * qCos(angle);
            }
            /* Nyquist分量 */
            if (fftBins > 1) {
                val += magnitude[fftBins - 1] * qCos(newPhase[fftBins - 1] + M_PI * n);
            }
            synthFrame[n] = val / m_winSize;
        }

        /* 加窗并重叠相加 */
        int synthStart = frame * synthHop;
        for (int i = 0; i < m_winSize && synthStart + i < outLen; ++i) {
            outBuf[synthStart + i] += synthFrame[i] * window[i];
            winSum[synthStart + i] += window[i] * window[i];
        }
    }

    /* 归一化: 除以窗函数重叠和 */
    for (int i = 0; i < outLen; ++i) {
        if (winSum[i] > 1e-8) {
            outBuf[i] /= winSum[i];
        }
    }

    /* 修剪输出为预期长度 */
    int expectedOutLen = outputLength(inLen);
    int finalLen = qMin(outLen, expectedOutLen);
    output = QVector<double>(outBuf.begin(), outBuf.begin() + finalLen);

    /* 更新统计 */
    m_stats.totalProcessings++;
    m_stats.totalFrames += numFrames;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(inLen, finalLen);
    return output;
}

/**
 * @brief 计算给定输入长度的输出长度
 * @param inputLen 输入信号长度
 * @return 输出信号的预期长度
 */
int PhaseVocoder3::outputLength(int inputLen) const
{
    if (inputLen <= m_winSize) {
        return m_winSize;
    }
    int numFrames = qMax(1, (inputLen - m_winSize) / m_hopSize + 1);
    int synthHop = qMax(1, static_cast<int>(qRound(m_hopSize * m_factor)));
    return (numFrames - 1) * synthHop + m_winSize;
}

/**
 * @brief 重置所有统计数据
 */
void PhaseVocoder3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 生成汉宁窗函数
 *
 * w(n) = 0.5 * (1 - cos(2*pi*n / (N-1)))
 *
 * @param n 窗函数长度
 * @return 汉宁窗系数
 */
QVector<double> PhaseVocoder3::generateWindow(int n) const
{
    QVector<double> win(n);
    for (int i = 0; i < n; ++i) {
        win[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
    }
    return win;
}
