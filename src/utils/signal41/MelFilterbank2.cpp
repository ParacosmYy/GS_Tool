/**
 * @file MelFilterbank2.cpp
 * @brief Mel滤波器组2 — MFCC特征提取+三角/Slaney滤波实现
 *
 * 实现Mel频率滤波器组，支持：
 * - 三角滤波器和Slaney滤波器两种类型
 * - Hz ↔ Mel频率转换
 * - 滤波器组矩阵构建
 * - DCT变换提取MFCC系数
 * 所有运算带有QElapsedTimer计时和统计信息追踪。
 */

#include "utils/signal41/MelFilterbank2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
MelFilterbank2::MelFilterbank2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief Hz转Mel频率
 * @param hz 频率（Hz）
 * @return Mel频率
 */
double MelFilterbank2::hzToMel(double hz) const
{
    return 2595.0 * qLn(1.0 + hz / 700.0) / qLn(10.0);
}

/**
 * @brief Mel频率转Hz
 * @param mel Mel频率
 * @return 频率（Hz）
 */
double MelFilterbank2::melToHz(double mel) const
{
    return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0);
}

/**
 * @brief 设置滤波器组参数
 * @param fftSize FFT点数
 * @param numFilters 滤波器数量
 * @param sampleRate 采样率
 * @param lowFreq 低频截止（Hz），默认0
 * @param highFreq 高频截止（Hz），默认0表示采样率/2
 * @param type 滤波器类型
 */
void MelFilterbank2::setParameters(int fftSize, int numFilters, double sampleRate,
                                    double lowFreq, double highFreq, FilterType type)
{
    m_fftSize = fftSize;
    m_numFilters = numFilters;
    m_sampleRate = sampleRate;
    m_lowFreq = lowFreq;
    m_highFreq = (highFreq > 0.0) ? highFreq : sampleRate / 2.0;
    m_filterType = type;
    m_initialized = false;
    buildFilterBank();
}

/**
 * @brief 构建Mel滤波器组矩阵
 *
 * 将Mel频率等分为numFilters+2个区间，根据滤波器类型
 * 构建三角或Slaney形状的滤波器权重矩阵。
 */
void MelFilterbank2::buildFilterBank()
{
    if (m_fftSize <= 0 || m_numFilters <= 0) return;

    int numBins = m_fftSize / 2 + 1;

    /* 计算Mel频率点 */
    double lowMel = hzToMel(m_lowFreq);
    double highMel = hzToMel(m_highFreq);
    m_melFreqs.resize(m_numFilters + 2);
    for (int i = 0; i < m_numFilters + 2; ++i) {
        double mel = lowMel + i * (highMel - lowMel) / (m_numFilters + 1);
        m_melFreqs[i] = melToHz(mel);
    }

    /* 将Mel频率点映射到FFT bin索引 */
    QVector<double> fftFreqs(numBins);
    for (int i = 0; i < numBins; ++i)
        fftFreqs[i] = (double)m_fftSize / m_sampleRate * i;

    /* 构建滤波器组矩阵 */
    m_filterBank.resize(m_numFilters);
    for (int m = 0; m < m_numFilters; ++m) {
        m_filterBank[m].resize(numBins, 0.0);
        double fLeft = m_melFreqs[m];
        double fCenter = m_melFreqs[m + 1];
        double fRight = m_melFreqs[m + 2];

        for (int k = 0; k < numBins; ++k) {
            double fk = (double)k * m_sampleRate / m_fftSize;

            if (fk >= fLeft && fk <= fCenter && fCenter > fLeft) {
                if (m_filterType == Triangular) {
                    m_filterBank[m][k] = (fk - fLeft) / (fCenter - fLeft);
                } else {
                    /* Slaney: 带宽归一化 */
                    double bw = qMax(fCenter - fLeft, 1e-10);
                    m_filterBank[m][k] = (fk - fLeft) / bw;
                }
            } else if (fk > fCenter && fk <= fRight && fRight > fCenter) {
                if (m_filterType == Triangular) {
                    m_filterBank[m][k] = (fRight - fk) / (fRight - fCenter);
                } else {
                    double bw = qMax(fRight - fCenter, 1e-10);
                    m_filterBank[m][k] = (fRight - fk) / bw;
                }
            }
        }
    }

    m_initialized = true;
}

/**
 * @brief 计算功率谱的Mel滤波器组输出
 * @param powerSpectrum 功率谱（长度fftSize/2+1）
 * @return Mel频谱（长度numFilters）
 */
QVector<double> MelFilterbank2::compute(const QVector<double>& powerSpectrum)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_initialized) buildFilterBank();

    int numBins = qMin(powerSpectrum.size(), m_fftSize / 2 + 1);
    QVector<double> melSpectrum(m_numFilters, 0.0);
    double totalEnergy = 0.0;

    for (int m = 0; m < m_numFilters; ++m) {
        double sum = 0.0;
        for (int k = 0; k < numBins; ++k) {
            sum += powerSpectrum[k] * m_filterBank[m][k];
        }
        /* 取对数功率 */
        melSpectrum[m] = qLn(qMax(sum, 1e-10));
        totalEnergy += sum;
    }

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalComputations++;
    m_stats.totalFrames++;
    m_stats.numFilters = m_numFilters;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(m_numFilters, totalEnergy);
    return melSpectrum;
}

/**
 * @brief 对Mel频谱执行DCT变换，提取MFCC系数
 * @param melSpectrum Mel频谱
 * @param numCoeffs 输出系数数量，默认13
 * @return MFCC系数向量
 */
QVector<double> MelFilterbank2::dct(const QVector<double>& melSpectrum, int numCoeffs) const
{
    QVector<double> coeffs(numCoeffs, 0.0);
    int N = melSpectrum.size();
    if (N == 0) return coeffs;

    for (int i = 0; i < numCoeffs; ++i) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n) {
            sum += melSpectrum[n] * qCos(M_PI * i * (n + 0.5) / N);
        }
        coeffs[i] = sum;
    }
    return coeffs;
}

/**
 * @brief 获取Mel滤波器中心频率
 * @return 中心频率列表（Hz）
 */
QVector<double> MelFilterbank2::melFrequencies() const
{
    return m_melFreqs;
}

/**
 * @brief 重置所有统计计数器
 */
void MelFilterbank2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
