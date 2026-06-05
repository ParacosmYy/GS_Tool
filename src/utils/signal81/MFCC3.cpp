/**
 * @file MFCC3.cpp
 * @brief 梅尔频率倒谱系数(MFCC)提取器实现
 *
 * 实现完整的MFCC特征提取流水线:
 * 1. 对输入帧进行DFT变换(零填充到FFT大小)
 * 2. 计算功率谱: P(k) = (Re^2 + Im^2) / N
 * 3. 应用梅尔三角滤波器组(将频域映射到Mel域)
 * 4. 取对数能量: log(E_mel + epsilon)
 * 5. DCT-II变换得到倒谱系数
 * 6. 返回前numCoeffs个系数(通常13个)
 *
 * 梅尔频率转换:
 * Mel(f) = 2595 * log10(1 + f/700)
 * 将线性频率轴映射为近似人耳听觉的非线性轴
 *
 * Delta系数计算:
 * 一阶差分: d(t) = sum_{n=1}^{N} n*(c(t+n) - c(t-n)) / (2*sum_{n=1}^{N} n^2)
 * 二阶差分: 对一阶差分再做同样的差分
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 */

#include "utils/signal81/MFCC3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化MFCC提取器
 * @param parent 父QObject指针
 *
 * 默认参数: 采样率16kHz, FFT 512点, 13个系数。
 */
MFCC3::MFCC3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率、帧长和系数数
 *
 * @param sampleRate 采样率(Hz)，影响梅尔滤波器组频率范围
 * @param fftSize FFT点数(建议为2的幂)，决定频率分辨率
 * @param numCoeffs 输出MFCC系数数量(通常13)，不含C0(能量项)
 */
void MFCC3::initialize(double sampleRate, int fftSize, int numCoeffs)
{
    m_sampleRate = qMax(1.0, sampleRate);
    m_fftSize = qMax(4, fftSize);
    m_numCoeffs = qBound(1, numCoeffs, m_fftSize / 2);
}

/**
 * @brief 计算一帧的MFCC系数
 *
 * 完整流水线:
 * 1. 零填充到FFT大小
 * 2. 计算DFT功率谱(仅正频率部分)
 * 3. 应用梅尔三角滤波器组(通常26个滤波器)
 * 4. 取对数能量(加epsilon避免log(0))
 * 5. DCT-II变换
 * 6. 取前numCoeffs个系数
 *
 * @param frame 输入音频帧(时域)
 * @return MFCC系数向量(长度为numCoeffs)
 */
QVector<double> MFCC3::compute(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> mfcc(m_numCoeffs, 0.0);

    if (frame.isEmpty()) {
        m_stats.totalFramesProcessed++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalFramesProcessed > 0)
            ? m_timeSum / m_stats.totalFramesProcessed : 0.0;
        emit coefficientsComputed(0);
        return mfcc;
    }

    const int N = m_fftSize;

    /* 步骤1: 计算DFT功率谱(正频率部分) */
    const int specLen = N / 2 + 1;
    QVector<double> powerSpec(specLen, 0.0);

    const int frameLen = qMin(frame.size(), N);
    for (int k = 0; k < specLen; ++k) {
        double re = 0.0;
        double im = 0.0;
        for (int n = 0; n < frameLen; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im += frame[n] * qSin(angle);
        }
        powerSpec[k] = (re * re + im * im) / N;
    }

    /* 步骤2: 应用梅尔三角滤波器组 */
    const int numFilters = qMax(2, m_numCoeffs * 2);  ///< 滤波器数量 >= 2 * 系数数
    QVector<double> melEnergy = applyMelFilterbank(powerSpec, numFilters);

    /* 步骤3: 取对数能量 */
    for (int i = 0; i < melEnergy.size(); ++i) {
        melEnergy[i] = qLn(qMax(melEnergy[i], 1e-10));
    }

    /* 步骤4: DCT-II变换 */
    QVector<double> dctResult(specLen, 0.0);
    int dctLen = qMin(melEnergy.size(), specLen);
    for (int k = 0; k < m_numCoeffs && k < dctLen; ++k) {
        double sum = 0.0;
        for (int n = 0; n < dctLen; ++n) {
            sum += melEnergy[n] * qCos(M_PI * (n + 0.5) * k / dctLen);
        }
        mfcc[k] = sum;
    }

    /* 更新统计 */
    m_stats.totalFramesProcessed++;
    m_stats.totalCoefficients += m_numCoeffs;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalFramesProcessed > 0)
        ? m_timeSum / m_stats.totalFramesProcessed : 0.0;

    emit coefficientsComputed(m_numCoeffs);
    return mfcc;
}

/**
 * @brief 计算delta和delta-delta系数
 *
 * 对输入帧计算MFCC及其一阶和二阶差分:
 * - 静态系数: 原始MFCC
 * - Delta(一阶差分): 反映系数随时间的变化速度
 * - Delta-delta(二阶差分): 反映变化的加速度
 *
 * Delta公式(回归斜率):
 * d[t] = sum_{n=1}^{N} n * (c[t+n] - c[t-n]) / (2 * sum_{n=1}^{N} n^2)
 *
 * 对于单帧输入，使用帧内各系数之间的差分近似。
 *
 * @param frame 输入音频帧
 * @return 三元素向量: [0]=静态MFCC, [1]=delta, [2]=delta-delta
 */
QVector<QVector<double>> MFCC3::computeWithDeltas(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> result(3);

    /* 静态MFCC系数 */
    result[0] = compute(frame);

    if (result[0].isEmpty()) {
        result[1] = result[0];
        result[2] = result[0];
        return result;
    }

    const int len = result[0].size();

    /* Delta系数: 相邻系数之间的差分 */
    result[1].resize(len, 0.0);
    const int N = 2;  ///< 回归窗口大小
    for (int i = 0; i < len; ++i) {
        double num = 0.0;
        double den = 0.0;
        for (int n = 1; n <= N; ++n) {
            double prev = (i - n >= 0) ? result[0][i - n] : result[0][0];
            double next = (i + n < len) ? result[0][i + n] : result[0][len - 1];
            num += n * (next - prev);
            den += n * n;
        }
        result[1][i] = (den > 1e-15) ? num / (2.0 * den) : 0.0;
    }

    /* Delta-delta系数: delta的差分 */
    result[2].resize(len, 0.0);
    for (int i = 0; i < len; ++i) {
        double num = 0.0;
        double den = 0.0;
        for (int n = 1; n <= N; ++n) {
            double prev = (i - n >= 0) ? result[1][i - n] : result[1][0];
            double next = (i + n < len) ? result[1][i + n] : result[1][len - 1];
            num += n * (next - prev);
            den += n * n;
        }
        result[2][i] = (den > 1e-15) ? num / (2.0 * den) : 0.0;
    }

    /* 统计更新已在compute()中完成 */
    double elapsed = timer.elapsed();
    Q_UNUSED(elapsed);

    return result;
}

/**
 * @brief 获取梅尔滤波器组能量
 *
 * 返回最近一次compute()调用中的梅尔滤波器组输出
 * (取对数之前)，用于可视化或进一步分析。
 *
 * @param frame 输入音频帧
 * @return 梅尔滤波器组能量向量
 */
QVector<double> MFCC3::melEnergies(const QVector<double>& frame) const
{
    if (frame.isEmpty()) {
        return {};
    }

    const int N = m_fftSize;
    const int specLen = N / 2 + 1;
    const int numFilters = qMax(2, m_numCoeffs * 2);

    /* 计算DFT功率谱 */
    QVector<double> powerSpec(specLen, 0.0);
    const int frameLen = qMin(frame.size(), N);
    for (int k = 0; k < specLen; ++k) {
        double re = 0.0;
        double im = 0.0;
        for (int n = 0; n < frameLen; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im += frame[n] * qSin(angle);
        }
        powerSpec[k] = (re * re + im * im) / N;
    }

    return applyMelFilterbank(powerSpec, numFilters);
}

/**
 * @brief 重置所有统计数据
 *
 * 清零帧计数、系数计数和平均处理时间。
 */
void MFCC3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 应用梅尔三角滤波器组(内部辅助)
 *
 * 创建numFilters个三角滤波器:
 * 1. 在Mel域均匀分布中心频率
 * 2. 转回Hz域映射到FFT bin
 * 3. 每个滤波器为三角形: 左中心→中心线性上升，中心→右中心线性下降
 * 4. 相邻滤波器在中心频率处有50%重叠
 *
 * @param powerSpec 功率谱(FFT正频率部分)
 * @param numFilters 梅尔滤波器数量
 * @return 各滤波器的加权能量和
 */
QVector<double> MFCC3::applyMelFilterbank(
    const QVector<double>& powerSpec, int numFilters) const
{
    const int specLen = powerSpec.size();
    QVector<double> melOutput(numFilters, 0.0);

    /* Hz -> Mel 转换 */
    auto hzToMel = [](double hz) -> double {
        return 2595.0 * qLn(1.0 + hz / 700.0) / qLn(10.0);
    };

    /* Mel -> Hz 转换 */
    auto melToHz = [](double mel) -> double {
        return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0);
    };

    /* 计算Mel频率范围 */
    double lowMel = hzToMel(0.0);
    double highMel = hzToMel(m_sampleRate / 2.0);

    /* 在Mel域均匀分布(numFilters + 2)个点 */
    QVector<double> melPoints(numFilters + 2);
    for (int i = 0; i < numFilters + 2; ++i) {
        melPoints[i] = lowMel + (highMel - lowMel) * i / (numFilters + 1);
    }

    /* 转回Hz并映射到FFT bin索引 */
    QVector<int> binPoints(numFilters + 2);
    for (int i = 0; i < numFilters + 2; ++i) {
        double hz = melToHz(melPoints[i]);
        binPoints[i] = qBound(0,
            static_cast<int>(qFloor((m_fftSize + 1) * hz / m_sampleRate)),
            specLen - 1);
    }

    /* 应用三角滤波器 */
    for (int m = 0; m < numFilters; ++m) {
        int left = binPoints[m];
        int center = binPoints[m + 1];
        int right = binPoints[m + 2];

        double sum = 0.0;

        /* 上升段: left -> center */
        for (int k = left; k <= center && k < specLen; ++k) {
            if (center > left) {
                sum += powerSpec[k] * (k - left) / static_cast<double>(center - left);
            }
        }

        /* 下降段: center -> right */
        for (int k = center + 1; k <= right && k < specLen; ++k) {
            if (right > center) {
                sum += powerSpec[k] * (right - k) / static_cast<double>(right - center);
            }
        }

        melOutput[m] = qMax(sum, 1e-10);
    }

    return melOutput;
}
