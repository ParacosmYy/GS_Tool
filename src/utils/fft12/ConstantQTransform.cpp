/**
 * @file ConstantQTransform.cpp
 * @brief 常数Q变换(CQT)实现 — 音乐信号分析
 */

#include "utils/fft12/ConstantQTransform.h"

#include <QElapsedTimer>

#include <cmath>
#include <algorithm>

/** @brief 构造函数 @param params CQT参数 @param parent 父对象 */
ConstantQTransform::ConstantQTransform(const Parameters& params, QObject* parent)
    : QObject(parent)
    , m_params(params)
{
    buildKernel();
}

/** @brief 计算基频Q值 */
double ConstantQTransform::computeQ() const
{
    return m_params.binsPerOctave / (std::pow(2.0, 1.0 / m_params.binsPerOctave) - 1.0);
}

/** @brief 构建CQT稀疏核矩阵 */
void ConstantQTransform::buildKernel()
{
    QElapsedTimer timer;
    timer.start();

    double Q = computeQ();
    int numOctaves = static_cast<int>(
        std::ceil(std::log2(m_params.maxFreq / m_params.minFreq)));
    m_totalBins = numOctaves * m_params.binsPerOctave;

    m_kernels.resize(m_totalBins);
    m_maxFftLen = 0;

    /* 对每个频率bin构建稀疏核 */
    for (int k = 0; k < m_totalBins; ++k) {
        double freq = m_params.minFreq * std::pow(2.0,
            static_cast<double>(k) / m_params.binsPerOctave);

        /* 该bin对应的FFT长度 */
        int fftLen = static_cast<int>(
            std::ceil(Q * m_params.sampleRate / freq));
        fftLen = std::max(fftLen, 1);
        /* 补零到2的幂次以提高FFT效率 */
        int fftPadded = 1;
        while (fftPadded < fftLen) fftPadded *= 2;

        m_kernels[k].fftLen = fftPadded;
        m_maxFftLen = std::max(m_maxFftLen, fftPadded);

        /* 生成窗函数(Hann)和复指数核 */
        m_kernels[k].indices.clear();
        m_kernels[k].realValues.clear();
        m_kernels[k].imagValues.clear();

        double norm = 0.0;
        for (int n = 0; n < fftLen; ++n) {
            double window = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / (fftLen - 1)));
            norm += window;
        }
        if (norm < 1e-12) norm = 1.0;

        for (int n = 0; n < fftLen; ++n) {
            double window = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / (fftLen - 1)));
            double real = window * std::cos(2.0 * M_PI * Q * n / fftLen) / norm;
            double imag = -window * std::sin(2.0 * M_PI * Q * n / fftLen) / norm;

            /* 稀疏化: 只保留超过阈值的核值 */
            if (std::abs(real) > m_params.threshold ||
                std::abs(imag) > m_params.threshold) {
                m_kernels[k].indices.append(n);
                m_kernels[k].realValues.append(real);
                m_kernels[k].imagValues.append(imag);
            }
        }
    }

    ++m_stats.totalKernelsBuilt;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / std::max(1, m_stats.totalTransformsComputed + m_stats.totalKernelsBuilt);
}

/** @brief 设置CQT参数并重建核 @param params 新参数 */
void ConstantQTransform::setParameters(const Parameters& params)
{
    m_params = params;
    buildKernel();
}

/** @brief 对输入信号执行CQT @param signal 时域输入 @return CQT复数输出(实部/虚部交织) */
QVector<double> ConstantQTransform::compute(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result(m_totalBins * 2, 0.0);

    for (int k = 0; k < m_totalBins; ++k) {
        const auto& kern = m_kernels[k];
        int fftLen = kern.fftLen;

        /* 取一段信号(长度=fftLen，居中) */
        int startIdx = 0;
        if (signal.size() > fftLen) {
            startIdx = (signal.size() - fftLen) / 2;
        }

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

    ++m_stats.totalTransformsComputed;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / std::max(1, m_stats.totalTransformsComputed + m_stats.totalKernelsBuilt);

    emit transformComputed(m_totalBins, signal.size());
    return result;
}

/** @brief 取CQT幅度谱 @param signal 时域输入 @return 幅度值 */
QVector<double> ConstantQTransform::computeMagnitude(const QVector<double>& signal)
{
    auto cqt = compute(signal);
    QVector<double> mag(m_totalBins);
    for (int k = 0; k < m_totalBins; ++k) {
        double re = cqt[k * 2], im = cqt[k * 2 + 1];
        mag[k] = std::sqrt(re * re + im * im);
    }
    return mag;
}

/** @brief 取CQT功率谱 @param signal 时域输入 @return 功率值 */
QVector<double> ConstantQTransform::computePower(const QVector<double>& signal)
{
    auto cqt = compute(signal);
    QVector<double> power(m_totalBins);
    for (int k = 0; k < m_totalBins; ++k) {
        double re = cqt[k * 2], im = cqt[k * 2 + 1];
        power[k] = re * re + im * im;
    }
    return power;
}

/** @brief 获取各bin的中心频率 @return 频率数组(Hz) */
QVector<double> ConstantQTransform::centerFrequencies() const
{
    QVector<double> freqs(m_totalBins);
    for (int k = 0; k < m_totalBins; ++k) {
        freqs[k] = m_params.minFreq * std::pow(2.0,
            static_cast<double>(k) / m_params.binsPerOctave);
    }
    return freqs;
}

/** @brief 重置统计 */
void ConstantQTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
