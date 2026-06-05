/**
 * @file WienerFilter.cpp
 * @brief 维纳反卷积滤波器实现 — 频域信号恢复
 */

#include "utils/wiener/WienerFilter.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
WienerFilter::WienerFilter(QObject* parent)
    : QObject(parent)
    , m_regularization(1e-6)
    , m_timeSum(0.0)
{
}

/** @brief 维纳滤波
 *  @param signal    输入信号
 *  @param noisePsd  噪声功率谱密度
 *  @param signalPsd 信号功率谱密度
 *  @return 滤波后信号 */
QVector<double> WienerFilter::filter(
    const QVector<double>& signal,
    const QVector<double>& noisePsd,
    const QVector<double>& signalPsd)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 4) {
        return signal;
    }

    /* 补零到2的幂 */
    int fftSize = 1;
    while (fftSize < n) fftSize *= 2;

    /* 复制到FFT缓冲区 */
    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < n; ++i) {
        real[i] = signal[i];
    }

    /* 正向FFT */
    fft(real, imag, false);

    /* 维纳滤波: H(f) = S(f) / (S(f) + N(f))
     * 其中 S = signalPsd, N = noisePsd */
    int psdSize = qMin(signalPsd.size(), noisePsd.size());
    int halfN = fftSize / 2;

    for (int i = 0; i < halfN; ++i) {
        double sPsd = (i < psdSize) ? signalPsd[i] : 0.0;
        double nPsd = (i < psdSize) ? noisePsd[i] : 1.0;

        /* 维纳增益: S/(S+N) + 正则化 */
        double gain = sPsd / (sPsd + nPsd + m_regularization);

        real[i] *= gain;
        imag[i] *= gain;

        /* 对称频率 */
        if (i > 0 && i < halfN) {
            real[fftSize - i] *= gain;
            imag[fftSize - i] *= gain;
        }
    }

    /* 逆FFT */
    fft(real, imag, true);

    /* 提取结果 */
    QVector<double> result(n);
    double scale = 1.0 / static_cast<double>(fftSize);
    for (int i = 0; i < n; ++i) {
        result[i] = real[i] * scale;
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalFilterings;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFilterings);

    emit filteringCompleted(n);
    return result;
}

/** @brief 设置正则化参数 @param lambda 正则化系数 */
void WienerFilter::setRegularization(double lambda)
{
    m_regularization = qMax(1e-12, lambda);
}

/** @brief 频域维纳滤波(自动FFT)
 *  @param signal  输入信号
 *  @param fftSize FFT大小(0表示自动)
 *  @return 滤波后信号 */
QVector<double> WienerFilter::frequencyDomainFilter(
    const QVector<double>& signal, int fftSize)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 4) {
        return signal;
    }

    /* 自动FFT大小 */
    if (fftSize <= 0) {
        fftSize = 1;
        while (fftSize < n) fftSize *= 2;
    }

    /* 估计信号和噪声的功率谱 */
    QVector<double> sigPsd = estimatePsd(signal);

    /* 噪声功率谱: 用高频部分的平均值估计 */
    int halfPsd = sigPsd.size() / 2;
    double noiseEst = 0.0;
    int noiseCount = 0;
    for (int i = halfPsd; i < sigPsd.size(); ++i) {
        noiseEst += sigPsd[i];
        ++noiseCount;
    }
    noiseEst = (noiseCount > 0) ? noiseEst / noiseCount : 1.0;

    QVector<double> noisePsd(sigPsd.size(), noiseEst);

    /* 复制到FFT缓冲区 */
    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < qMin(n, fftSize); ++i) {
        real[i] = signal[i];
    }

    fft(real, imag, false);

    int halfN = fftSize / 2;
    for (int i = 0; i < halfN; ++i) {
        double sPsd = (i < sigPsd.size()) ? sigPsd[i] : 0.0;
        double nPsd = (i < noisePsd.size()) ? noisePsd[i] : noiseEst;
        double gain = sPsd / (sPsd + nPsd + m_regularization);

        real[i] *= gain;
        imag[i] *= gain;

        if (i > 0 && i < halfN) {
            real[fftSize - i] *= gain;
            imag[fftSize - i] *= gain;
        }
    }

    fft(real, imag, true);

    QVector<double> result(n);
    double scale = 1.0 / static_cast<double>(fftSize);
    for (int i = 0; i < n; ++i) {
        result[i] = real[i] * scale;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalFilterings;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFilterings);

    emit filteringCompleted(n);
    return result;
}

/** @brief 重置统计 */
void WienerFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 基2 FFT(就地)
 *  @param real    实部
 *  @param imag    虚部
 *  @param inverse 是否逆变换 */
void WienerFilter::fft(QVector<double>& real, QVector<double>& imag,
                       bool inverse) const
{
    int n = real.size();
    if (n <= 1) return;

    imag.fill(0.0);

    /* 位反转排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
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
                double newIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
                curIm = newIm;
            }
        }
    }
}

/** @brief 估计功率谱 @param signal 信号 @return 功率谱 */
QVector<double> WienerFilter::estimatePsd(const QVector<double>& signal) const
{
    int n = signal.size();
    int fftSize = 1;
    while (fftSize < n) fftSize *= 2;

    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < n; ++i) {
        real[i] = signal[i];
    }

    /* 应用Hann窗 */
    for (int i = 0; i < n; ++i) {
        double t = static_cast<double>(i) / static_cast<double>(n - 1);
        real[i] *= 0.5 * (1.0 - qCos(2.0 * M_PI * t));
    }

    fft(real, imag, false);

    int halfN = fftSize / 2;
    QVector<double> psd(halfN);
    for (int i = 0; i < halfN; ++i) {
        psd[i] = (real[i] * real[i] + imag[i] * imag[i]) / n;
    }

    return psd;
}
