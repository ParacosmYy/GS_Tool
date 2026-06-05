#include "Cepstrum4.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class Cepstrum4
 * @brief 倒谱(Cepstrum)分析器实现
 *
 * 倒谱是"频谱的频谱"，通过对数功率谱再做逆傅里叶变换得到。
 * 在语音处理中广泛用于基频检测(倒谱峰值对应基频周期)
 * 和声道特征提取(MFCC特征)。
 *
 * 实倒谱: |IFFT(log(|FFT(x)|^2))|
 * 复倒谱: IFFT(log(FFT(x)))
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
Cepstrum4::Cepstrum4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 简化FFT实现(基2 Cooley-Tukey)
 *
 * 就地计算FFT，支持逆变换。用于倒谱计算中的正/逆变换。
 *
 * @param real 实部数组
 * @param imag 虚部数组
 * @param inverse 是否逆变换
 */
static void simpleFFT(QVector<double>& real, QVector<double>& imag, bool inverse)
{
    int N = real.size();
    /* 位反转排列 */
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    for (int len = 2; len <= N; len <<= 1) {
        double angle = 2.0 * M_PI / len * (inverse ? -1.0 : 1.0);
        double wRe = qCos(angle);
        double wIm = qSin(angle);

        for (int i = 0; i < N; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tRe = curRe * real[v] - curIm * imag[v];
                double tIm = curRe * imag[v] + curIm * real[v];
                real[v] = real[u] - tRe;
                imag[v] = imag[u] - tIm;
                real[u] += tRe;
                imag[u] += tIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < N; ++i) {
            real[i] /= N;
            imag[i] /= N;
        }
    }
}

/**
 * @brief 计算实倒谱
 *
 * 实倒谱的计算步骤:
 * 1. FFT求频谱
 * 2. 取功率谱的对数
 * 3. IFFT回到"倒频"域
 *
 * 倒谱的横轴为倒频率(quefrency)，单位与时间相同。
 * 倒谱中的峰值位置对应原信号的基频周期。
 *
 * @param signal 输入信号(建议长度为2的幂)
 * @return 实倒谱向量
 */
QVector<double> Cepstrum4::realCepstrum(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    /* 补零到2的幂 */
    int fftSize = 1;
    while (fftSize < N) fftSize <<= 1;

    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < N; ++i) {
        real[i] = signal[i];
    }

    /* FFT */
    simpleFFT(real, imag, false);

    /* 取对数功率谱 */
    for (int i = 0; i < fftSize; ++i) {
        double mag = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
        real[i] = qLn(qMax(mag, 1e-15));
        imag[i] = 0.0;
    }

    /* IFFT */
    simpleFFT(real, imag, true);

    /* 取实部的绝对值作为实倒谱 */
    QVector<double> cepstrum(N);
    double peakQ = 0.0;
    double peakVal = 0.0;
    for (int i = 0; i < N; ++i) {
        cepstrum[i] = qAbs(real[i]);
        if (i > 1 && cepstrum[i] > peakVal) {
            peakVal = cepstrum[i];
            peakQ = i;
        }
    }

    m_stats.totalCepstraComputed++;
    m_stats.totalPeaksFound++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalCepstraComputed);

    emit cepstrumComputed(N, peakQ);

    return cepstrum;
}

/**
 * @brief 计算复倒谱
 *
 * 复倒谱保留相位信息，需要相位展开。
 * 步骤: FFT -> 对数(幅度+展开相位) -> IFFT
 *
 * @param signal 输入信号
 * @return 复倒谱向量(实部)
 */
QVector<double> Cepstrum4::complexCepstrum(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    int fftSize = 1;
    while (fftSize < N) fftSize <<= 1;

    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < N; ++i) {
        real[i] = signal[i];
    }

    simpleFFT(real, imag, false);

    /* 对数: log(|X|) + j*unwrap(angle(X)) */
    double prevPhase = 0.0;
    for (int i = 0; i < fftSize; ++i) {
        double mag = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
        double phase = qAtan2(imag[i], real[i]);

        /* 简化相位展开 */
        double diff = phase - prevPhase;
        while (diff > M_PI) diff -= 2.0 * M_PI;
        while (diff < -M_PI) diff += 2.0 * M_PI;
        phase = prevPhase + diff;
        prevPhase = phase;

        real[i] = qLn(qMax(mag, 1e-15));
        imag[i] = phase;
    }

    simpleFFT(real, imag, true);

    QVector<double> cepstrum(N);
    for (int i = 0; i < N; ++i) {
        cepstrum[i] = real[i];
    }

    m_stats.totalCepstraComputed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalCepstraComputed);

    emit cepstrumComputed(N, 0.0);

    return cepstrum;
}

/**
 * @brief 重置所有统计数据
 *
 * 将倒谱计算计数、峰值计数和计时归零。
 */
void Cepstrum4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
