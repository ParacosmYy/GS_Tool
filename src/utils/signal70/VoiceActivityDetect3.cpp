/**
 * @file VoiceActivityDetect3.cpp
 * @brief 语音活动检测(VAD)实现
 *
 * 实现基于能量和过零率的双特征语音活动检测，支持帧级检测、
 * 悬挂(hangover)机制和语音比例统计。适用于语音通信前端处理。
 */

#include "utils/signal70/VoiceActivityDetect3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
VoiceActivityDetect3::VoiceActivityDetect3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率(Hz)
 */
void VoiceActivityDetect3::setSampleRate(double sr)
{
    m_sampleRate = qBound(8000.0, sr, 96000.0);
}

/**
 * @brief 设置帧大小
 * @param size 每帧采样点数
 */
void VoiceActivityDetect3::setFrameSize(int size)
{
    m_frameSize = qMax(64, size);
}

/**
 * @brief 设置悬挂帧数
 * @param frames 检测到语音结束后保持为语音的额外帧数
 */
void VoiceActivityDetect3::setHangover(int frames)
{
    m_hangover = qBound(0, frames, 100);
}

/**
 * @brief 检测信号中的语音活动
 * @param signal 输入语音信号
 * @return 每帧的语音检测结果（1=语音，0=静音）
 *
 * 检测流程：
 * 1. 分帧计算每帧的能量和过零率
 * 2. 使用前几帧估计噪声基底
 * 3. 自适应阈值判定（能量+过零率双特征）
 * 4. 应用hangover机制平滑检测结果
 */
QVector<int> VoiceActivityDetect3::detect(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (signal.isEmpty()) return result;

    int numFrames = signal.size() / m_frameSize;
    result.reserve(numFrames);

    // 第一遍：计算所有帧的能量和过零率
    QVector<double> energies(numFrames);
    QVector<double> zcrs(numFrames);
    for (int f = 0; f < numFrames; ++f) {
        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize; ++i) {
            frame[i] = signal[f * m_frameSize + i];
        }
        energies[f] = computeEnergy(frame);
        zcrs[f] = computeZCR(frame);
    }

    // 第二遍：估计噪声水平（使用前5帧，假设为静音）
    int noiseFrames = qMin(5, numFrames);
    double noiseEnergy = 0.0;
    double noiseZCR = 0.0;
    for (int f = 0; f < noiseFrames; ++f) {
        noiseEnergy += energies[f];
        noiseZCR += zcrs[f];
    }
    noiseEnergy /= qMax(noiseFrames, 1);
    noiseZCR /= qMax(noiseFrames, 1);

    // 自适应阈值
    double energyThreshold = qMax(noiseEnergy * 2.5, 1e-8);
    double zcrHighThreshold = noiseZCR + 0.3;

    // 第三遍：逐帧检测 + hangover平滑
    m_speechFrames = 0;
    int hangoverCount = 0;

    for (int f = 0; f < numFrames; ++f) {
        bool isSpeech = false;

        // 能量特征判定
        if (energies[f] > energyThreshold) {
            isSpeech = true;
        }

        // 过零率辅助判定（区分高频噪声）
        if (isSpeech && zcrs[f] > zcrHighThreshold + 0.2) {
            // 高过零率+低能量→可能是噪声
            if (energies[f] < energyThreshold * 5.0) {
                isSpeech = false;
            }
        }

        // Hangover机制：语音结束后保持N帧
        if (isSpeech) {
            hangoverCount = m_hangover;
        } else if (hangoverCount > 0) {
            isSpeech = true;
            hangoverCount--;
        }

        result.append(isSpeech ? 1 : 0);
        if (isSpeech) {
            m_speechFrames++;
            emit voiceDetected(f, energies[f]);
        }
    }

    // 计算语音比例
    m_speechRatio = (numFrames > 0) ? static_cast<double>(m_speechFrames) / numFrames : 0.0;

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalDetections++;
    m_stats.totalFrames += numFrames;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    return result;
}

/**
 * @brief 重置统计信息
 */
void VoiceActivityDetect3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算帧的RMS能量
 * @param frame 输入帧
 * @return 帧的RMS能量值
 */
double VoiceActivityDetect3::computeEnergy(const QVector<double>& frame) const
{
    if (frame.isEmpty()) return 0.0;
    double sumSq = 0.0;
    for (double s : frame) {
        sumSq += s * s;
    }
    return qSqrt(sumSq / frame.size());
}

/**
 * @brief 计算帧的过零率
 * @param frame 输入帧
 * @return 过零率（0~1之间）
 */
double VoiceActivityDetect3::computeZCR(const QVector<double>& frame) const
{
    if (frame.size() < 2) return 0.0;
    int crossings = 0;
    for (int i = 1; i < frame.size(); ++i) {
        if ((frame[i] >= 0.0) != (frame[i - 1] >= 0.0)) {
            crossings++;
        }
    }
    return static_cast<double>(crossings) / (frame.size() - 1);
}

/**
 * @brief 计算帧的峰值能量
 * @param frame 输入帧
 * @return 帧的峰值绝对值
 */
double VoiceActivityDetect3::computePeak(const QVector<double>& frame) const
{
    if (frame.isEmpty()) return 0.0;
    double peak = 0.0;
    for (double s : frame) {
        peak = qMax(peak, qAbs(s));
    }
    return peak;
}

/**
 * @brief 计算帧的频谱质心（简化版）
 * @param frame 输入帧
 * @return 频谱质心估计（归一化频率0~0.5）
 *
 * 使用DFT幅度谱的加权平均估计频谱质心，
 * 用于区分语音（低质心）和噪声（高质心）。
 */
double VoiceActivityDetect3::computeSpectralCentroid(const QVector<double>& frame) const
{
    int N = frame.size();
    if (N == 0) return 0.0;

    // 简化DFT计算幅度谱
    int numBins = N / 2 + 1;
    double weightedSum = 0.0;
    double totalWeight = 0.0;

    for (int k = 0; k < numBins; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = 2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im -= frame[n] * qSin(angle);
        }
        double mag = qSqrt(re * re + im * im);
        weightedSum += k * mag;
        totalWeight += mag;
    }

    return (totalWeight > 1e-12) ? weightedSum / (totalWeight * N) : 0.0;
}
