/**
 * @file StereoProcessor3.cpp
 * @brief 立体声处理器实现
 *
 * 实现Mid/Side编解码处理，支持增益控制、声像法则
 * 和立体声相关度/平衡度分析。
 */

#include "utils/dsp72/StereoProcessor3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
StereoProcessor3::StereoProcessor3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置Mid增益
 * @param gain Mid通道增益（dB转线性）
 */
void StereoProcessor3::setMidGain(double gain)
{
    m_midGain = qBound(0.0, gain, 4.0);
}

/**
 * @brief 设置Side增益
 * @param gain Side通道增益（dB转线性）
 */
void StereoProcessor3::setSideGain(double gain)
{
    m_sideGain = qBound(0.0, gain, 4.0);
}

/**
 * @brief 设置声像法则
 * @param law 声像法则："constant_power"、"linear"、"equal_power"
 */
void StereoProcessor3::setPanLaw(const QString& law)
{
    if (law == "constant_power" || law == "linear" || law == "equal_power") {
        m_panLaw = law;
    }
}

/**
 * @brief 处理立体声信号
 * @param input 输入数据，input[0]=左声道，input[1]=右声道
 * @return 处理后的立体声数据
 */
QVector<QVector<double>> StereoProcessor3::process(const QVector<QVector<double>>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> output;
    if (input.size() < 2 || input[0].isEmpty()) return output;

    const int N = input[0].size();
    output.resize(2);
    output[0].resize(N);
    output[1].resize(N);

    double sumL = 0.0, sumR = 0.0;
    double sumLR = 0.0, sumLL = 0.0, sumRR = 0.0;

    for (int i = 0; i < N; ++i) {
        double L = input[0][i];
        double R = (i < input[1].size()) ? input[1][i] : 0.0;

        // Mid/Side编码
        double M = (L + R) * M_SQRT1_2;
        double S = (L - R) * M_SQRT1_2;

        // 应用增益
        M *= m_midGain;
        S *= m_sideGain;

        // Mid/Side解码回左右
        output[0][i] = (M + S) * M_SQRT1_2;
        output[1][i] = (M - S) * M_SQRT1_2;

        // 累计用于统计
        sumL += output[0][i] * output[0][i];
        sumR += output[1][i] * output[1][i];
        sumLR += output[0][i] * output[1][i];
    }

    // 计算相关系数
    double denom = qSqrt(sumL * sumR);
    m_corr = (denom > 1e-12) ? sumLR / denom : 0.0;

    // 计算平衡度：-1(全左)到+1(全右)
    double totalEnergy = sumL + sumR;
    m_balance = (totalEnergy > 1e-12) ? (sumR - sumL) / totalEnergy : 0.0;

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalProcessings++;
    m_stats.totalSamples += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(N, m_balance);
    return output;
}

/**
 * @brief 重置统计信息
 */
void StereoProcessor3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
