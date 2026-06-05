/**
 * @file StereoProcessor3.cpp
 * @brief 立体声处理器3实现 — MS编解码+声场旋转
 *
 * 立体声信号处理器，支持Mid/Side编解码、立体声宽度调节、
 * 声场旋转、左右平衡控制和低频单声道化。
 */

#include "utils/dsp46/StereoProcessor3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
StereoProcessor3::StereoProcessor3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置立体声宽度
 * @param width 宽度系数[0,2]，0=单声道，1=原始宽度，>1=扩展
 */
void StereoProcessor3::setWidth(double width)
{
    m_width = qBound(0.0, width, 3.0);
}

/**
 * @brief 设置声场旋转角度
 * @param degrees 旋转角度(度)，正值向右旋转，负值向左
 */
void StereoProcessor3::setRotation(double degrees)
{
    m_rotation = qBound(-45.0, degrees, 45.0);
}

/**
 * @brief 设置左右平衡
 * @param balance 平衡值[-1,1]，-1=全左，0=居中，1=全右
 */
void StereoProcessor3::setBalance(double balance)
{
    m_balance = qBound(-1.0, balance, 1.0);
}

/**
 * @brief 设置低频单声道化截止频率
 * @param freqHz 截止频率(Hz)，低于此频率的信号合并为单声道
 */
void StereoProcessor3::setMonoBelow(double freqHz)
{
    m_monoBelow = qMax(0.0, freqHz);
}

/**
 * @brief 处理立体声信号
 * @param left 左声道输入
 * @param right 右声道输入
 * @return 处理后的左右声道对
 *
 * 处理链: LR -> MS -> 宽度调节 -> 旋转 -> 平衡 -> 低频单声道化 -> LR
 */
QPair<QVector<double>, QVector<double>> StereoProcessor3::process(
    const QVector<double>& left, const QVector<double>& right)
{
    QElapsedTimer timer;
    timer.start();

    const int n = qMin(left.size(), right.size());
    if (n == 0) {
        return {QVector<double>(), QVector<double>()};
    }

    QVector<double> outL(n, 0.0);
    QVector<double> outR(n, 0.0);

    /* ---- 第一步: 转换为Mid/Side ---- */
    auto ms = toMS(left, right);
    QVector<double> mid = ms.first;
    QVector<double> side = ms.second;

    /* ---- 第二步: 宽度调节 ---- */
    /* width=0: 只保留Mid(单声道), width=2: Side加倍 */
    double midGain = 1.0;
    double sideGain = m_width;
    for (int i = 0; i < n; ++i) {
        mid[i] *= midGain;
        side[i] *= sideGain;
    }

    /* ---- 第三步: 声场旋转 ---- */
    if (qFabs(m_rotation) > 0.01) {
        double rotRad = m_rotation * M_PI / 180.0;
        double cosR = qCos(rotRad);
        double sinR = qSin(rotRad);

        for (int i = 0; i < n; ++i) {
            double newMid = mid[i] * cosR - side[i] * sinR;
            double newSide = mid[i] * sinR + side[i] * cosR;
            mid[i] = newMid;
            side[i] = newSide;
        }
    }

    /* ---- 第四步: 转回LR ---- */
    auto lr = toLR(mid, side);

    /* ---- 第五步: 平衡控制 ---- */
    if (qFabs(m_balance) > 0.001) {
        double leftGain = qSqrt(qMax(0.0, 1.0 - m_balance));
        double rightGain = qSqrt(qMin(1.0, 1.0 + m_balance));

        for (int i = 0; i < n; ++i) {
            lr.first[i] *= leftGain;
            lr.second[i] *= rightGain;
        }
    }

    /* ---- 第六步: 低频单声道化 ---- */
    if (m_monoBelow > 0.0) {
        QVector<double> monoLow = lowpass(lr.first);
        QVector<double> monoLowR = lowpass(lr.second);

        for (int i = 0; i < n; ++i) {
            double monoSignal = (monoLow[i] + monoLowR[i]) * 0.5;
            lr.first[i] = lr.first[i] - monoLow[i] + monoSignal;
            lr.second[i] = lr.second[i] - monoLowR[i] + monoSignal;
        }
    }

    outL = lr.first;
    outR = lr.second;

    /* 计算立体声相关系数 */
    double sumLR = 0.0, sumL2 = 0.0, sumR2 = 0.0;
    for (int i = 0; i < n; ++i) {
        sumLR += outL[i] * outR[i];
        sumL2 += outL[i] * outL[i];
        sumR2 += outR[i] * outR[i];
    }
    double denom = qSqrt(sumL2 * sumR2);
    double correlation = (denom > 1e-12) ? sumLR / denom : 0.0;

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessCalls++;
    m_stats.totalSamplesProcessed += n;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    emit processingCompleted(n, correlation);
    return {outL, outR};
}

/**
 * @brief LR转Mid/Side
 * @param left 左声道
 * @param right 右声道
 * @return Mid/Side对
 *
 * Mid = (L + R) / sqrt(2)
 * Side = (L - R) / sqrt(2)
 */
QPair<QVector<double>, QVector<double>> StereoProcessor3::toMS(
    const QVector<double>& left, const QVector<double>& right) const
{
    const int n = qMin(left.size(), right.size());
    QVector<double> mid(n, 0.0);
    QVector<double> side(n, 0.0);

    const double norm = M_SQRT1_2; /* 1/sqrt(2) */
    for (int i = 0; i < n; ++i) {
        mid[i] = (left[i] + right[i]) * norm;
        side[i] = (left[i] - right[i]) * norm;
    }

    return {mid, side};
}

/**
 * @brief Mid/Side转LR
 * @param mid Mid声道
 * @param side Side声道
 * @return 左右声道对
 *
 * L = (M + S) / sqrt(2)
 * R = (M - S) / sqrt(2)
 */
QPair<QVector<double>, QVector<double>> StereoProcessor3::toLR(
    const QVector<double>& mid, const QVector<double>& side) const
{
    const int n = qMin(mid.size(), side.size());
    QVector<double> left(n, 0.0);
    QVector<double> right(n, 0.0);

    const double norm = M_SQRT1_2;
    for (int i = 0; i < n; ++i) {
        left[i] = (mid[i] + side[i]) * norm;
        right[i] = (mid[i] - side[i]) * norm;
    }

    return {left, right};
}

/**
 * @brief 一阶IIR低通滤波器
 * @param input 输入信号
 * @return 滤波后的信号
 *
 * 用于低频单声道化处理，提取低频分量。
 */
QVector<double> StereoProcessor3::lowpass(const QVector<double>& input)
{
    const int n = input.size();
    if (n == 0) return QVector<double>();

    /* 计算IIR系数: 一阶巴特沃斯 */
    double fc = m_monoBelow / m_sampleRate;
    fc = qBound(0.0, fc, 0.5);
    double x = qExp(-2.0 * M_PI * fc);
    double a0 = 1.0 - x;
    double b1 = x;

    QVector<double> output(n, 0.0);
    double state = (m_lpfState.isEmpty()) ? 0.0 : m_lpfState[0];

    for (int i = 0; i < n; ++i) {
        state = a0 * input[i] + b1 * state;
        output[i] = state;
    }

    /* 保存滤波器状态 */
    m_lpfState = {state};

    return output;
}

/**
 * @brief 重置所有统计信息
 */
void StereoProcessor3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
