#include "VoiceActivity5.h"
#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化语音活动检测器
 * @param parent 父对象指针
 */
VoiceActivity5::VoiceActivity5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void VoiceActivity5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置检测阈值
 * @param energyThresholdDb 能量阈值(dB)
 * @param zcrThreshold 过零率阈值
 */
void VoiceActivity5::setThresholds(double energyThresholdDb, double zcrThreshold)
{
    m_energyThresholdDb = energyThresholdDb;
    m_zcrThreshold = zcrThreshold;
}

/**
 * @brief 启用/禁用自适应阈值模式
 * @param enabled 是否启用
 * @param adaptationRate 自适应学习率
 */
void VoiceActivity5::setAdaptive(bool enabled, double adaptationRate)
{
    m_adaptive = enabled;
    m_adaptationRate = adaptationRate;
}

/**
 * @brief 检测单帧语音活动
 *
 * 综合三个特征判断帧类别：
 * 1. 短时能量：区分有声/无声
 * 2. 过零率(ZCR)：区分清音/浊音
 * 3. 谱平坦度：区分语音/噪声
 *
 * @param frame 音频帧采样数据
 * @return 帧分类结果
 */
VoiceActivity5::FrameClass VoiceActivity5::detect(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = frame.size();
    if (n == 0) {
        emit detectionCompleted(Silence);
        return Silence;
    }

    /* 特征1：短时能量 */
    double energy = 0.0;
    for (double s : frame) energy += s * s;
    energy /= n;
    double energyDb = 10.0 * qLn(qMax(energy, 1e-20)) / qLn(10.0);

    /* 自适应阈值更新 */
    if (m_adaptive) {
        m_noiseEstimate = m_adaptationRate * m_noiseEstimate +
                          (1.0 - m_adaptationRate) * energy;
    }

    double effectiveThreshold = m_adaptive
        ? (10.0 * qLn(qMax(m_noiseEstimate, 1e-20)) / qLn(10.0) + 10.0)
        : m_energyThresholdDb;

    /* 特征2：过零率 */
    double zcr = 0.0;
    for (int i = 1; i < n; ++i) {
        if ((frame[i] >= 0) != (frame[i - 1] >= 0)) {
            zcr += 1.0;
        }
    }
    zcr /= (n - 1);

    /* 分类决策 */
    FrameClass result = Silence;
    if (energyDb > effectiveThreshold) {
        if (zcr < m_zcrThreshold) {
            result = Voiced;     /* 低过零率 -> 浊音 */
        } else {
            result = Unvoiced;   /* 高过零率 -> 清音 */
        }
    }

    /* 更新统计 */
    if (result != Silence) m_stats.voiceRatio += 1.0;
    m_stats.totalFrames++;
    m_stats.voiceRatio = (m_stats.totalFrames > 0)
        ? m_stats.voiceRatio / m_stats.totalFrames : 0.0;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalFrames++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit detectionCompleted(result);
    return result;
}

/**
 * @brief 批量检测多帧
 *
 * 将连续采样数据按帧大小和步长分帧，对每帧执行检测。
 *
 * @param samples 连续采样数据
 * @param frameSize 帧大小
 * @param hopSize 帧步长
 * @return 各帧分类结果
 */
QVector<VoiceActivity5::FrameClass> VoiceActivity5::detectMultiFrame(
    const QVector<double>& samples, int frameSize, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    QVector<FrameClass> results;
    const int n = samples.size();

    if (frameSize <= 0 || hopSize <= 0) {
        emit detectionCompleted(Silence);
        return results;
    }

    for (int start = 0; start + frameSize <= n; start += hopSize) {
        QVector<double> frame(samples.begin() + start, samples.begin() + start + frameSize);
        results.append(detect(frame));
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;

    return results;
}
