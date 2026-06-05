/**
 * @file StereoWidth3.cpp
 * @brief 立体声宽度控制器实现
 *
 * 实现基于M/S (Mid/Side) 处理的立体声宽度控制:
 * 1. 将L/R立体声信号转换为M/S表示
 *    M = (L + R) / 2  (中心/和信号)
 *    S = (L - R) / 2  (侧边/差信号)
 * 2. 通过调节Side分量增益控制立体声宽度
 *    width = 0: 单声道 (Side = 0)
 *    width = 1: 原始宽度
 *    width > 1: 增强立体声 (Side放大)
 * 3. 计算L/R通道间的互相关系数
 * 4. 将M/S信号转回L/R: L = M + S, R = M - S
 *
 * 互相关系数rho的范围和含义:
 * - rho = +1.0: 完全相关 (L=R，单声道)
 * - rho = 0.0: 不相关 (L与R独立，最大立体声宽度)
 * - rho = -1.0: 完全反相关 (L=-R，相位相反)
 *
 * 应用场景:
 * - 立体声宽度调节: width=0合并为单声道，width>1增强立体感
 * - 中心声像提取: 设置width=0只保留Mid信号
 * - 立体声兼容性检查: 监测互相关系数避免反相问题
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
 *
 * 默认参数: width=1.0 (原始), centerLevel=1.0, link=true
 */
StereoWidth3::StereoWidth3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置立体声宽度
 * @param w 宽度系数 (默认 1.0)
 *          0.0 = 单声道 (全中置，Side通道静音)
 *          1.0 = 原始宽度不变
 *          2.0 = 双倍宽度 (Side通道放大2倍)
 *
 * 宽度通过调节Side通道增益实现:
 * Side_out = Side_in * width
 */
void StereoWidth3::setWidth(double w)
{
    m_width = qBound(0.0, w, 3.0);
}

/**
 * @brief 设置中心电平
 * @param level 中心通道电平 (默认 1.0)
 *
 * 独立调节Mid(中心)信号的增益:
 * Mid_out = Mid_in * centerLevel
 * 增大centerLevel增强中心声像，减小则削弱
 */
void StereoWidth3::setCenterLevel(double level)
{
    m_centerLevel = qBound(0.0, level, 2.0);
}

/**
 * @brief 设置是否联动中心电平
 * @param link true: 增加宽度时自动降低中心电平以保持响度一致
 *             false: 独立调节width和centerLevel
 *
 * 联动模式下: centerGain = centerLevel / sqrt(width)
 * 这样可以在调节宽度时保持总能量大致恒定
 */
void StereoWidth3::setLink(bool link)
{
    m_link = link;
}

/**
 * @brief 处理立体声信号
 *
 * 处理流程 (逐采样):
 * 1. 验证输入格式 (2通道，每通道长度相同)
 * 2. M/S编码: M = (L+R)/2, S = (L-R)/2
 * 3. 增益调节: M *= centerGain, S *= width
 * 4. M/S解码: L = M + S, R = M - S
 * 5. 计算L/R间的互相关系数
 *
 * 互相关公式:
 * rho = sum(L*R) / sqrt(sum(L*L) * sum(R*R))
 *
 * @param input 输入立体声信号 [2][N]，input[0]=L通道, input[1]=R通道
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
    double sumLR = 0.0;   /* sum(L_out * R_out) */
    double sumLL = 0.0;   /* sum(L_out * L_out) */
    double sumRR = 0.0;   /* sum(R_out * R_out) */

    /* 逐采样处理 */
    for (int i = 0; i < N; ++i) {
        double L = input[0][i];
        double R = input[1][i];

        /* 步骤1: L/R -> M/S 编码 */
        /* Mid信号: L和R的平均值，代表中心声像 */
        double M = (L + R) * 0.5;
        /* Side信号: L和R的差值，代表立体声差异 */
        double S = (L - R) * 0.5;

        /* 步骤2: 应用增益 */
        /* centerGain控制中心声像强度，sideGain控制立体声宽度 */
        M *= centerGain;
        S *= sideGain;

        /* 步骤3: M/S -> L/R 解码 */
        /* L = M + S, R = M - S (原始M/S编解码的逆变换) */
        output[0][i] = M + S;
        output[1][i] = M - S;

        /* 步骤4: 累积互相关计算量 */
        /* 使用输出信号计算互相关，反映处理后的立体声特性 */
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
 *
 * 重置统计计数器、计时器累积和互相关值。
 * 调用后 stats() 返回的统计值将全部为零，
 * correlation() 返回 0.0。
 */
void StereoWidth3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_correlation = 0.0;
}
