/**
 * @file PolyPhase3.cpp
 * @brief 多相FFT滤波器组实现 — 分析/综合滤波器组
 *
 * 多相FFT滤波器组(Polyphase DFT Filterbank)使用多相分解实现高效的
 * 均匀DFT滤波器组。通过将原型低通滤波器分解为多个多相分支，
 * 结合DFT调制，大幅降低计算复杂度。支持分析(analysis)和综合(synthesis)
 * 双向操作，适用于频分复用、子带编码、音频处理和频谱感知等场景。
 *
 * 分析滤波器组: 时域宽带信号 → 多个频域窄带子带
 * 综合滤波器组: 多个频域窄带子带 → 时域宽带信号
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 */

#include "utils/fft81/PolyPhase3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <complex>

/**
 * @brief 构造函数，初始化默认滤波器组参数
 * @param parent 父QObject指针
 *
 * 默认64通道，滤波器长度自动设为4*channels。
 */
PolyPhase3::PolyPhase3(QObject* parent)
    : QObject(parent)
    , m_channels(64)
    , m_filterLength(0)
{
}

/**
 * @brief 配置滤波器组参数
 * @param channels 通道数(子带数，决定频率分辨率)
 * @param filterLength 原型滤波器长度(0表示自动: 4*channels)
 * @return 配置是否成功
 *
 * 通道数必须 >= 2。较长的滤波器提供更好的频率选择性
 * 和阻带衰减，但增加计算量和延迟。
 */
bool PolyPhase3::configure(int channels, int filterLength)
{
    if (channels < 2) {
        return false;
    }
    m_channels = channels;
    m_filterLength = (filterLength > 0) ? filterLength : (4 * channels);

    /* 设计原型低通滤波器 */
    designPrototypeFilter();

    return true;
}

/**
 * @brief 分析滤波器组 — 时域信号分解为频域子带
 * @param input 输入时域实数信号
 * @return 子带矩阵 [channels][subbandSamples]
 *
 * 多相分析步骤:
 * 1. 将输入数据按通道数分块
 * 2. 对每个块: 多相滤波器分支与输入卷积
 * 3. 对每个时间块的M个分支执行DFT调制
 * 4. 输出各通道的子带复数样本
 *
 * 数学表达: X_k[m] = sum_{p=0}^{M-1} (sum_n x[nM+p] * h_p[n]) * e^{-j*2*pi*k*p/M}
 */
QVector<QVector<std::complex<double>>> PolyPhase3::analysis(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<std::complex<double>>> subbands(m_channels);

    if (input.isEmpty() || m_prototype.isEmpty()) {
        m_stats.totalFilterbanks++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalFilterbanks > 0)
            ? m_timeSum / m_stats.totalFilterbanks : 0.0;
        emit filterbankProcessed(m_channels, 0);
        return subbands;
    }

    const int N = input.size();
    const int M = m_channels;
    const int L = m_filterLength;
    const int polyPhaseLen = L / M;

    /* 计算子带样本数 */
    int numBlocks = N / M;
    for (int ch = 0; ch < M; ++ch) {
        subbands[ch].resize(numBlocks, std::complex<double>(0.0, 0.0));
    }

    /* 逐块处理 */
    for (int block = 0; block < numBlocks; ++block) {
        /* 多相滤波: 对每个多相分支计算卷积累加 */
        QVector<std::complex<double>> dftInput(M, std::complex<double>(0.0, 0.0));

        for (int p = 0; p < M; ++p) {
            double sum = 0.0;
            /* 多相分支: h_p[n] = h[n*M + p]，与输入卷积 */
            for (int r = 0; r < polyPhaseLen; ++r) {
                int filterIdx = r * M + p;
                int inputBlock = block - r;
                if (filterIdx < L && inputBlock >= 0) {
                    int inputIdx = inputBlock * M + p;
                    if (inputIdx < N) {
                        sum += m_prototype[filterIdx] * input[inputIdx];
                    }
                }
            }
            dftInput[p] = sum;
        }

        /* DFT调制: 将多相分支输出转换到频域各通道 */
        for (int k = 0; k < M; ++k) {
            std::complex<double> Xk(0.0, 0.0);
            for (int p = 0; p < M; ++p) {
                double angle = -2.0 * M_PI * k * p / M;
                Xk += dftInput[p] * std::complex<double>(qCos(angle), qSin(angle));
            }
            subbands[k][block] = Xk;
        }
    }

    /* 更新统计信息 */
    m_stats.totalFilterbanks++;
    m_stats.totalSubbands += numBlocks * M;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFilterbanks;

    emit filterbankProcessed(M, numBlocks);
    return subbands;
}

/**
 * @brief 综合滤波器组 — 频域子带重建为时域信号
 * @param subbands 子带矩阵 [channels][subbandSamples]
 * @return 重建的时域实数信号
 *
 * 综合步骤(分析逆过程):
 * 1. 对每个时间块: IDFT将频域子带转回多相分支
 * 2. 多相分支上采样并滤波
 * 3. 所有分支叠加求和得到重建信号
 *
 * 数学表达: y[n] = sum_k sum_m X_k[m] * g[n-mM] * e^{j*2*pi*k*n/M}
 */
QVector<double> PolyPhase3::synthesis(const QVector<QVector<std::complex<double>>>& subbands)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;

    if (subbands.size() != m_channels || subbands.isEmpty() || m_prototype.isEmpty()) {
        m_stats.totalFilterbanks++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalFilterbanks > 0)
            ? m_timeSum / m_stats.totalFilterbanks : 0.0;
        emit filterbankProcessed(m_channels, 0);
        return output;
    }

    const int M = m_channels;
    const int L = m_filterLength;
    const int polyPhaseLen = L / M;
    const int numBlocks = subbands[0].size();

    /* 输出缓冲区 */
    int outputLen = numBlocks * M + L;
    output.resize(outputLen, 0.0);

    /* 逐块重建 */
    for (int block = 0; block < numBlocks; ++block) {
        /* IDFT: 频域子带 → 多相分支 */
        QVector<double> idftOutput(M, 0.0);
        for (int p = 0; p < M; ++p) {
            double sum = 0.0;
            for (int k = 0; k < M; ++k) {
                double angle = 2.0 * M_PI * k * p / M;
                double re = subbands[k][block].real() * qCos(angle)
                          - subbands[k][block].imag() * qSin(angle);
                sum += re;
            }
            idftOutput[p] = sum / M;
        }

        /* 多相综合: 分支滤波输出叠加到时域 */
        for (int r = 0; r < polyPhaseLen; ++r) {
            for (int p = 0; p < M; ++p) {
                int filterIdx = r * M + p;
                int outIdx = (block + r) * M + p;
                if (filterIdx < L && outIdx < outputLen) {
                    output[outIdx] += idftOutput[p] * m_prototype[filterIdx];
                }
            }
        }
    }

    /* 归一化: 补偿通道增益 */
    double scale = 1.0 / M;
    for (int i = 0; i < outputLen; ++i) {
        output[i] *= scale;
    }

    /* 更新统计信息 */
    m_stats.totalFilterbanks++;
    m_stats.totalSubbands += numBlocks * M;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFilterbanks;

    emit filterbankProcessed(M, outputLen);
    return output;
}

/**
 * @brief 重置所有累计统计信息
 */
void PolyPhase3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 设计原型低通滤波器
 *
 * 使用sinc函数加汉宁窗设计原型滤波器:
 * h[n] = sinc(2*fc*(n-(L-1)/2)) * w[n]
 * 其中 fc = 1/(2*M) (归一化截止频率)
 * w[n] 为汉宁窗
 *
 * 原型滤波器是滤波器组的核心，需要满足:
 * 1. 通带平坦(无幅度失真)
 * 2. 阻带高衰减(减少混叠)
 * 3. 长度为通道数的整数倍(多相分解要求)
 */
void PolyPhase3::designPrototypeFilter()
{
    /* 确保滤波器长度为通道数的整数倍 */
    int adjustedLen = m_filterLength;
    if (adjustedLen % m_channels != 0) {
        adjustedLen = ((adjustedLen / m_channels) + 1) * m_channels;
    }
    m_filterLength = adjustedLen;
    m_prototype.resize(m_filterLength);

    double cutoff = 1.0 / (2.0 * m_channels);
    double mid = (m_filterLength - 1) / 2.0;

    for (int n = 0; n < m_filterLength; ++n) {
        double t = static_cast<double>(n) - mid;

        /* sinc函数 */
        double sincVal;
        if (qAbs(t) < 1e-10) {
            sincVal = 1.0;
        } else {
            sincVal = qSin(2.0 * M_PI * cutoff * t) / (M_PI * t);
        }

        /* 汉宁窗 */
        double win = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (m_filterLength - 1)));

        m_prototype[n] = sincVal * win;
    }

    /* 归一化: 使通带增益为1 */
    double energy = 0.0;
    for (int i = 0; i < m_filterLength; ++i) {
        energy += m_prototype[i] * m_prototype[i];
    }
    if (energy > 1e-15) {
        double norm = 1.0 / qSqrt(energy);
        for (int i = 0; i < m_filterLength; ++i) {
            m_prototype[i] *= norm * qSqrt(static_cast<double>(m_channels));
        }
    }
}
