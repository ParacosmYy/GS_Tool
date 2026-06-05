/**
 * @file GaborTransform2.cpp
 * @brief Gabor变换实现
 *
 * 实现基于Gabor原子的时频分析，支持前向Gabor变换和逆变换。
 * Gabor变换使用高斯窗提供最优的时频分辨率。
 */

#include "utils/fft70/GaborTransform2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数并设计Gabor原子
 * @param parent 父对象指针
 */
GaborTransform2::GaborTransform2(QObject* parent)
    : QObject(parent)
{
    designGaborAtom();
}

/**
 * @brief 设置窗口大小
 * @param n 窗口大小（采样点数）
 */
void GaborTransform2::setWindowSize(int n)
{
    m_winSize = qMax(16, n);
    designGaborAtom();
}

/**
 * @brief 设置跳跃大小
 * @param hop 相邻帧之间的采样间隔
 */
void GaborTransform2::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

/**
 * @brief 设置频率bin数量
 * @param bins 频率分辨率
 */
void GaborTransform2::setNumFrequencyBins(int bins)
{
    m_numBins = qMax(4, bins);
}

/**
 * @brief 前向Gabor变换
 * @param signal 输入时域信号
 * @return 时频表示矩阵（时间帧 x 频率bin）
 */
QVector<QVector<double>> GaborTransform2::forward(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty() || m_gabor.isEmpty()) {
        return QVector<QVector<double>>();
    }

    int numFrames = qMax(1, (signal.size() - m_winSize) / m_hopSize + 1);
    int numBins = qMin(m_numBins, m_winSize / 2 + 1);

    QVector<QVector<double>> result(numFrames, QVector<double>(numBins, 0.0));

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;

        for (int k = 0; k < numBins; ++k) {
            double re = 0.0, im = 0.0;
            double freq = 2.0 * M_PI * k / m_winSize;

            for (int n = 0; n < m_winSize; ++n) {
                int idx = start + n;
                if (idx < signal.size()) {
                    double gaborVal = m_gabor[n];
                    double angle = freq * n;
                    re += signal[idx] * gaborVal * qCos(angle);
                    im -= signal[idx] * gaborVal * qSin(angle);
                }
            }
            // 幅度谱
            result[f][k] = qSqrt(re * re + im * im);
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalFrames += numFrames;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(numFrames, numBins);
    return result;
}

/**
 * @brief 逆Gabor变换
 * @param transform 时频表示矩阵
 * @return 重建的时域信号
 */
QVector<double> GaborTransform2::inverse(const QVector<QVector<double>>& transform)
{
    QElapsedTimer timer;
    timer.start();

    if (transform.isEmpty()) return QVector<double>();

    int numFrames = transform.size();
    int numBins = transform[0].size();
    int sigLen = (numFrames - 1) * m_hopSize + m_winSize;

    QVector<double> signal(sigLen, 0.0);
    QVector<double> windowSum(sigLen, 0.0);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;
        for (int n = 0; n < m_winSize && (start + n) < sigLen; ++n) {
            double sample = 0.0;
            for (int k = 0; k < numBins; ++k) {
                double freq = 2.0 * M_PI * k / m_winSize;
                // 使用幅度和余弦近似重建
                sample += transform[f][k] * qCos(freq * n) / m_winSize;
            }
            double w = m_gabor[n];
            signal[start + n] += sample * w;
            windowSum[start + n] += w * w;
        }
    }

    // 重叠相加归一化
    for (int i = 0; i < sigLen; ++i) {
        if (windowSum[i] > 1e-10) {
            signal[i] /= windowSum[i];
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;

    return signal;
}

/**
 * @brief 重置统计信息
 */
void GaborTransform2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设计Gabor原子（高斯窗函数）
 *
 * 构建归一化的高斯窗函数作为Gabor基函数。
 * 窗函数宽度sigma = winSize/6，提供良好的时频集中性。
 */
void GaborTransform2::designGaborAtom()
{
    m_gabor.resize(m_winSize);
    double sigma = m_winSize / 6.0;
    double center = (m_winSize - 1) / 2.0;
    double norm = 0.0;

    for (int i = 0; i < m_winSize; ++i) {
        double x = (i - center) / sigma;
        m_gabor[i] = qExp(-0.5 * x * x);
        norm += m_gabor[i] * m_gabor[i];
    }

    // 归一化：使能量为1
    norm = qSqrt(norm);
    if (norm > 1e-15) {
        for (double& g : m_gabor) g /= norm;
    }
}

/**
 * @brief 计算Gabor系数的能量分布
 * @param transform Gabor变换结果
 * @return 每帧的总能量
 */
QVector<double> GaborTransform2::energyDistribution(const QVector<QVector<double>>& transform) const
{
    QVector<double> energy;
    if (transform.isEmpty()) return energy;

    energy.reserve(transform.size());
    for (const auto& frame : transform) {
        double sum = 0.0;
        for (double v : frame) sum += v * v;
        energy.append(sum);
    }
    return energy;
}

/**
 * @brief 计算瞬时频率估计
 * @param transform Gabor变换结果
 * @return 每帧的峰值频率索引
 */
QVector<int> GaborTransform2::peakFrequencies(const QVector<QVector<double>>& transform) const
{
    QVector<int> peaks;
    if (transform.isEmpty()) return peaks;

    peaks.reserve(transform.size());
    for (const auto& frame : transform) {
        int bestBin = 0;
        double bestVal = 0.0;
        for (int k = 0; k < frame.size(); ++k) {
            if (frame[k] > bestVal) {
                bestVal = frame[k];
                bestBin = k;
            }
        }
        peaks.append(bestBin);
    }
    return peaks;
}
