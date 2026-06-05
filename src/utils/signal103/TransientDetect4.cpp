#include "TransientDetect4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file TransientDetect4.cpp
 * @brief 瞬态信号检测器实现
 *
 * 通过滑动窗口能量分析和自适应阈值检测瞬态事件:
 * 1. 计算短时能量包络
 * 2. 计算能量的变化率(差分)
 * 3. 变化率超过自适应阈值时标记为瞬态
 */

/**
 * @brief 构造函数，初始化默认检测参数
 * @param parent 父QObject对象指针
 */
TransientDetect4::TransientDetect4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置检测灵敏度
 * @param sensitivity 灵敏度因子(大于1更灵敏)
 */
void TransientDetect4::setSensitivity(double sensitivity)
{
    m_sensitivity = qMax(0.1, sensitivity);
}

/**
 * @brief 设置分析窗口大小
 * @param size 能量计算窗口的采样点数
 */
void TransientDetect4::setWindowSize(int size)
{
    m_windowSize = qMax(2, size);
}

/**
 * @brief 对信号执行瞬态检测
 *
 * 检测流程:
 * 1. 滑动窗口计算短时能量
 * 2. 计算能量包络的一阶差分
 * 3. 根据局部能量的中位数确定自适应阈值
 * 4. 差分超过阈值的位置标记为瞬态
 *
 * @param signal 输入信号
 * @return 检测到的瞬态事件位置列表
 */
QVector<int> TransientDetect4::detect(const QVector<double>& signal)
{
    if (signal.size() < m_windowSize * 2) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = signal.size();
    const int halfWin = m_windowSize / 2;

    // 步骤1: 计算短时能量包络
    const int envLen = N - m_windowSize + 1;
    QVector<double> energy(envLen, 0.0);

    for (int i = 0; i < envLen; ++i) {
        double sum = 0.0;
        for (int j = 0; j < m_windowSize; ++j) {
            sum += signal[i + j] * signal[i + j];
        }
        energy[i] = sum / m_windowSize;
    }

    // 步骤2: 计算能量差分(变化率)
    QVector<double> diff(envLen - 1, 0.0);
    for (int i = 0; i < envLen - 1; ++i) {
        diff[i] = energy[i + 1] - energy[i];
    }

    // 步骤3: 计算自适应阈值
    QVector<double> sortedDiff = diff;
    std::sort(sortedDiff.begin(), sortedDiff.end());
    const double median = sortedDiff[sortedDiff.size() / 2];
    const double mad = [] (const QVector<double>& v, double med) {
        QVector<double> absDev;
        absDev.reserve(v.size());
        for (double val : v) absDev.append(std::fabs(val - med));
        std::sort(absDev.begin(), absDev.end());
        return absDev[absDev.size() / 2];
    }(diff, median);

    const double threshold = median + m_sensitivity * mad * 1.4826; // 1.4826 = 1/0.6745

    // 步骤4: 检测瞬态位置
    QVector<int> positions;
    for (int i = 0; i < diff.size(); ++i) {
        if (diff[i] > threshold) {
            // 避免连续检测(最小间隔)
            if (positions.isEmpty() || (i - positions.last()) >= halfWin) {
                positions.append(i + halfWin); // 转换回原始信号位置
            }
        }
    }

    // 更新统计信息
    m_stats.totalDetected += positions.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalDetected);

    for (int pos : positions) {
        emit detected(pos);
    }

    return positions;
}

/**
 * @brief 重置所有统计信息
 */
void TransientDetect4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
