/**
 * @file StereoProcessor3.cpp
 * @brief 立体声处理器实现
 *
 * 实现Mid/Side编解码、声像控制和立体声宽度调节，
 * 支持多种声像法则和相关度/平衡度计算。
 */

#include "utils/dsp72/StereoProcessor3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
StereoProcessor3::StereoProcessor3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置中间声道增益
 * @param gain 增益值(0.0~2.0)
 */
void StereoProcessor3::setMidGain(double gain)
{
    m_midGain = qBound(0.0, gain, 2.0);
}

/**
 * @brief 设置侧边声道增益
 * @param gain 增益值(0.0~2.0)
 */
void StereoProcessor3::setSideGain(double gain)
{
    m_sideGain = qBound(0.0, gain, 2.0);
}

/**
 * @brief 设置声像法则
 * @param law 法则名称: "constant_power" 或 "linear"
 */
void StereoProcessor3::setPanLaw(const QString& law)
{
    if (law == "constant_power" || law == "linear") {
        m_panLaw = law;
    }
}

/**
 * @brief 处理立体声信号
 * @param input 输入双声道信号 [left, right]
 * @return 处理后的双声道信号 [left, right]
 *
 * 处理流程:
 * 1. 输入校验与零值填充
 * 2. LR转MS编码
 * 3. 应用Mid/Side增益控制立体声宽度
 * 4. MS转LR解码
 * 5. 计算立体声相关度(Pearson)
 * 6. 计算声道平衡度
 */
QVector<QVector<double>> StereoProcessor3::process(const QVector<QVector<double>>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> output(2);
    if (input.size() < 2) {
        m_timeSum += timer.elapsed();
        m_stats.totalProcessings++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;
        return output;
    }

    /* 确保两个声道长度一致 */
    const int N = qMin(input[0].size(), input[1].size());
    if (N == 0) {
        m_timeSum += timer.elapsed();
        m_stats.totalProcessings++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;
        return output;
    }

    output[0].resize(N, 0.0);
    output[1].resize(N, 0.0);

    /* 步骤1: LR -> MS编码 */
    QVector<double> mid(N, 0.0);
    QVector<double> side(N, 0.0);

    for (int i = 0; i < N; ++i) {
        double left = input[0][i];
        double right = input[1][i];
        /* Mid = (L+R)/2, Side = (L-R)/2 */
        mid[i] = (left + right) * 0.5;
        side[i] = (left - right) * 0.5;
    }

    /* 步骤2: 应用Mid/Side增益(控制立体声宽度) */
    /* midGain=1.0, sideGain=1.0 保持原始宽度 */
    /* sideGain=0.0 转为单声道 */
    /* sideGain>1.0 加宽立体声 */
    for (int i = 0; i < N; ++i) {
        mid[i] *= m_midGain;
        side[i] *= m_sideGain;
    }

    /* 步骤3: MS -> LR解码 */
    for (int i = 0; i < N; ++i) {
        output[0][i] = mid[i] + side[i];  /* Left = Mid + Side */
        output[1][i] = mid[i] - side[i];  /* Right = Mid - Side */
    }

    /* 步骤4: 计算立体声相关度(Pearson相关系数) */
    double sumLR = 0.0, sumL2 = 0.0, sumR2 = 0.0;
    double sumL = 0.0, sumR = 0.0;
    for (int i = 0; i < N; ++i) {
        double l = output[0][i];
        double r = output[1][i];
        sumL += l;
        sumR += r;
        sumLR += l * r;
        sumL2 += l * l;
        sumR2 += r * r;
    }
    double meanL = sumL / N;
    double meanR = sumR / N;
    double covLR = sumLR / N - meanL * meanR;
    double varL = sumL2 / N - meanL * meanL;
    double varR = sumR2 / N - meanR * meanR;
    double denom = qSqrt(qMax(varL, 0.0) * qMax(varR, 0.0));
    m_corr = (denom > 1e-15) ? covLR / denom : 0.0;

    /* 步骤5: 计算立体声平衡度 (-1=全左, 0=居中, 1=全右) */
    double energyL = 0.0, energyR = 0.0;
    for (int i = 0; i < N; ++i) {
        energyL += output[0][i] * output[0][i];
        energyR += output[1][i] * output[1][i];
    }
    double totalEnergy = energyL + energyR;
    m_balance = (totalEnergy > 1e-15) ? (energyR - energyL) / totalEnergy : 0.0;

    /* 步骤6: 峰值限制(防止削波) */
    double peakL = 0.0, peakR = 0.0;
    for (int i = 0; i < N; ++i) {
        peakL = qMax(peakL, qAbs(output[0][i]));
        peakR = qMax(peakR, qAbs(output[1][i]));
    }
    double peakMax = qMax(peakL, peakR);
    if (peakMax > 1.0) {
        double scale = 1.0 / peakMax;
        for (int i = 0; i < N; ++i) {
            output[0][i] *= scale;
            output[1][i] *= scale;
        }
    }

    /* 更新统计信息 */
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

/**
 * @brief 计算立体声互相关函数
 * @param input 输入双声道信号
 * @param maxLag 最大延迟采样数
 * @return 互相关函数（归一化）
 */
QVector<double> StereoProcessor3::crossCorrelation(
    const QVector<QVector<double>>& input, int maxLag) const
{
    QVector<double> result;
    if (input.size() < 2) return result;

    int N = qMin(input[0].size(), input[1].size());
    maxLag = qMin(maxLag, N);
    result.resize(maxLag + 1, 0.0);

    // 计算均值
    double meanL = 0.0, meanR = 0.0;
    for (int i = 0; i < N; ++i) {
        meanL += input[0][i];
        meanR += input[1][i];
    }
    meanL /= N; meanR /= N;

    // 计算归一化因子
    double normL = 0.0, normR = 0.0;
    for (int i = 0; i < N; ++i) {
        normL += (input[0][i] - meanL) * (input[0][i] - meanL);
        normR += (input[1][i] - meanR) * (input[1][i] - meanR);
    }
    double norm = qSqrt(normL * normR);

    for (int lag = 0; lag <= maxLag; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < N - lag; ++i) {
            sum += (input[0][i] - meanL) * (input[1][i + lag] - meanR);
        }
        result[lag] = (norm > 1e-12) ? sum / norm : 0.0;
    }

    return result;
}

/**
 * @brief 计算Mid/Side能量比
 * @param input 输入双声道信号
 * @return M/S能量比（0=全Side，1=全Mid）
 */
double StereoProcessor3::midSideRatio(const QVector<QVector<double>>& input) const
{
    if (input.size() < 2 || input[0].isEmpty()) return 1.0;

    int N = qMin(input[0].size(), input[1].size());
    double midEnergy = 0.0, sideEnergy = 0.0;

    for (int i = 0; i < N; ++i) {
        double M = (input[0][i] + input[1][i]) * 0.5;
        double S = (input[0][i] - input[1][i]) * 0.5;
        midEnergy += M * M;
        sideEnergy += S * S;
    }

    double total = midEnergy + sideEnergy;
    return (total > 1e-12) ? midEnergy / total : 1.0;
}
