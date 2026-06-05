#include "TransientDetect6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化瞬态检测器
 * @param parent 父对象指针
 */
TransientDetect6::TransientDetect6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void TransientDetect6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置检测参数
 * @param sensitivity 灵敏度 ("low"/"medium"/"high")
 * @param minIntervalMs 最小瞬态间隔(ms)
 * @param sampleRate 采样率(Hz)
 */
void TransientDetect6::setDetectionParams(const QString& sensitivity,
                                           double minIntervalMs,
                                           double sampleRate)
{
    Q_UNUSED(sensitivity)
    Q_UNUSED(minIntervalMs)
    Q_UNUSED(sampleRate)
}

/**
 * @brief 检测信号中的瞬态位置
 *
 * 多特征融合检测方法：
 * 1. 计算短时能量包络
 * 2. 计算能量导数
 * 3. 自适应阈值峰值检测
 * 4. 合并过近的检测点
 *
 * @param signal 输入音频信号
 * @param threshold 检测阈值 [0, 1]
 * @return 瞬态发生位置的采样点索引
 */
QVector<int> TransientDetect6::detect(const QVector<double>& signal,
                                       double threshold)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> positions;
    const int n = signal.size();
    if (n < 256) {
        emit detectionCompleted(0);
        return positions;
    }

    /* 计算短时能量 */
    const int hopSize = 256;
    const int winSize = 256;
    const int numFrames = (n - winSize) / hopSize + 1;

    QVector<double> energy(numFrames, 0.0);
    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;
        double sum = 0.0;
        for (int i = 0; i < winSize && start + i < n; ++i) {
            sum += signal[start + i] * signal[start + i];
        }
        energy[f] = sum / winSize;
    }

    /* 计算能量差分（正值部分） */
    QVector<double> diff(numFrames, 0.0);
    for (int f = 1; f < numFrames; ++f) {
        diff[f] = qMax(0.0, energy[f] - energy[f - 1]);
    }

    /* 自适应阈值 */
    double meanDiff = 0.0;
    for (double d : diff) meanDiff += d;
    meanDiff /= numFrames;

    double varDiff = 0.0;
    for (double d : diff) {
        double d2 = d - meanDiff;
        varDiff += d2 * d2;
    }
    double sigma = qSqrt(varDiff / numFrames);
    double adaptiveThreshold = meanDiff + threshold * 3.0 * sigma + 1e-10;

    /* 峰值检测 */
    const int minGapFrames = 3;
    int lastPeak = -minGapFrames - 1;

    for (int f = 1; f < numFrames - 1; ++f) {
        if (diff[f] > adaptiveThreshold
            && diff[f] >= diff[f - 1]
            && diff[f] >= diff[f + 1]
            && (f - lastPeak) >= minGapFrames) {
            positions.append(f * hopSize + winSize / 2);
            lastPeak = f;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDetectOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetectOps;

    emit detectionCompleted(positions.size());
    return positions;
}

/**
 * @brief 计算频谱通量特征
 *
 * 频谱通量 = sum(max(0, |X_t| - |X_{t-1}|))
 * 衡量帧间频谱幅度的上升量，用于onset检测。
 *
 * @param frames 分帧后的频谱序列
 * @return 各帧的频谱通量值
 */
QVector<double> TransientDetect6::spectralFlux(
    const QVector<QVector<double>>& frames)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> flux;
    const int numFrames = frames.size();
    if (numFrames < 2) {
        emit detectionCompleted(0);
        return flux;
    }

    flux.resize(numFrames);
    flux[0] = 0.0;

    for (int f = 1; f < numFrames; ++f) {
        const int len = qMin(frames[f].size(), frames[f - 1].size());
        double sum = 0.0;
        for (int k = 0; k < len; ++k) {
            double diff = frames[f][k] - frames[f - 1][k];
            sum += qMax(0.0, diff);
        }
        flux[f] = sum;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDetectOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetectOps;

    emit detectionCompleted(numFrames);
    return flux;
}

/**
 * @brief 计算能量导数特征
 *
 * 对信号分帧计算短时能量，然后计算一阶差分。
 *
 * @param signal 输入信号
 * @param hopSize 帧移
 * @return 能量导数序列
 */
QVector<double> TransientDetect6::energyDerivative(
    const QVector<double>& signal, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> derivative;
    const int n = signal.size();
    hopSize = qMax(1, hopSize);
    const int winSize = hopSize * 2;
    if (n < winSize * 2) {
        emit detectionCompleted(0);
        return derivative;
    }

    const int numFrames = (n - winSize) / hopSize + 1;
    QVector<double> energy(numFrames);
    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;
        double sum = 0.0;
        for (int i = 0; i < winSize && start + i < n; ++i) {
            sum += signal[start + i] * signal[start + i];
        }
        energy[f] = sum;
    }

    derivative.resize(numFrames);
    derivative[0] = 0.0;
    for (int f = 1; f < numFrames; ++f) {
        derivative[f] = energy[f] - energy[f - 1];
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDetectOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetectOps;

    emit detectionCompleted(numFrames);
    return derivative;
}
