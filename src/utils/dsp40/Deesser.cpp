/**
 * @file Deesser.cpp
 * @brief 去齿音器 — 唇齿音检测+频段压缩实现
 *
 * 实现去齿音（de-essing）处理：
 * - 二阶IIR带通滤波器提取齿音频段（通常4-10kHz）
 * - 包络跟随器检测齿音电平
 * - 阈值/比率控制的增益压缩
 * - attack/release时间平滑
 * 所有运算带有QElapsedTimer计时和统计信息追踪。
 */

#include "dsp40/Deesser.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
Deesser::Deesser(QObject* parent)
    : QObject(parent)
{
    designBandpass();
}

/**
 * @brief 设计带通滤波器系数
 *
 * 使用双线性变换设计二阶IIR带通滤波器，
 * 中心频率为(lowFreq+highFreq)/2，带宽为highFreq-lowFreq。
 */
void Deesser::designBandpass()
{
    double fc = (m_lowFreq + m_highFreq) / 2.0;
    double bw = m_highFreq - m_lowFreq;
    double fs = m_sampleRate;

    if (fc <= 0.0 || bw <= 0.0 || fs <= 0.0) return;

    double w0 = 2.0 * M_PI * fc / fs;
    double Q = fc / qMax(bw, 1e-10);
    double alpha = qSin(w0) / (2.0 * Q);
    alpha = qBound(1e-10, alpha, 10.0);

    /* 带通滤波器系数（常量Q） */
    double b0 = alpha;
    double b1 = 0.0;
    double b2 = -alpha;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * qCos(w0);
    double a2 = 1.0 - alpha;

    m_bpCoeffs.resize(5);
    m_bpCoeffs[0] = b0 / a0;
    m_bpCoeffs[1] = b1 / a0;
    m_bpCoeffs[2] = b2 / a0;
    m_bpCoeffs[3] = a1 / a0;
    m_bpCoeffs[4] = a2 / a0;

    m_bpState.resize(4, 0.0);
}

/**
 * @brief 带通滤波处理
 * @param input 输入采样点
 * @return 带通滤波后采样点
 */
QVector<double> Deesser::bandpass(const QVector<double>& input)
{
    int n = input.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double x0 = input[i];
        double y0 = m_bpCoeffs[0] * x0
                  + m_bpCoeffs[1] * m_bpState[0]
                  + m_bpCoeffs[2] * m_bpState[1]
                  - m_bpCoeffs[3] * m_bpState[2]
                  - m_bpCoeffs[4] * m_bpState[3];
        output[i] = y0;
        m_bpState[1] = m_bpState[0]; m_bpState[0] = x0;
        m_bpState[3] = m_bpState[2]; m_bpState[2] = y0;
    }
    return output;
}

/**
 * @brief 设置齿音检测频率范围
 * @param lowFreq 低频截止（Hz）
 * @param highFreq 高频截止（Hz）
 */
void Deesser::setFrequencyRange(double lowFreq, double highFreq)
{
    m_lowFreq = qMax(lowFreq, 1000.0);
    m_highFreq = qMin(highFreq, m_sampleRate / 2.0);
    designBandpass();
}

/**
 * @brief 设置检测阈值
 * @param thresholdDb 阈值（dB），超过此电平触发压缩
 */
void Deesser::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置压缩比率
 * @param ratio 压缩比率（>1.0）
 */
void Deesser::setRatio(double ratio)
{
    m_ratio = qMax(ratio, 1.0);
}

/**
 * @brief 设置attack时间
 * @param ms attack时间（毫秒）
 */
void Deesser::setAttack(double ms)
{
    m_attack = qMax(ms, 0.1);
}

/**
 * @brief 设置release时间
 * @param ms release时间（毫秒）
 */
void Deesser::setRelease(double ms)
{
    m_release = qMax(ms, 1.0);
}

/**
 * @brief 处理输入音频帧
 * @param input 输入采样点
 * @return 去齿音后的采样点
 *
 * 处理流程：
 * 1. 带通滤波提取齿音频段
 * 2. 包络跟随检测电平
 * 3. 超过阈值时按比率压缩增益
 * 4. 将增益调制应用到原始信号
 */
QVector<double> Deesser::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);

    /* 计算包络系数 */
    double attackCoeff = qExp(-1.0 / (m_attack * m_sampleRate / 1000.0));
    double releaseCoeff = qExp(-1.0 / (m_sampleRate / 1000.0 * m_release));

    /* 带通滤波提取齿音频段 */
    QVector<double> bpSignal = bandpass(input);

    int sibilantCount = 0;

    for (int i = 0; i < n; ++i) {
        /* 检测齿音电平 */
        double absBp = qAbs(bpSignal[i]);

        /* 包络跟随 */
        double coeff = (absBp > m_envelope) ? attackCoeff : releaseCoeff;
        m_envelope = coeff * m_envelope + (1.0 - coeff) * absBp;

        /* 转为dB */
        double levelDb = 20.0 * qLn(qMax(m_envelope, 1e-10)) / qLn(10.0);
        m_sibilanceLevel = levelDb;

        /* 阈值判断 */
        if (levelDb > m_threshold) {
            m_isSibilant = true;
            sibilantCount++;

            /* 计算压缩增益 */
            double overDb = levelDb - m_threshold;
            double reducedDb = overDb / m_ratio;
            double gainReduction = overDb - reducedDb; /* 减少的dB数 */
            double gainLinear = qPow(10.0, -gainReduction / 20.0);

            /* 对齿音频段应用增益衰减 */
            output[i] = input[i] * gainLinear;
        } else {
            m_isSibilant = false;
            output[i] = input[i];
        }
    }

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessCalls++;
    m_stats.totalSamplesProcessed += n;
    m_stats.totalSibilanceDetected += sibilantCount;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    if (sibilantCount > 0) {
        double avgLevel = m_sibilanceLevel;
        emit sibilanceDetected(avgLevel, m_threshold);
    }

    return output;
}

/**
 * @brief 重置所有统计计数器
 */
void Deesser::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
