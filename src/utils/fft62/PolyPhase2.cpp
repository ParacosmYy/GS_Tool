/**
 * @file PolyPhase2.cpp
 * @brief 多相滤波器组实现 (分析/综合)
 *
 * 实现多相滤波器组:
 * - 分析滤波器组: 将宽带信号分解为多个窄带子带信号
 * - 综合滤波器组: 将子带信号重新合成为宽带信号
 * - 使用原型低通滤波器设计各通道滤波器
 * - 支持过采样因子以减少混叠
 * 广泛用于子带编码、均衡器和音频效果处理。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/fft62/PolyPhase2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化多相滤波器组
 * @param parent 父QObject指针
 */
PolyPhase2::PolyPhase2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置通道数(子带数)
 * @param n 通道数 (默认 8)
 *
 * 通道数决定了频率分辨率，越大分辨率越高
 */
void PolyPhase2::setNumChannels(int n)
{
    m_numChan = qMax(2, n);
}

/**
 * @brief 设置原型滤波器长度
 * @param len 滤波器长度 (默认 64)
 *
 * 较长的滤波器提供更好的频率选择性
 */
void PolyPhase2::setFilterLength(int len)
{
    m_filterLen = qMax(m_numChan, len);
}

/**
 * @brief 设置过采样因子
 * @param os 过采样因子 (默认 1 = 临界采样)
 *
 * os > 1 可以减少混叠，但增加数据量
 */
void PolyPhase2::setOversampling(int os)
{
    m_oversamp = qMax(1, os);
}

/**
 * @brief 分析滤波器组
 *
 * 将输入信号分解为 m_numChan 个子带信号:
 * 1. 设计原型低通滤波器
 * 2. 对每个通道:
 *    a. 将原型滤波器调制到对应的中心频率
 *    b. 对输入信号进行滤波
 *    c. 按抽取因子下采样
 *
 * @param input 输入信号
 * @return 子带信号矩阵 [numChannels][subbandLength]
 */
QVector<QVector<double>> PolyPhase2::analyze(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> bands(m_numChan);

    if (input.isEmpty()) {
        m_stats.totalTransforms++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
            ? m_timeSum / m_stats.totalTransforms : 0.0;
        emit transformCompleted(m_numChan, 0);
        return bands;
    }

    const int N = input.size();

    /* 设计原型滤波器 */
    designPrototype();

    /* 计算抽取因子 */
    int decimFactor = m_numChan / m_oversamp;
    if (decimFactor < 1) decimFactor = 1;

    /* 子带输出长度 */
    int subbandLen = (N + decimFactor - 1) / decimFactor;

    for (int ch = 0; ch < m_numChan; ++ch) {
        bands[ch].resize(subbandLen);

        /* 中心频率 */
        double centerFreq = 2.0 * M_PI * ch / m_numChan;

        for (int n = 0; n < subbandLen; ++n) {
            int inputIdx = n * decimFactor;
            double val = 0.0;

            /* 多相滤波: 对滤波器和输入信号进行调制 */
            for (int k = 0; k < m_filterLen; ++k) {
                int sampleIdx = inputIdx + k;
                if (sampleIdx < N) {
                    /* 复数调制: h_k * e^{-j*2*pi*ch*k/M} */
                    double modulation = qCos(centerFreq * k);
                    val += m_protoFilter[k] * input[sampleIdx] * modulation;
                }
            }

            bands[ch][n] = val;
        }
    }

    /* 更新统计 */
    m_stats.totalTransforms++;
    m_stats.totalChannels += m_numChan;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(m_numChan, N);
    return bands;
}

/**
 * @brief 综合滤波器组
 *
 * 将子带信号重新合成为宽带信号:
 * 1. 对每个通道:
 *    a. 进行上采样(插零)
 *    b. 用调制后的原型滤波器进行滤波
 * 2. 将所有通道的结果相加
 *
 * @param bands 子带信号矩阵 [numChannels][subbandLength]
 * @return 重建的宽带信号
 */
QVector<double> PolyPhase2::synthesize(const QVector<QVector<double>>& bands)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;

    if (bands.size() != m_numChan) {
        m_stats.totalTransforms++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
        emit transformCompleted(m_numChan, 0);
        return output;
    }

    /* 计算抽取因子 */
    int decimFactor = m_numChan / m_oversamp;
    if (decimFactor < 1) decimFactor = 1;

    /* 确定输出长度 */
    int subbandLen = bands[0].size();
    int outputLen = subbandLen * decimFactor + m_filterLen;
    output.resize(outputLen, 0.0);

    /* 确保原型滤波器已设计 */
    if (m_protoFilter.isEmpty()) {
        designPrototype();
    }

    for (int ch = 0; ch < m_numChan; ++ch) {
        double centerFreq = 2.0 * M_PI * ch / m_numChan;

        for (int n = 0; n < subbandLen; ++n) {
            /* 上采样位置 */
            int baseIdx = n * decimFactor;

            for (int k = 0; k < m_filterLen; ++k) {
                int outIdx = baseIdx + k;
                if (outIdx < outputLen) {
                    /* 综合滤波: 调制后相加 */
                    double modulation = qCos(centerFreq * k);
                    output[outIdx] += bands[ch][n] * m_protoFilter[k] * modulation;
                }
            }
        }
    }

    /* 归一化: 补偿通道数 */
    double scale = 1.0 / m_numChan;
    for (int i = 0; i < outputLen; ++i) {
        output[i] *= scale;
    }

    /* 更新统计 */
    m_stats.totalTransforms++;
    m_stats.totalChannels += m_numChan;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(m_numChan, outputLen);
    return output;
}

/**
 * @brief 重置所有统计数据
 */
void PolyPhase2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_protoFilter.clear();
}

/**
 * @brief 设计原型低通滤波器
 *
 * 使用 sinc 函数加窗设计原型滤波器:
 * h[n] = sinc(n - (L-1)/2) * w[n]
 * 其中 w[n] 是汉宁窗，sinc 截止频率 = 1/(2*M)
 *
 * 原型滤波器需要满足重建条件以实现完全重建
 */
void PolyPhase2::designPrototype()
{
    m_protoFilter.resize(m_filterLen);
    double cutoff = 1.0 / (2.0 * m_numChan);

    for (int n = 0; n < m_filterLen; ++n) {
        /* 归一化时间 */
        double t = static_cast<double>(n) - (m_filterLen - 1) / 2.0;

        /* sinc 函数 */
        double sinc;
        if (qAbs(t) < 1e-10) {
            sinc = 1.0;
        } else {
            sinc = qSin(2.0 * M_PI * cutoff * t) / (M_PI * t);
        }

        /* 汉宁窗 */
        double win = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (m_filterLen - 1)));

        m_protoFilter[n] = sinc * win;
    }

    /* 归一化: 使滤波器增益为1 */
    double energy = 0.0;
    for (double h : m_protoFilter) {
        energy += h * h;
    }
    if (energy > 1e-15) {
        double norm = 1.0 / qSqrt(energy);
        for (double& h : m_protoFilter) {
            h *= norm * qSqrt(static_cast<double>(m_numChan));
        }
    }
}
