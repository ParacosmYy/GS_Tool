#include "TransientDetect5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化瞬态检测器
 * @param parent 父对象指针
 */
TransientDetect5::TransientDetect5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置检测灵敏度阈值
 * @param sensitivity 灵敏度(0.0~1.0)，值越高检测越严格
 */
void TransientDetect5::setSensitivity(double sensitivity)
{
    m_sensitivity = qBound(0.0, sensitivity, 1.0);
}

/**
 * @brief 设置分析窗口参数
 * @param windowSize 窗口长度(样本数)
 * @param hopSize 滑动步长(样本数)
 */
void TransientDetect5::setWindowParams(int windowSize, int hopSize)
{
    m_windowSize = qMax(16, windowSize);
    m_hopSize = qMax(1, hopSize);
}

/**
 * @brief 执行瞬态信号检测
 *
 * 通过短时能量变化率(spectral flux)检测信号中的瞬态事件：
 * 1. 将信号分帧计算每帧能量
 * 2. 计算帧间能量差分的正值部分(onset strength)
 * 3. 自适应阈值判断瞬态位置
 * 4. 合并过近的检测点
 *
 * @param signal 输入时域信号
 * @return 瞬态事件列表(位置, 强度)
 */
QVector<QPair<int, double>> TransientDetect5::detect(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, double>> results;
    const int n = signal.size();
    if (n < m_windowSize * 2) {
        emit detectionCompleted(0);
        return results;
    }

    /* 步骤1：计算每帧短时能量 */
    const int numFrames = (n - m_windowSize) / m_hopSize + 1;
    QVector<double> frameEnergy(numFrames);
    for (int f = 0; f < numFrames; ++f) {
        double energy = 0.0;
        int start = f * m_hopSize;
        for (int i = 0; i < m_windowSize && start + i < n; ++i) {
            energy += signal[start + i] * signal[start + i];
        }
        frameEnergy[f] = energy / m_windowSize;
    }

    /* 步骤2：计算onset strength（帧间能量差分的正值部分） */
    QVector<double> onsetStrength(numFrames, 0.0);
    for (int f = 1; f < numFrames; ++f) {
        double diff = frameEnergy[f] - frameEnergy[f - 1];
        onsetStrength[f] = qMax(0.0, diff);
    }

    /* 步骤3：自适应阈值检测 */
    double meanOnset = 0.0;
    for (double v : onsetStrength) meanOnset += v;
    meanOnset /= numFrames;

    double varOnset = 0.0;
    for (double v : onsetStrength) {
        double d = v - meanOnset;
        varOnset += d * d;
    }
    varOnset = qSqrt(varOnset / numFrames);

    double threshold = meanOnset + (1.0 - m_sensitivity) * 2.0 * varOnset
                       + 1e-10;

    /* 步骤4：峰值提取，合并过近检测点 */
    const int minGap = qMax(1, m_windowSize / m_hopSize);
    int lastDetected = -minGap - 1;

    for (int f = 1; f < numFrames - 1; ++f) {
        if (onsetStrength[f] > threshold
            && onsetStrength[f] >= onsetStrength[f - 1]
            && onsetStrength[f] >= onsetStrength[f + 1]
            && (f - lastDetected) >= minGap) {
            int position = f * m_hopSize + m_windowSize / 2;
            results.append(qMakePair(position, onsetStrength[f]));
            lastDetected = f;
        }
    }

    m_stats.totalDetected += results.size();

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalDetected > 0)
        ? m_timeSum / qMax(1, static_cast<int>(m_stats.totalDetected))
        : 0.0;

    emit detectionCompleted(results.size());
    return results;
}

/**
 * @brief 重置所有统计信息
 */
void TransientDetect5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
