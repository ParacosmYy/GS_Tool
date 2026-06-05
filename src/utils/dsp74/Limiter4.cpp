/**
 * @file Limiter4.cpp
 * @brief 噪声门效果器实现
 *
 * 实现可配置阈值的噪声门处理器，支持攻击、释放、保持
 * 时间参数和范围控制。用于音频信号中噪声的动态抑制。
 */

#include "utils/dsp74/Limiter4.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
Limiter4::Limiter4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置门限阈值
 * @param thresh 阈值(dB)，低于此值的信号被衰减
 */
void Limiter4::setThreshold(double thresh)
{
    m_threshold = qBound(-100.0, thresh, 0.0);
}

/**
 * @brief 设置攻击时间
 * @param ms 攻击时间(ms)，门打开的速度
 */
void Limiter4::setAttack(double ms)
{
    m_attack = qBound(0.01, ms, 100.0);
}

/**
 * @brief 设置释放时间
 * @param ms 释放时间(ms)，门关闭的速度
 */
void Limiter4::setRelease(double ms)
{
    m_release = qBound(1.0, ms, 5000.0);
}

/**
 * @brief 设置保持时间
 * @param ms 保持时间(ms)，信号低于阈值后保持开启的时间
 */
void Limiter4::setHold(double ms)
{
    m_hold = qBound(0.0, ms, 1000.0);
}

/**
 * @brief 设置衰减范围
 * @param db 衰减范围(dB)，门关闭时的最大衰减量
 */
void Limiter4::setRange(double db)
{
    m_range = qBound(-100.0, db, 0.0);
}

/**
 * @brief 处理音频信号
 * @param input 输入音频采样
 * @return 处理后的音频采样
 *
 * 通过电平检测、包络跟随和增益控制实现噪声门功能。
 * 使用RMS检测器计算信号电平，与阈值比较后控制增益。
 * 增益过渡使用攻击/释放系数平滑，避免咔嗒噪声。
 */
QVector<double> Limiter4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return QVector<double>();

    const int N = input.size();
    QVector<double> output(N, 0.0);

    /* 时间常数转换为平滑系数 */
    double sr = 44100.0; /* 默认采样率 */
    double attackCoeff = qExp(-1.0 / (sr * m_attack / 1000.0));
    double releaseCoeff = qExp(-1.0 / (sr * m_release / 1000.0));
    int holdSamples = qRound(sr * m_hold / 1000.0);

    /* 将dB阈值和范围转换为线性值 */
    double thresholdLin = qPow(10.0, m_threshold / 20.0);
    double rangeLin = qPow(10.0, m_range / 20.0);

    double envelope = 0.0;  ///< 信号包络电平
    double gain = 0.0;      ///< 当前增益值
    int holdCounter = 0;    ///< 保持计数器
    m_open = false;

    for (int i = 0; i < N; ++i) {
        /* 步骤1: 计算绝对值作为瞬时电平 */
        double absVal = qAbs(input[i]);

        /* 步骤2: 包络跟随器 - 峰值检测模式 */
        if (absVal > envelope) {
            /* 信号上升: 使用攻击系数平滑 */
            envelope = attackCoeff * envelope + (1.0 - attackCoeff) * absVal;
        } else {
            /* 信号下降: 使用释放系数平滑 */
            envelope = releaseCoeff * envelope + (1.0 - releaseCoeff) * absVal;
        }

        /* 步骤3: 状态判定和增益计算 */
        if (envelope >= thresholdLin) {
            /* 信号超过阈值: 门打开 */
            m_open = true;
            holdCounter = holdSamples;
            /* 快速恢复到单位增益 */
            gain = attackCoeff * gain + (1.0 - attackCoeff) * 1.0;
        } else if (holdCounter > 0) {
            /* 保持阶段: 信号已低于阈值但在保持时间内 */
            holdCounter--;
            /* 维持当前增益不变 */
            gain = attackCoeff * gain + (1.0 - attackCoeff) * 1.0;
        } else {
            /* 释放阶段: 门关闭，增益衰减到范围值 */
            m_open = false;
            gain = releaseCoeff * gain + (1.0 - releaseCoeff) * rangeLin;
        }

        /* 步骤4: 限制增益在有效范围内 */
        gain = qBound(rangeLin, gain, 1.0);

        /* 步骤5: 应用增益到输出 */
        output[i] = input[i] * gain;
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalProcessings++;
    m_stats.totalSamples += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(N, m_open);
    return output;
}

/**
 * @brief 重置统计信息
 */
void Limiter4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算信号的RMS电平(dB)
 * @param signal 输入信号
 * @return RMS电平(dB)，静音返回-120dB
 */
double Limiter4::computeRMSLevel(const QVector<double>& signal) const
{
    if (signal.isEmpty()) return -120.0;
    double sumSq = 0.0;
    for (double s : signal) sumSq += s * s;
    double rms = qSqrt(sumSq / signal.size());
    if (rms < 1e-10) return -120.0;
    return 20.0 * qLn(rms) / qLn(10.0);
}

/**
 * @brief 计算信号的峰值电平(dB)
 * @param signal 输入信号
 * @return 峰值电平(dB)，静音返回-120dB
 */
double Limiter4::computePeakLevel(const QVector<double>& signal) const
{
    if (signal.isEmpty()) return -120.0;
    double peak = 0.0;
    for (double s : signal) peak = qMax(peak, qAbs(s));
    if (peak < 1e-10) return -120.0;
    return 20.0 * qLn(peak) / qLn(10.0);
}

/**
 * @brief 计算信号中被门控（衰减）的采样点比例
 * @param input 输入信号
 * @return 门控比例(0~1)，0=无门控，1=全部静音
 */
double Limiter4::gateRatio(const QVector<double>& input) const
{
    if (input.isEmpty()) return 0.0;

    double thresholdLin = qPow(10.0, m_threshold / 20.0);
    int gatedCount = 0;

    for (double s : input) {
        if (qAbs(s) < thresholdLin) gatedCount++;
    }

    return static_cast<double>(gatedCount) / input.size();
}
