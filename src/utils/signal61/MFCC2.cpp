/**
 * @file MFCC2.cpp
 * @brief MFCC (Mel频率倒谱系数) 特征提取实现
 *
 * 实现完整的MFCC特征提取流水线:
 * 1. 对输入帧进行FFT变换
 * 2. 计算功率谱
 * 3. 应用Mel滤波器组
 * 4. 取对数能量
 * 5. DCT-II变换得到倒谱系数
 * 支持可配置的采样率、FFT大小、系数数量和滤波器数量。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/signal61/MFCC2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化MFCC提取器
 * @param parent 父QObject指针
 */
MFCC2::MFCC2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率 (Hz)，默认 44100.0
 */
void MFCC2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置FFT大小
 * @param n FFT点数 (必须为2的幂)，默认 512
 */
void MFCC2::setFFTSize(int n)
{
    m_fftSize = qMax(4, n);
}

/**
 * @brief 设置MFCC系数数量
 * @param n 输出MFCC系数的个数，默认 13
 */
void MFCC2::setNumCoeffs(int n)
{
    m_numCoeffs = qMax(1, n);
}

/**
 * @brief 设置Mel滤波器数量
 * @param n Mel滤波器组中的滤波器个数，默认 26
 */
void MFCC2::setNumFilters(int n)
{
    m_numFilters = qMax(2, n);
}

/**
 * @brief 计算单帧信号的MFCC特征
 *
 * 处理流程:
 * 1. 对输入帧进行零填充到FFT大小
 * 2. 计算FFT并得到功率谱
 * 3. 应用Mel三角滤波器组
 * 4. 取对数能量
 * 5. 进行DCT-II变换
 * 6. 返回前 m_numCoeffs 个系数
 *
 * @param frame 输入音频帧
 * @return MFCC系数向量 (长度为 m_numCoeffs)
 */
QVector<double> MFCC2::compute(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> mfcc(m_numCoeffs, 0.0);

    if (frame.isEmpty()) {
        m_stats.totalComputations++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
        emit computed(0, 0.0);
        return mfcc;
    }

    const int N = m_fftSize;

    /* 步骤1: 零填充 + 计算DFT功率谱 */
    QVector<double> powerSpectrum(N / 2 + 1, 0.0);

    for (int k = 0; k <= N / 2; ++k) {
        double re = 0.0;
        double im = 0.0;
        for (int n = 0; n < qMin(frame.size(), N); ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im += frame[n] * qSin(angle);
        }
        powerSpectrum[k] = (re * re + im * im) / N;
    }

    /* 步骤2: 应用Mel滤波器组 */
    m_melSpec = melFilterBank(powerSpectrum);

    /* 步骤3: 取对数 */
    double totalEnergy = 0.0;
    for (int i = 0; i < m_melSpec.size(); ++i) {
        m_melSpec[i] = qLn(qMax(m_melSpec[i], 1e-10));
        totalEnergy += m_melSpec[i];
    }

    /* 步骤4: DCT-II变换 */
    QVector<double> dctResult = dctII(m_melSpec);

    /* 取前 m_numCoeffs 个系数 */
    for (int i = 0; i < m_numCoeffs && i < dctResult.size(); ++i) {
        mfcc[i] = dctResult[i];
    }

    /* 更新统计 */
    m_stats.totalComputations++;
    m_stats.totalFrames++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    double energy = (totalEnergy / m_melSpec.size());
    emit computed(m_numCoeffs, energy);

    return mfcc;
}

/**
 * @brief 重置所有统计数据
 */
void MFCC2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_melSpec.clear();
}

/**
 * @brief 将Hz频率转换为Mel频率
 *
 * Mel(f) = 2595 * log10(1 + f/700)
 *
 * @param hz 输入频率 (Hz)
 * @return Mel频率值
 */
double MFCC2::hzToMel(double hz) const
{
    return 2595.0 * qLn(1.0 + hz / 700.0) / qLn(10.0);
}

/**
 * @brief 将Mel频率转换为Hz频率
 *
 * Hz(m) = 700 * (10^(m/2595) - 1)
 *
 * @param mel 输入Mel频率
 * @return Hz频率值
 */
double MFCC2::melToHz(double mel) const
{
    return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0);
}

/**
 * @brief 构建并应用Mel三角滤波器组
 *
 * 创建 m_numFilters 个三角滤波器:
 * - 滤波器中心频率在Mel域均匀分布
 * - 每个滤波器从左中心到中心线性上升，从中心到右中心线性下降
 * - 相邻滤波器在中心频率处有50%重叠
 *
 * @param spectrum 输入功率谱 (FFT正频率部分)
 * @return Mel滤波器组输出 (每个滤波器的加权能量和)
 */
QVector<double> MFCC2::melFilterBank(const QVector<double>& spectrum)
{
    const int numBins = spectrum.size();
    QVector<double> melOutput(m_numFilters, 0.0);

    /* 计算Mel频率范围 */
    double lowMel = hzToMel(0.0);
    double highMel = hzToMel(m_sampleRate / 2.0);

    /* 在Mel域均匀分布中心频率 */
    QVector<double> melPoints(m_numFilters + 2);
    for (int i = 0; i < m_numFilters + 2; ++i) {
        melPoints[i] = lowMel + (highMel - lowMel) * i / (m_numFilters + 1);
    }

    /* 转回Hz并映射到FFT bin索引 */
    QVector<int> binPoints(m_numFilters + 2);
    for (int i = 0; i < m_numFilters + 2; ++i) {
        double hz = melToHz(melPoints[i]);
        binPoints[i] = qBound(0, static_cast<int>(qFloor((m_fftSize + 1) * hz / m_sampleRate)), numBins - 1);
    }

    /* 应用三角滤波器 */
    for (int m = 0; m < m_numFilters; ++m) {
        int left = binPoints[m];
        int center = binPoints[m + 1];
        int right = binPoints[m + 2];

        double sum = 0.0;

        /* 上升部分 */
        for (int k = left; k <= center && k < numBins; ++k) {
            if (center > left) {
                sum += spectrum[k] * (k - left) / static_cast<double>(center - left);
            }
        }

        /* 下降部分 */
        for (int k = center; k <= right && k < numBins; ++k) {
            if (right > center) {
                sum += spectrum[k] * (right - k) / static_cast<double>(right - center);
            }
        }

        melOutput[m] = sum;
    }

    return melOutput;
}

/**
 * @brief 计算DCT-II变换
 *
 * DCT-II公式: X[k] = sum_n x[n] * cos(pi/N * (n + 0.5) * k)
 *
 * @param input 输入向量
 * @return DCT-II变换结果
 */
QVector<double> MFCC2::dctII(const QVector<double>& input)
{
    const int N = input.size();
    QVector<double> output(N, 0.0);

    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n) {
            sum += input[n] * qCos(M_PI * (n + 0.5) * k / N);
        }
        output[k] = sum;
    }

    return output;
}
