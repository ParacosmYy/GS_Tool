/**
 * @file DynamicEQ.cpp
 * @brief 动态均衡器 — 频段自适应增益控制实现
 *
 * 实现多频段动态均衡处理，包含：
 * - 二阶IIR带通滤波器组
 * - 包络跟随器（attack/release平滑）
 * - 增益计算（阈值/比率压缩曲线）
 * 所有运算带有QElapsedTimer计时和统计信息追踪。
 */

#include "dsp39/DynamicEQ.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化频段配置
 * @param numBands 频段数量，默认4
 * @param parent 父对象
 */
DynamicEQ::DynamicEQ(int numBands, QObject* parent)
    : QObject(parent)
    , m_numBands(numBands)
{
    m_bands.resize(numBands);
    m_envelopeState.resize(numBands, 0.0);
    m_filterCoeffs.resize(numBands);
    m_initialized = false;
}

/**
 * @brief 设置频段参数
 * @param index 频段索引
 * @param config 频段配置
 */
void DynamicEQ::setBand(int index, const BandConfig& config)
{
    if (index < 0 || index >= m_numBands) return;
    m_bands[index] = config;
    m_initialized = false;
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率（Hz）
 */
void DynamicEQ::setSampleRate(double sampleRate)
{
    m_sampleRate = sampleRate;
    m_initialized = false;
}

/**
 * @brief 初始化IIR带通滤波器系数
 *
 * 为每个频段设计二阶IIR带通滤波器：
 * 中心频率由BandConfig::freq指定，带宽由bandwidth指定。
 * 使用双线性变换从模拟原型得到数字系数。
 */
void DynamicEQ::initFilters()
{
    if (m_initialized) return;

    m_filterCoeffs.resize(m_numBands);
    for (int b = 0; b < m_numBands; ++b) {
        double fc = m_bands[b].freq;
        double bw = m_bands[b].bandwidth;
        double fs = m_sampleRate;

        /* 归一化频率 */
        double w0 = 2.0 * M_PI * fc / fs;
        double alpha = qSin(w0) * qSinh(qLn(2.0) / 2.0 * bw * w0 / qSin(w0));
        alpha = qBound(1e-10, alpha, 1.0);

        /* 带通滤波器系数 */
        QVector<double>& coeff = m_filterCoeffs[b];
        coeff.resize(6); /* b0,b1,b2, a1,a2, 归一化 */

        double b0 = alpha;
        double b1 = 0.0;
        double b2 = -alpha;
        double a0 = 1.0 + alpha;
        double a1 = -2.0 * qCos(w0);
        double a2 = 1.0 - alpha;

        coeff[0] = b0 / a0;
        coeff[1] = b1 / a0;
        coeff[2] = b2 / a0;
        coeff[3] = a1 / a0;
        coeff[4] = a2 / a0;
        coeff[5] = 0.0; /* 滤波器状态 */
    }
    m_initialized = true;
}

/**
 * @brief 对输入信号施加带通滤波
 * @param input 输入采样点
 * @param band 频段索引
 * @return 滤波后信号
 */
QVector<double> DynamicEQ::bandpass(const QVector<double>& input, int band)
{
    int n = input.size();
    QVector<double> output(n, 0.0);
    const QVector<double>& c = m_filterCoeffs[band];

    double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
    for (int i = 0; i < n; ++i) {
        double x0 = input[i];
        double y0 = c[0] * x0 + c[1] * x1 + c[2] * x2 - c[3] * y1 - c[4] * y2;
        output[i] = y0;
        x2 = x1; x1 = x0;
        y2 = y1; y1 = y0;
    }
    return output;
}

/**
 * @brief 根据信号电平和频段配置计算压缩增益
 * @param level 信号电平（dB）
 * @param cfg 频段配置
 * @return 增益值（线性）
 */
double DynamicEQ::computeGain(double level, const BandConfig& cfg)
{
    double threshold = cfg.threshold;
    double ratio = cfg.ratio;
    if (level <= threshold) return 1.0;

    /* 压缩曲线：超过阈值部分按ratio压缩 */
    double overDb = level - threshold;
    double reducedDb = overDb / ratio;
    return qPow(10.0, (threshold + reducedDb - level) / 20.0);
}

/**
 * @brief 处理输入音频帧
 * @param input 输入采样点
 * @return 处理后的采样点
 *
 * 对每个频段：带通滤波 → 包络跟随 → 增益计算 → 应用增益
 */
QVector<double> DynamicEQ::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_initialized) initFilters();

    int n = input.size();
    QVector<double> output = input;
    double peakReduction = 0.0;

    for (int b = 0; b < m_numBands; ++b) {
        /* 带通滤波提取频段信号 */
        QVector<double> bandSignal = bandpass(input, b);

        /* 计算attack/release系数 */
        double attackCoeff = qExp(-1.0 / (m_bands[b].attack * m_sampleRate / 1000.0));
        double releaseCoeff = qExp(-1.0 / (m_bands[b].release * m_sampleRate / 1000.0));

        /* 包络跟随 + 增益计算 */
        for (int i = 0; i < n; ++i) {
            double absVal = qAbs(bandSignal[i]);
            /* 平滑包络 */
            double coeff = (absVal > m_envelopeState[b]) ? attackCoeff : releaseCoeff;
            m_envelopeState[b] = coeff * m_envelopeState[b] + (1.0 - coeff) * absVal;

            /* 转为dB并计算增益 */
            double levelDb = 20.0 * qLn(qMax(m_envelopeState[b], 1e-10)) / qLn(10.0);
            double gain = computeGain(levelDb, m_bands[b]);

            /* 应用增益到输出 */
            output[i] += bandSignal[i] * (gain - 1.0);
            peakReduction = qMax(peakReduction, qAbs(gain - 1.0));
        }
    }

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessCalls++;
    m_stats.totalSamplesProcessed += n;
    m_stats.activeBands = m_numBands;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    emit processingCompleted(n, peakReduction);
    return output;
}

/**
 * @brief 获取当前各频段增益值
 * @return 各频段增益向量
 */
QVector<double> DynamicEQ::bandGains() const
{
    QVector<double> gains(m_numBands, 1.0);
    for (int b = 0; b < m_numBands; ++b) {
        double levelDb = 20.0 * qLn(qMax(m_envelopeState[b], 1e-10)) / qLn(10.0);
        gains[b] = const_cast<DynamicEQ*>(this)->computeGain(levelDb, m_bands[b]);
    }
    return gains;
}

/**
 * @brief 重置所有统计计数器
 */
void DynamicEQ::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
