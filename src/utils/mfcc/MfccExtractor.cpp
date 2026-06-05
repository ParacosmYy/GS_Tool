/**
 * @file MfccExtractor.cpp
 * @brief MFCC特征提取实现 — 梅尔频率倒谱系数
 */

#include "utils/mfcc/MfccExtractor.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
MfccExtractor::MfccExtractor(QObject* parent)
    : QObject(parent)
    , m_fftSize(512)
    , m_numFilters(26)
    , m_timeSum(0.0)
{
}

/** @brief 提取MFCC特征
 *  @param signal 输入信号
 *  @param sampleRate 采样率
 *  @param numCoeffs 倒谱系数数量
 *  @return MFCC系数向量 */
QVector<double> MfccExtractor::extract(const QVector<double>& signal,
                                       double sampleRate, int numCoeffs)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n == 0) { return {}; }

    numCoeffs = qBound(1, numCoeffs, m_numFilters);

    /* 加Hann窗并补零到FFT大小 */
    int fftN = m_fftSize;
    QVector<double> real(fftN, 0.0), imag(fftN, 0.0);
    for (int i = 0; i < qMin(n, fftN); ++i) {
        double t = static_cast<double>(i) / static_cast<double>(qMin(n, fftN) - 1);
        real[i] = signal[i] * 0.5 * (1.0 - qCos(2.0 * M_PI * t));
    }

    /* FFT */
    fft(real, imag, false);

    /* 功率谱 */
    int halfN = fftN / 2 + 1;
    QVector<double> powerSpec(halfN);
    for (int i = 0; i < halfN; ++i) {
        powerSpec[i] = (real[i] * real[i] + imag[i] * imag[i]) / fftN;
    }

    /* 梅尔滤波器组 */
    auto filterbank = melFilterbank(m_numFilters, fftN, sampleRate);

    /* 应用滤波器组 */
    QVector<double> melEnergies(m_numFilters, 0.0);
    for (int m = 0; m < m_numFilters; ++m) {
        for (int k = 0; k < halfN; ++k) {
            melEnergies[m] += powerSpec[k] * filterbank[m][k];
        }
        melEnergies[m] = qMax(1e-30, melEnergies[m]);
    }

    /* 取对数 */
    QVector<double> logMel(m_numFilters);
    for (int m = 0; m < m_numFilters; ++m) {
        logMel[m] = std::log(melEnergies[m]);
    }

    /* DCT */
    QVector<double> mfcc = dct(logMel, numCoeffs);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalExtractions;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalExtractions);

    emit extractionCompleted(numCoeffs);
    return mfcc;
}

/** @brief 生成梅尔滤波器组
 *  @param numFilters 滤波器数量
 *  @param fftSize FFT大小
 *  @param sampleRate 采样率
 *  @return 滤波器组矩阵 */
QVector<QVector<double>> MfccExtractor::melFilterbank(int numFilters,
                                                       int fftSize,
                                                       double sampleRate)
{
    numFilters = qMax(1, numFilters);
    int halfN = fftSize / 2 + 1;
    double lowMel = hzToMel(0.0);
    double highMel = hzToMel(sampleRate / 2.0);

    /* 均匀分布的Mel频率点 */
    QVector<double> melPoints(numFilters + 2);
    for (int i = 0; i < numFilters + 2; ++i) {
        melPoints[i] = lowMel + (highMel - lowMel)
            * static_cast<double>(i) / static_cast<double>(numFilters + 1);
    }

    /* 转回Hz对应的FFT bin */
    QVector<int> bins(numFilters + 2);
    for (int i = 0; i < numFilters + 2; ++i) {
        double hz = melToHz(melPoints[i]);
        bins[i] = static_cast<int>(qRound(
            (fftSize + 1) * hz / sampleRate));
        bins[i] = qBound(0, bins[i], halfN - 1);
    }

    /* 三角滤波器 */
    QVector<QVector<double>> fbank(numFilters,
                                    QVector<double>(halfN, 0.0));
    for (int m = 0; m < numFilters; ++m) {
        int left = bins[m];
        int center = bins[m + 1];
        int right = bins[m + 2];

        for (int k = left; k <= center && k < halfN; ++k) {
            if (center > left) {
                fbank[m][k] = static_cast<double>(k - left)
                    / static_cast<double>(center - left);
            }
        }
        for (int k = center; k <= right && k < halfN; ++k) {
            if (right > center) {
                fbank[m][k] = static_cast<double>(right - k)
                    / static_cast<double>(right - center);
            }
        }
    }

    return fbank;
}

/** @brief 设置FFT大小 @param size FFT大小 */
void MfccExtractor::setFftSize(int size)
{
    m_fftSize = qMax(16, size);
}

/** @brief 设置梅尔滤波器数量 @param num 滤波器数量 */
void MfccExtractor::setNumFilters(int num)
{
    m_numFilters = qMax(4, num);
}

/** @brief 重置统计 */
void MfccExtractor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief Hz转Mel @param hz 频率 @return Mel值 */
double MfccExtractor::hzToMel(double hz)
{
    return 2595.0 * std::log10(1.0 + hz / 700.0);
}

/** @brief Mel转Hz @param mel Mel值 @return 频率 */
double MfccExtractor::melToHz(double mel)
{
    return 700.0 * (std::pow(10.0, mel / 2595.0) - 1.0);
}

/** @brief 基2 FFT(就地) */
void MfccExtractor::fft(QVector<double>& real, QVector<double>& imag,
                         bool inverse) const
{
    int n = real.size();
    if (n <= 1) return;
    imag.fill(0.0);

    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len *= 2) {
        double angle = sign * 2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int even = i + j, odd = i + j + len / 2;
                double tRe = curRe * real[odd] - curIm * imag[odd];
                double tIm = curRe * imag[odd] + curIm * real[odd];
                real[odd] = real[even] - tRe;
                imag[odd] = imag[even] - tIm;
                real[even] += tRe;
                imag[even] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }
}

/** @brief 离散余弦变换(DCT-II) */
QVector<double> MfccExtractor::dct(const QVector<double>& input,
                                    int numCoeffs)
{
    int N = input.size();
    QVector<double> result(numCoeffs, 0.0);
    for (int k = 0; k < numCoeffs; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n) {
            sum += input[n] * qCos(M_PI * static_cast<double>(k)
                 * (static_cast<double>(n) + 0.5) / static_cast<double>(N));
        }
        result[k] = sum;
    }
    return result;
}
