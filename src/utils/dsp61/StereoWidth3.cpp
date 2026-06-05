/**
 * @file StereoWidth3.cpp
 * @brief 立体声宽度控制器实现
 *
 * 实现基于M/S (Mid/Side) 处理的立体声宽度控制:
 * 1. 将L/R立体声信号转换为M/S表示
 * 2. 通过调节Side分量增益控制立体声宽度
 * 3. 计算L/R通道间的互相关性
 * 4. 将M/S信号转回L/R
 * width = 0: 单声道, width = 1: 原始, width > 1: 增强立体声
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/dsp61/StereoWidth3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化立体声宽度控制器
 * @param parent 父QObject指针
 */
StereoWidth3::StereoWidth3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置立体声宽度
 * @param w 宽度系数 (默认 1.0)
 *          0.0 = 单声道 (全中置)
 *          1.0 = 原始宽度
 *          2.0 = 双倍宽度
 */
void StereoWidth3::setWidth(double w)
{
    m_width = qBound(0.0, w, 3.0);
}

/**
 * @brief 设置中心电平
 * @param level 中心通道电平 (默认 1.0)
 *
 * 调节Mid(中心)信号的增益
 */
void StereoWidth3::setCenterLevel(double level)
{
    m_centerLevel = qBound(0.0, level, 2.0);
}

/**
 * @brief 设置是否联动中心电平
 * @param link true: 增加宽度时自动降低中心电平以保持响度一致
 *             false: 独立调节
 */
void StereoWidth3::setLink(bool link)
{
    m_link = link;
}

/**
 * @brief 处理立体声信号
 *
 * 处理流程:
 * 1. 验证输入格式 (2通道，每通道长度相同)
 * 2. 对每个采样帧:
 *    a. L/R -> M/S: M = (L+R)/2, S = (L-R)/2
 *    b. 调节: M *= centerLevel, S *= width
 *    c. 如果link模式，自动调节centerLevel
 *    d. M/S -> L/R: L = M + S, R = M - S
 * 3. 计算L/R间的互相关系数
 *
 * @param input 输入立体声信号 [2][N]，input[0]=L, input[1]=R
 * @return 处理后的立体声信号 [2][N]
 */
QVector<QVector<double>> StereoWidth3::process(const QVector<QVector<double>>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> output(2);

    if (input.size() < 2 || input[0].isEmpty() || input[1].isEmpty()) {
        m_stats.totalProcessings++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalProcessings > 0)
            ? m_timeSum / m_stats.totalProcessings : 0.0;
        emit processingCompleted(0, m_width);
        return output;
    }

    const int N = qMin(input[0].size(), input[1].size());
    output[0].resize(N);
    output[1].resize(N);

    /* 计算实际使用的centerLevel (link模式) */
    double centerGain = m_centerLevel;
    if (m_link) {
        /* 联动模式: 增加宽度时降低中心电平以保持总能量 */
        centerGain = m_centerLevel / qMax(0.5, qSqrt(m_width));
    }

    double sideGain = m_width;

    /* 用于计算互相关的累积量 */
    double sumLR = 0.0;   /* sum(L*R) */
    double sumLL = 0.0;   /* sum(L*L) */
    double sumRR = 0.0;   /* sum(R*R) */

    for (int i = 0; i < N; ++i) {
        double L = input[0][i];
        double R = input[1][i];

        /* L/R -> M/S 编码 */
        double M = (L + R) * 0.5;
        double S = (L - R) * 0.5;

        /* 应用增益 */
        M *= centerGain;
        S *= sideGain;

        /* M/S -> L/R 解码 */
        output[0][i] = M + S;
        output[1][i] = M - S;

        /* 累积互相关计算量 */
        sumLR += output[0][i] * output[1][i];
        sumLL += output[0][i] * output[0][i];
        sumRR += output[1][i] * output[1][i];
    }

    /* 计算互相关系数: rho = sum(L*R) / sqrt(sum(L*L) * sum(R*R)) */
    double denom = qSqrt(sumLL * sumRR);
    if (denom > 1e-15) {
        m_correlation = sumLR / denom;
    } else {
        m_correlation = 0.0;
    }

    /* 限制互相关在 [-1, 1] */
    m_correlation = qBound(-1.0, m_correlation, 1.0);

    /* 更新统计 */
    m_stats.totalProcessings++;
    m_stats.totalSamples += N;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(N, m_width);
    return output;
}

/**
 * @brief 重置所有统计数据
 */
void StereoWidth3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_correlation = 0.0;
}
