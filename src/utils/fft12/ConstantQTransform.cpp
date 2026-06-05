/**
 * @file ConstantQTransform.cpp
 * @brief 常数Q变换(CQT)实现 — 对数频率bins的稀疏核+窗口DFT
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现CQT变换核心:
 * 1. 基频Q = binsPerOctave / (2^(1/binsPerOctave) - 1)
 * 2. 每个频率bin对应不同的窗口长度N_k = ceil(Q * fs / f_k)
 * 3. 汉宁窗 + 复指数核 temp[n] = w(n) * exp(-j*2*pi*Q*n/N_k)
 * 4. 稀疏化: 只保留 |temp[n]| > threshold * max 的点
 * 5. 计算时对每个bin用稀疏核做点积
 *
 * 适用于音乐音高检测、音阶分析、音频频谱可视化。
 * 统计计数器方法见同名Stats.cpp。
 */

#include "utils/fft12/ConstantQTransform.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════════
// 构造函数
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 构造CQT引擎并预计算稀疏核矩阵
 * @param params CQT参数(频率范围/bins密度/采样率/稀疏阈值)
 * @param parent 父对象,纳入QObject父子树自动管理生命周期
 *
 * 构造时即调用buildKernel()预计算所有频率bin的稀疏核,
 * 后续compute()调用直接复用缓存核,无需重复构建。
 */
ConstantQTransform::ConstantQTransform(const Parameters& params, QObject* parent)
    : QObject(parent)
    , m_params(params)
{
    buildKernel();
}

// ═══════════════════════════════════════════════════════════════════════════════
// 核构建
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算基频Q值(所有bin共享的常数Q)
 * @return Q = binsPerOctave / (2^(1/binsPerOctave) - 1)
 *
 * Q值保证每个频率bin具有相同的频率分辨率比例。
 * binsPerOctave越大,频率分辨率越精细(如24 bins/octave比12更精细)。
 */
double ConstantQTransform::computeQ() const
{
    return m_params.binsPerOctave / (std::pow(2.0, 1.0 / m_params.binsPerOctave) - 1.0);
}

/**
 * @brief 构建CQT稀疏核矩阵
 *
 * 对每个频率bin k:
 * 1. 中心频率 f_k = f_min * 2^(k/binsPerOctave)
 * 2. 窗口长度 N_k = ceil(Q * fs / f_k),补零到2的幂次
 * 3. 汉宁窗 w(n) = 0.5*(1 - cos(2*pi*n/(N_k-1)))
 * 4. 复核 real(n) = w(n)*cos(2*pi*Q*n/N_k) / norm
 *         imag(n) = -w(n)*sin(2*pi*Q*n/N_k) / norm
 * 5. 稀疏化: 只保留 |real|>threshold 或 |imag|>threshold 的点
 *
 * 稀疏核存储格式: indices[], realValues[], imagValues[] 三组平行数组。
 * 大幅减少后续计算量(通常只有20%~40%的核元素超过阈值)。
 */
void ConstantQTransform::buildKernel()
{
    QElapsedTimer timer;
    timer.start();

    double Q = computeQ();

    /* 总bin数 = 倍频程数 * 每倍频程bin数 */
    int numOctaves = static_cast<int>(
        std::ceil(std::log2(m_params.maxFreq / m_params.minFreq)));
    m_totalBins = numOctaves * m_params.binsPerOctave;

    m_kernels.resize(m_totalBins);
    m_maxFftLen = 0;

    /* 对每个频率bin构建稀疏核 */
    for (int k = 0; k < m_totalBins; ++k) {
        double freq = m_params.minFreq * std::pow(2.0,
            static_cast<double>(k) / m_params.binsPerOctave);

        /* 该bin对应的FFT窗口长度: N_k = Q * fs / f_k */
        int fftLen = static_cast<int>(
            std::ceil(Q * m_params.sampleRate / freq));
        fftLen = std::max(fftLen, 1);

        /* 补零到2的幂次以提高FFT效率 */
        int fftPadded = 1;
        while (fftPadded < fftLen) fftPadded *= 2;

        m_kernels[k].fftLen = fftPadded;
        m_maxFftLen = std::max(m_maxFftLen, fftPadded);

        /* 清空该bin的稀疏核存储 */
        m_kernels[k].indices.clear();
        m_kernels[k].realValues.clear();
        m_kernels[k].imagValues.clear();

        /* 计算窗函数归一化因子(窗函数面积) */
        double norm = 0.0;
        for (int n = 0; n < fftLen; ++n) {
            double window = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / (fftLen - 1)));
            norm += window;
        }
        if (norm < 1e-12) {
            norm = 1.0; ///< 防止除零(极短窗口)
        }

        /* 生成汉宁窗加权的复指数核并稀疏化 */
        for (int n = 0; n < fftLen; ++n) {
            double window = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / (fftLen - 1)));
            double real = window * std::cos(2.0 * M_PI * Q * n / fftLen) / norm;
            double imag = -window * std::sin(2.0 * M_PI * Q * n / fftLen) / norm;

            /* 稀疏化: 只保留超过阈值的核值,减少计算量 */
            if (std::abs(real) > m_params.threshold ||
                std::abs(imag) > m_params.threshold) {
                m_kernels[k].indices.append(n);
                m_kernels[k].realValues.append(real);
                m_kernels[k].imagValues.append(imag);
            }
        }
    }

    /* 更新统计 */
    ++m_stats.totalKernelsBuilt;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / std::max(1, m_stats.totalTransformsComputed + m_stats.totalKernelsBuilt);
}

// ═══════════════════════════════════════════════════════════════════════════════
// 参数设置
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 更新CQT参数并重建核矩阵
 * @param params 新的CQT参数
 *
 * 参数变更会触发核矩阵完全重建,计算量较大,
 * 建议在非实时线程中调用以避免阻塞UI。
 */
void ConstantQTransform::setParameters(const Parameters& params)
{
    m_params = params;
    buildKernel();
}

// ═══════════════════════════════════════════════════════════════════════════════
// CQT变换
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 对输入信号执行CQT变换
 * @param signal 时域输入信号
 * @return CQT复数输出(实部/虚部交织: [re0, im0, re1, im1, ...])
 *
 * 对每个频率bin k,用对应的稀疏核做点积:
 *   X[k].re = sum_n ( kernel_real[k][n] * signal[start+n] )
 *   X[k].im = sum_n ( kernel_imag[k][n] * signal[start+n] )
 *
 * 如果信号长于窗口,取信号中心段(居中对齐)。
 * 只在稀疏核非零位置进行乘法,大幅减少计算量。
 */
QVector<double> ConstantQTransform::compute(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) {
        return {};
    }

    QVector<double> result(m_totalBins * 2, 0.0);

    for (int k = 0; k < m_totalBins; ++k) {
        const auto& kern = m_kernels[k];
        int fftLen = kern.fftLen;

        /* 取一段信号(长度=fftLen,居中对齐) */
        int startIdx = 0;
        if (signal.size() > fftLen) {
            startIdx = (signal.size() - fftLen) / 2;
        }

        /* 稀疏核点积: 只在indices指定的非零位置计算 */
        double realSum = 0.0, imagSum = 0.0;
        for (int i = 0; i < kern.indices.size(); ++i) {
            int idx = startIdx + kern.indices[i];
            if (idx >= 0 && idx < signal.size()) {
                double s = signal[idx];
                realSum += s * kern.realValues[i];
                imagSum += s * kern.imagValues[i];
            }
        }

        result[k * 2] = realSum;
        result[k * 2 + 1] = imagSum;
    }

    /* 更新统计 */
    ++m_stats.totalTransformsComputed;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / std::max(1, m_stats.totalTransformsComputed + m_stats.totalKernelsBuilt);

    emit transformComputed(m_totalBins, signal.size());
    return result;
}

/**
 * @brief 计算CQT幅度谱
 * @param signal 时域输入信号
 * @return 各频率bin的幅度值,长度=totalBins
 *
 * 幅度 = sqrt(real^2 + imag^2),用于频谱可视化。
 */
QVector<double> ConstantQTransform::computeMagnitude(const QVector<double>& signal)
{
    QVector<double> cqt = compute(signal);
    if (cqt.isEmpty()) {
        return {};
    }

    QVector<double> mag(m_totalBins, 0.0);
    for (int k = 0; k < m_totalBins; ++k) {
        double re = cqt[k * 2];
        double im = cqt[k * 2 + 1];
        mag[k] = std::sqrt(re * re + im * im);
    }
    return mag;
}

/**
 * @brief 计算CQT功率谱
 * @param signal 时域输入信号
 * @return 各频率bin的功率值,长度=totalBins
 *
 * 功率 = real^2 + imag^2,幅度谱的平方。
 * 适用于能量分析、音高检测后处理。
 */
QVector<double> ConstantQTransform::computePower(const QVector<double>& signal)
{
    QVector<double> cqt = compute(signal);
    if (cqt.isEmpty()) {
        return {};
    }

    QVector<double> power(m_totalBins, 0.0);
    for (int k = 0; k < m_totalBins; ++k) {
        double re = cqt[k * 2];
        double im = cqt[k * 2 + 1];
        power[k] = re * re + im * im;
    }
    return power;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 频率信息
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 获取各频率bin的中心频率
 * @return 频率数组(Hz),长度=totalBins
 *
 * 第k个bin的中心频率 = f_min * 2^(k/binsPerOctave)。
 * 频率按对数等比递增: 低频区间密(分辨率高),高频区间疏(分辨率低)。
 * 这正是CQT区别于FFT的核心特征。
 */
QVector<double> ConstantQTransform::centerFrequencies() const
{
    QVector<double> freqs(m_totalBins, 0.0);
    for (int k = 0; k < m_totalBins; ++k) {
        freqs[k] = m_params.minFreq * std::pow(2.0,
            static_cast<double>(k) / m_params.binsPerOctave);
    }
    return freqs;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 重置所有统计计数器和时间累加器
 */
void ConstantQTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
