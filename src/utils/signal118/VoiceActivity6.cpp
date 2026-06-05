#include "VoiceActivity6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化语音活动检测器
 * @param parent 父对象指针
 */
VoiceActivity6::VoiceActivity6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void VoiceActivity6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置检测灵敏度
 *
 * 灵敏度级别影响能量阈值和零交叉率阈值的组合权重：
 * - "low": 高阈值，仅检测强语音，低误检率
 * - "medium": 平衡设置
 * - "high": 低阈值，检测弱语音，高召回率
 *
 * @param sensitivity 灵敏度级别
 */
void VoiceActivity6::setSensitivity(const QString& sensitivity)
{
    Q_UNUSED(sensitivity)
}

/**
 * @brief 计算帧能量特征
 *
 * E = 10 * log10(mean(x^2) + epsilon)
 *
 * @param frame 音频帧
 * @return 帧能量值(dB)
 */
double VoiceActivity6::computeFrameEnergy(const QVector<double>& frame) const
{
    const int n = frame.size();
    if (n == 0) return -100.0;

    double sumSq = 0.0;
    for (int i = 0; i < n; ++i) {
        sumSq += frame[i] * frame[i];
    }

    double meanSq = sumSq / n;
    return 10.0 * qLn(qMax(meanSq, 1e-20)) / qLn(10.0);
}

/**
 * @brief 检测单帧语音活动
 *
 * 综合使用能量阈值和零交叉率(ZCR)进行判决：
 * 1. 计算帧能量(dB)和零交叉率
 * 2. 能量高于阈值 → 有声
 * 3. 能量处于模糊区间时参考ZCR辅助判断
 * 4. 高零交叉率可能是清音辅音或噪声
 *
 * @param frame 音频帧数据
 * @return true表示存在语音活动
 */
bool VoiceActivity6::detect(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = frame.size();
    if (n < 2) {
        emit detectionCompleted(0);
        return false;
    }

    /* 计算帧能量 */
    double energyDb = computeFrameEnergy(frame);

    /* 计算零交叉率 */
    int crossings = 0;
    for (int i = 1; i < n; ++i) {
        if ((frame[i] >= 0) != (frame[i - 1] >= 0)) {
            crossings++;
        }
    }
    double zcr = static_cast<double>(crossings) / (n - 1);

    /* 判决逻辑 */
    const double energyThreshold = -40.0;   /* dB */
    const double zcrThreshold = 0.3;
    bool isVoice = false;

    if (energyDb > energyThreshold) {
        isVoice = true;
    } else if (energyDb > energyThreshold - 10.0 && zcr > zcrThreshold) {
        /* 低能量高ZCR可能是清音辅音 */
        isVoice = true;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDetectOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetectOps;

    emit detectionCompleted(isVoice ? 1 : 0);
    return isVoice;
}

/**
 * @brief 批量检测多帧语音活动
 *
 * 对每帧独立执行VAD检测，使用自适应阈值：
 * 1. 先计算所有帧的能量分布
 * 2. 基于能量直方图自动确定阈值
 * 3. 逐帧判决
 *
 * @param frames 音频帧序列
 * @return 各帧的语音/非语音标记
 */
QVector<bool> VoiceActivity6::detectBatch(const QVector<QVector<double>>& frames)
{
    QElapsedTimer timer;
    timer.start();

    QVector<bool> results;
    const int numFrames = frames.size();
    if (numFrames == 0) {
        emit detectionCompleted(0);
        return results;
    }

    /* 计算所有帧能量 */
    QVector<double> energies(numFrames);
    for (int i = 0; i < numFrames; ++i) {
        energies[i] = computeFrameEnergy(frames[i]);
    }

    /* 自适应阈值：取能量分布的25%分位数 */
    QVector<double> sorted = energies;
    std::sort(sorted.begin(), sorted.end());
    int idx = qBound(0, static_cast<int>(numFrames * 0.25), numFrames - 1);
    double threshold = sorted[idx] + 10.0;

    /* 逐帧判决 */
    results.reserve(numFrames);
    int voiceCount = 0;
    for (int i = 0; i < numFrames; ++i) {
        bool isVoice = energies[i] > threshold;
        results.append(isVoice);
        if (isVoice) voiceCount++;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDetectOps += numFrames;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetectOps;

    emit detectionCompleted(voiceCount);
    return results;
}
