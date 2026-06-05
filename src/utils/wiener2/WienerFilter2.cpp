/**
 * @file WienerFilter2.cpp
 * @brief 频域维纳滤波器实现 — 信号去噪
 */

#include "utils/wiener2/WienerFilter2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
WienerFilter2::WienerFilter2(QObject* parent)
    : QObject(parent)
    , m_regularization(1e-6)
    , m_timeSum(0.0)
{
}

/** @brief 设计维纳滤波器
 *  @param signalPower 信号功率谱
 *  @param noisePower  噪声功率谱
 *  @return 滤波器频率响应 */
QVector<double> WienerFilter2::design(const QVector<double>& signalPower,
                                      const QVector<double>& noisePower)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(signalPower.size(), noisePower.size());
    m_response.resize(n);

    for (int i = 0; i < n; ++i) {
        double sp = qMax(0.0, signalPower[i]);
        double np = qMax(0.0, noisePower[i]);
        /* 维纳增益: H(f) = S(f) / (S(f) + N(f)) */
        m_response[i] = sp / (sp + np + m_regularization);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDesigns;
    m_timeSum += elapsed;
    double total = static_cast<double>(m_stats.totalDesigns + m_stats.totalApplications);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit designCompleted(n);
    return m_response;
}

/** @brief 应用维纳滤波器去噪
 *  @param signal 输入信号
 *  @return 去噪后信号 */
QVector<double> WienerFilter2::apply(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 4) { return signal; }

    /* 补零到2的幂 */
    int fftSize = 1;
    while (fftSize < n) fftSize *= 2;

    /* 如果没有预先设计滤波器，自动估计功率谱 */
    if (m_response.isEmpty()) {
        QVector<double> sigPsd = estimatePsd(signal);
        int halfPsd = sigPsd.size() / 2;
        double noiseEst = 0.0;
        int cnt = 0;
        for (int i = halfPsd; i < sigPsd.size(); ++i) {
            noiseEst += sigPsd[i]; ++cnt;
        }
        noiseEst = (cnt > 0) ? noiseEst / cnt : 1.0;

        QVector<double> noisePsd(sigPsd.size(), noiseEst);
        design(sigPsd, noisePsd);
    }

    /* FFT */
    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < n; ++i) real[i] = signal[i];
    fft(real, imag, false);

    /* 应用维纳增益 */
    int halfN = fftSize / 2;
    int respSize = m_response.size();
    for (int i = 0; i < halfN; ++i) {
        double gain = (i < respSize) ? m_response[i] : 0.0;
        real[i] *= gain;
        imag[i] *= gain;
        if (i > 0 && i < halfN) {
            real[fftSize - i] *= gain;
            imag[fftSize - i] *= gain;
        }
    }

    /* 逆FFT */
    fft(real, imag, true);

    QVector<double> result(n);
    double scale = 1.0 / static_cast<double>(fftSize);
    for (int i = 0; i < n; ++i) result[i] = real[i] * scale;

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalApplications;
    m_timeSum += elapsed;
    double totalCount = static_cast<double>(m_stats.totalDesigns + m_stats.totalApplications);
    m_stats.avgProcessingTimeMs = (totalCount > 0) ? m_timeSum / totalCount : 0.0;

    emit applicationCompleted(n);
    return result;
}

/** @brief 设置正则化参数 @param lambda 正则化系数 */
void WienerFilter2::setRegularization(double lambda)
{
    m_regularization = qMax(1e-12, lambda);
}

/** @brief 重置统计 */
void WienerFilter2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 基2 FFT(就地)
 *  @param real 实部 @param imag 虚部 @param inverse 逆变换 */
void WienerFilter2::fft(QVector<double>& real, QVector<double>& imag,
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
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }
}

/** @brief 估计功率谱 @param signal 信号 @return 功率谱 */
QVector<double> WienerFilter2::estimatePsd(const QVector<double>& signal) const
{
    int n = signal.size();
    int fftSize = 1;
    while (fftSize < n) fftSize *= 2;

    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < n; ++i) real[i] = signal[i];

    /* Hann窗 */
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
