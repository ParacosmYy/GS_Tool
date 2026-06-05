/**
 * @file PitchShift.cpp
 * @brief 变调器实现 — 相位声码器/频域拉伸/重采样/音高缩放
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/signal31/PitchShift.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <vector>

/** @brief 构造函数 @param parent 父对象 */
PitchShift::PitchShift(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置音高缩放因子(>1升调, <1降调) @param factor 缩放因子 */
void PitchShift::setShiftFactor(double factor)
{
    m_shiftFactor = qBound(0.25, factor, 4.0);
}

/** @brief 设置FFT窗口大小(必须是2的幂) @param samples 窗口采样数 */
void PitchShift::setWindowSize(int samples)
{
    /* 确保是2的幂 */
    int p = 1;
    while (p < samples) p <<= 1;
    m_windowSize = qMax(64, p);
    m_prevPhase.clear();
    m_synPhase.clear();
}

/** @brief 设置跳跃大小 @param samples 跳跃采样数 */
void PitchShift::setHopSize(int samples)
{
    m_hopSize = qMax(1, samples);
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void PitchShift::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief Cooley-Tukey基2就地FFT(原地计算)
 *  @param real 实部数组
 *  @param imag 虚部数组
 */
void PitchShift::forwardFFT(QVector<double>& real, QVector<double>& imag)
{
    int N = real.size();
    if (N <= 1) return;

    /* 位反转排列 */
    int bits = 0;
    while ((1 << bits) < N) ++bits;
    for (int i = 0; i < N; ++i) {
        int j = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b)) j |= (1 << (bits - 1 - b));
        }
        if (j > i) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= N; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wReal = qCos(angle);
        double wImag = qSin(angle);
        for (int i = 0; i < N; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tReal = curReal * real[v] - curImag * imag[v];
                double tImag = curReal * imag[v] + curImag * real[v];
                real[v] = real[u] - tReal;
                imag[v] = imag[u] - tImag;
                real[u] += tReal;
                imag[u] += tImag;
                double newCurR = curReal * wReal - curImag * wImag;
                double newCurI = curReal * wImag + curImag * wReal;
                curReal = newCurR;
                curImag = newCurI;
            }
        }
    }
}

/** @brief 逆FFT(对forwardFFT结果取共轭后再做FFT然后除以N)
 *  @param real 实部数组
 *  @param imag 虚部数组
 */
void PitchShift::inverseFFT(QVector<double>& real, QVector<double>& imag)
{
    int N = real.size();
    /* 取共轭 */
    for (int i = 0; i < N; ++i) {
        imag[i] = -imag[i];
    }
    forwardFFT(real, imag);
    /* 取共轭并归一化 */
    for (int i = 0; i < N; ++i) {
        real[i] /= N;
        imag[i] = -imag[i] / N;
    }
}

/** @brief 相位声码器处理: 时间拉伸+重采样实现变调
 *  @param input 输入采样数据
 *  @return 变调后的采样数据
 */
QVector<double> PitchShift::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = m_windowSize;
    int hopA = m_hopSize;
    int hopS = m_hopSize;
    int halfN = N / 2;

    /* 初始化相位累加器 */
    if (m_prevPhase.size() != halfN) {
        m_prevPhase.fill(0.0, halfN);
    }
    if (m_synPhase.size() != halfN) {
        m_synPhase.fill(0.0, halfN);
    }

    /* Hann窗函数 */
    QVector<double> window(N);
    for (int i = 0; i < N; ++i) {
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / N));
    }

    /* 分析帧数 */
    int numFrames = qMax(1, (input.size() - N) / hopA + 1);
    double stretchFactor = 1.0 / m_shiftFactor;
    int hopOut = qRound(hopS * stretchFactor);
    if (hopOut < 1) hopOut = 1;

    /* 输出缓冲区 */
    int outputLen = numFrames * hopOut + N;
    QVector<double> output(outputLen, 0.0);
    QVector<double> outputWin(outputLen, 0.0);

    double expectedPhaseInc = 2.0 * M_PI * hopA / N;

    for (int frame = 0; frame < numFrames; ++frame) {
        int pos = frame * hopA;

        /* 加窗提取帧 */
        QVector<double> frameReal(N, 0.0), frameImag(N, 0.0);
        for (int i = 0; i < N; ++i) {
            int idx = pos + i;
            frameReal[i] = (idx < input.size()) ?
                input[idx] * window[i] : 0.0;
        }

        /* 前向FFT */
        forwardFFT(frameReal, frameImag);

        /* 相位分析和综合 */
        for (int k = 0; k < halfN; ++k) {
            double mag = qSqrt(frameReal[k] * frameReal[k] +
                               frameImag[k] * frameImag[k]);
            double phase = qAtan2(frameImag[k], frameReal[k]);

            /* 计算相位差 */
            double dPhi = phase - m_prevPhase[k];
            m_prevPhase[k] = phase;

            /* 去除预期相位增量，得到偏差 */
            dPhi -= expectedPhaseInc * k;

            /* 将相位偏差包裹到[-pi, pi] */
            while (dPhi > M_PI) dPhi -= 2.0 * M_PI;
            while (dPhi < -M_PI) dPhi += 2.0 * M_PI;

            /* 计算瞬时频率 */
            double instFreq = expectedPhaseInc * k + dPhi;

            /* 累积综合相位(按输出hop缩放) */
            m_synPhase[k] += instFreq * stretchFactor;

            /* 重建频域表示 */
            frameReal[k] = mag * qCos(m_synPhase[k]);
            frameImag[k] = mag * qSin(m_synPhase[k]);
        }

        /* 镜像对称(实信号FFT共轭对称) */
        for (int k = halfN; k < N; ++k) {
            int mirror = N - k;
            frameReal[k] = frameReal[mirror];
            frameImag[k] = -frameImag[mirror];
        }

        /* 逆FFT */
        inverseFFT(frameReal, frameImag);

        /* 加窗并叠加到输出 */
        int outPos = frame * hopOut;
        for (int i = 0; i < N; ++i) {
            int idx = outPos + i;
            if (idx < outputLen) {
                output[idx] += frameReal[i] * window[i];
                outputWin[idx] += window[i] * window[i];
            }
        }
    }

    /* 归一化(窗函数重叠补偿) */
    for (int i = 0; i < outputLen; ++i) {
        if (outputWin[i] > 1e-6) {
            output[i] /= outputWin[i];
        }
    }

    /* 裁剪尾部零填充 */
    while (!output.isEmpty() && qAbs(output.back()) < 1e-10) {
        output.removeLast();
    }

    m_stats.totalShifts++;
    m_stats.totalSamplesProcessed += input.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = (m_stats.totalShifts > 0) ?
        m_timeSum / m_stats.totalShifts : 0.0;
    emit shiftComplete(input.size(), output.size());
    return output;
}

/** @brief 以半音为单位进行变调
 *  @param input 输入采样数据
 *  @param semitones 半音偏移量(正=升调, 负=降调)
 *  @return 变调后的采样数据
 */
QVector<double> PitchShift::pitchShift(const QVector<double>& input,
                                        double semitones)
{
    double oldFactor = m_shiftFactor;
    m_shiftFactor = qPow(2.0, semitones / 12.0);
    QVector<double> result = process(input);
    m_shiftFactor = oldFactor;
    return result;
}

/** @brief 重置内部状态(相位累加器) */
void PitchShift::reset()
{
    m_prevPhase.clear();
    m_synPhase.clear();
}

/** @brief 重置所有统计计数器 */
void PitchShift::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
