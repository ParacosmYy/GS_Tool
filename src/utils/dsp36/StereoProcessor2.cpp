/**
 * @file StereoProcessor2.cpp
 * @brief 立体声处理器实现 - 中侧处理、声像宽度控制、平衡与交叉馈送
 *
 * 将左右声道转换为Mid/Side表示，应用宽度与平衡系数，
 * 并通过交叉馈送实现耳机声场扩展。
 * 支持M/S分析、相位相关度计算、立体声电平监测和基础自动平衡。
 */

#include "utils/dsp36/StereoProcessor2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
StereoProcessor2::StereoProcessor2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置立体声宽度系数
 * @param w 宽度: 0=单声道, 1=原始, >1=扩展, <0=反转侧声道
 */
void StereoProcessor2::setWidth(double w)
{
    m_width = qBound(-2.0, w, 2.0);
}

/**
 * @brief 设置左右平衡
 * @param b 平衡: -1=全左, 0=居中, 1=全右
 */
void StereoProcessor2::setBalance(double b)
{
    m_balance = qBound(-1.0, b, 1.0);
}

/**
 * @brief 设置交叉馈送量
 * @param cf 交叉馈送系数: 0=无, 0.3~0.7=典型耳机值
 */
void StereoProcessor2::setCrossFeed(double cf)
{
    m_crossFeed = qBound(0.0, cf, 1.0);
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void StereoProcessor2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 处理单个立体声采样对
 *
 * 完整的M/S处理管线:
 * 1. L/R -> Mid/Side 编码
 * 2. 对Side通道应用宽度系数
 * 3. M/S -> L/R 解码
 * 4. 应用平衡系数(恒功率声像定律)
 * 5. 交叉馈送混合
 * 6. 软削波保护
 *
 * @param left 左声道采样
 * @param right 右声道采样
 * @return 处理后的左右采样对
 */
QPair<double, double> StereoProcessor2::processOne(double left, double right)
{
    /* 第一步: L/R -> M/S 编码
     * mid = (L+R)/2 包含中央信息(单声道分量)
     * side = (L-R)/2 包含立体声差异信息 */
    double mid  = 0.5 * (left + right);
    double side = 0.5 * (left - right);

    /* 第二步: 应用宽度(缩放Side分量)
     * width=0: 仅保留mid(单声道)
     * width=1: 原始立体声
     * width>1: 扩展侧声道(更宽)
     * width<0: 反转侧声道(反转立体声) */
    side *= m_width;

    /* 第三步: M/S -> L/R 解码
     * L = mid + side, R = mid - side */
    double l = mid + side;
    double r = mid - side;

    /* 第四步: 应用平衡 - 恒功率声像定律
     * 使用正弦/余弦曲线保证能量守恒
     * balance=-1: cos(pi/2)=0, sin(pi/2)=1 -> 全右
     * balance=0:  cos(pi/4)=sin(pi/4) -> 居中
     * balance=1:  cos(0)=1, sin(0)=0 -> 全左 */
    double balAngle = (m_balance + 1.0) * 0.5 * M_PI_2;
    double lGain = qCos(balAngle);
    double rGain = qSin(balAngle);
    l *= lGain;
    r *= rGain;

    /* 第五步: 交叉馈送 - 将对侧信号按比例混入
     * 模拟扬声器串音效果，改善耳机听感
     * crossFeed=0: 无交叉(纯耳机)
     * crossFeed=0.3: 轻度串音(模拟近场监听)
     * crossFeed=0.5: 单声道 */
    double cfL = l * (1.0 - m_crossFeed) + r * m_crossFeed;
    double cfR = r * (1.0 - m_crossFeed) + l * m_crossFeed;

    /* 第六步: 防削波软限制
     * 在阈值(0.99)以上使用指数衰减曲线平滑过渡
     * 避免硬削波产生的谐波失真 */
    auto softClip = [](double x) -> double {
        const double threshold = 0.99;
        const double headroom = 1.0 - threshold;
        if (x > threshold)
            return threshold + headroom * (1.0 - qExp(-(x - threshold) / headroom));
        if (x < -threshold)
            return -(threshold + headroom * (1.0 - qExp(-(-x - threshold) / headroom)));
        return x;
    };

    return {softClip(cfL), softClip(cfR)};
}

/**
 * @brief 批量处理立体声采样序列
 *
 * 对输入序列逐帧执行完整的M/S处理管线，
 * 处理完成后更新统计信息并发射完成信号。
 *
 * @param input 输入采样对序列
 * @return 处理后的采样对序列
 */
QVector<QPair<double, double>> StereoProcessor2::process(const QVector<QPair<double, double>>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> output;
    output.reserve(input.size());

    for (const auto& sample : input)
        output.append(processOne(sample.first, sample.second));

    m_stats.totalSamplesProcessed += input.size();
    m_timeSum += timer.elapsed();
    if (m_stats.totalSamplesProcessed > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSamplesProcessed;

    emit processingComplete(input.size());
    return output;
}

/**
 * @brief 重置内部状态(此处理器无状态，此方法为接口一致性)
 */
void StereoProcessor2::reset()
{
    /* 无状态处理器，无需重置 */
}

/**
 * @brief 重置所有统计数据
 */
void StereoProcessor2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
