/**
 * @file SpectralWhitening.cpp
 * @brief 谱白化引擎实现 — FFT频域均衡+谱平坦度计算
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/signal26/SpectralWhitening.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数
 *
 * 初始化FFT大小并验证为2的幂。
 *
 * @param fftSize FFT大小(必须为2的幂,默认1024)
 * @param parent 父对象
 */
SpectralWhitening::SpectralWhitening(int fftSize, QObject* parent)
    : QObject(parent)
    , m_fftSize(fftSize)
{
    /* 确保fftSize为2的幂 */
    int p = 1;
    while (p < fftSize) {
        p <<= 1;
    }
    m_fftSize = p;
}

/**
 * @brief 白化信号
 *
 * 将信号功率谱白化(平坦化), 根据指定方法:
 * - Direct: X'(k) = X(k) / |X(k)|^alpha
 * - PhaseOnly: 仅保留相位,幅度置为1
 * - FrequencyMask: 用频率掩模平滑幅度谱后白化
 * - Adaptive: 调用adaptiveWhiten()
 *
 * @param signal 输入信号
 * @param method 白化方法
 * @return 白化后信号
 */
QVector<double> SpectralWhitening::whiten(const QVector<double>& signal, Method method)
{
    if (signal.isEmpty()) {
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    double flatBefore = spectralFlatness(signal);
    int n = signal.size();
    int fftN = m_fftSize;

    /* 零填充到FFT大小 */
    QVector<double> real(fftN, 0.0);
    QVector<double> imag(fftN, 0.0);
    int copyLen = qMin(n, fftN);
    for (int i = 0; i < copyLen; ++i) {
        real[i] = signal[i];
    }

    /* 正向FFT */
    fft(real, imag, false);

    QVector<double> realOut(fftN, 0.0);
    QVector<double> imagOut(fftN, 0.0);

    if (method == Direct) {
        /* 直接除以幅度谱的alpha次方 */
        const double alpha = 0.5;
        for (int k = 0; k < fftN; ++k) {
            double mag = std::sqrt(real[k] * real[k] + imag[k] * imag[k]);
            if (mag > 1e-10) {
                double scale = std::pow(mag, alpha) / mag;
                realOut[k] = real[k] * scale;
                imagOut[k] = imag[k] * scale;
            } else {
                realOut[k] = 0.0;
                imagOut[k] = 0.0;
            }
        }
    } else if (method == PhaseOnly) {
        /* 仅保留相位,幅度置为1 */
        for (int k = 0; k < fftN; ++k) {
            double mag = std::sqrt(real[k] * real[k] + imag[k] * imag[k]);
            if (mag > 1e-10) {
                realOut[k] = real[k] / mag;
                imagOut[k] = imag[k] / mag;
            } else {
                realOut[k] = 0.0;
                imagOut[k] = 0.0;
            }
        }
    } else if (method == FrequencyMask) {
        /* 频率掩模: 对幅度谱进行平滑后作为权重 */
        QVector<double> mag(fftN);
        for (int k = 0; k < fftN; ++k) {
            mag[k] = std::sqrt(real[k] * real[k] + imag[k] * imag[k]);
        }
        /* 三点滑动平均平滑 */
        QVector<double> smoothMag(fftN);
        for (int k = 0; k < fftN; ++k) {
            double sum = mag[k];
            int cnt = 1;
            if (k > 0) { sum += mag[k - 1]; ++cnt; }
            if (k < fftN - 1) { sum += mag[k + 1]; ++cnt; }
            smoothMag[k] = sum / cnt;
        }
        for (int k = 0; k < fftN; ++k) {
            if (smoothMag[k] > 1e-10) {
                double scale = 1.0 / smoothMag[k];
                realOut[k] = real[k] * scale;
                imagOut[k] = imag[k] * scale;
            }
        }
    } else {
        /* Adaptive方法 */
        timer.invalidate();
        return adaptiveWhiten(signal, 0.98);
    }

    /* 逆FFT */
    fft(realOut, imagOut, true);

    /* 取前n个样本作为结果 */
    QVector<double> result(n);
    int outLen = qMin(n, fftN);
    for (int i = 0; i < outLen; ++i) {
        result[i] = realOut[i];
    }

    double flatAfter = spectralFlatness(result);

    m_stats.totalWhitens++;
    m_stats.totalSamplesProcessed += n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalWhitens;

    emit whiteningCompleted(flatBefore, flatAfter);
    return result;
}

/**
 * @brief 自适应均衡白化
 *
 * 使用指数移动平均估计参考谱, 然后用当前谱除以参考谱。
 * 平滑因子越大,参考谱更新越慢,白化效果越强。
 *
 * @param signal 输入信号
 * @param smoothingFactor 平滑因子[0,1], 越大参考谱越稳定
 * @return 白化后信号
 */
QVector<double> SpectralWhitening::adaptiveWhiten(const QVector<double>& signal,
                                                    double smoothingFactor)
{
    if (signal.isEmpty()) {
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    int fftN = m_fftSize;
    int hopSize = fftN / 2;
    int frames = qMax(1, (n - fftN) / hopSize + 1);

    QVector<double> result(n, 0.0);
    QVector<double> refMag(fftN, 0.0);
    bool refInitialized = false;

    for (int f = 0; f < frames; ++f) {
        int start = f * hopSize;

        /* 提取帧并加窗(Hann窗) */
        QVector<double> real(fftN, 0.0);
        QVector<double> imag(fftN, 0.0);
        for (int i = 0; i < fftN; ++i) {
            int idx = start + i;
            if (idx < n) {
                double hann = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (fftN - 1)));
                real[i] = signal[idx] * hann;
            }
        }

        fft(real, imag, false);

        /* 计算幅度谱 */
        QVector<double> mag(fftN);
        for (int k = 0; k < fftN; ++k) {
            mag[k] = std::sqrt(real[k] * real[k] + imag[k] * imag[k]);
        }

        /* 更新参考谱(指数移动平均) */
        if (!refInitialized) {
            refMag = mag;
            refInitialized = true;
        } else {
            for (int k = 0; k < fftN; ++k) {
                refMag[k] = smoothingFactor * refMag[k]
                           + (1.0 - smoothingFactor) * mag[k];
            }
        }

        /* 白化: 除以参考谱 */
        QVector<double> realW(fftN, 0.0);
        QVector<double> imagW(fftN, 0.0);
        for (int k = 0; k < fftN; ++k) {
            if (refMag[k] > 1e-10) {
                realW[k] = real[k] / refMag[k];
                imagW[k] = imag[k] / refMag[k];
            }
        }

        /* 逆FFT */
        fft(realW, imagW, true);

        /* 重叠相加 */
        for (int i = 0; i < fftN; ++i) {
            int idx = start + i;
            if (idx < n) {
                double hann = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (fftN - 1)));
                result[idx] += realW[i] * hann;
            }
        }
    }

    m_stats.totalWhitens++;
    m_stats.totalSamplesProcessed += n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalWhitens;

    return result;
}

/**
 * @brief 计算谱平坦度
 *
 * 谱平坦度 = 几何平均功率谱 / 算术平均功率谱。
 * 值域[0,1], 1表示完全平坦(白噪声), 0表示纯音。
 *
 * @param signal 输入信号
 * @return 谱平坦度[0,1]
 */
double SpectralWhitening::spectralFlatness(const QVector<double>& signal) const
{
    QVector<double> psd = powerSpectrum(signal);
    if (psd.isEmpty()) {
        return 0.0;
    }

    /* 仅取正频率部分 */
    int halfN = psd.size() / 2 + 1;
    double sumLog = 0.0;
    double sumLin = 0.0;
    int count = 0;

    for (int k = 1; k < halfN; ++k) {
        double val = qMax(psd[k], 1e-30);
        sumLog += std::log(val);
        sumLin += val;
        ++count;
    }

    if (count == 0 || sumLin < 1e-30) {
        return 0.0;
    }

    double geoMean = std::exp(sumLog / count);
    double ariMean = sumLin / count;

    return qBound(0.0, geoMean / ariMean, 1.0);
}

/**
 * @brief 计算功率谱
 *
 * 通过FFT计算信号的功率谱密度。
 * 使用|X(k)|^2作为功率估计。
 *
 * @param signal 输入信号
 * @return 功率谱(长度为fftSize)
 */
QVector<double> SpectralWhitening::powerSpectrum(const QVector<double>& signal) const
{
    if (signal.isEmpty()) {
        return {};
    }

    int n = signal.size();
    int fftN = m_fftSize;

    QVector<double> real(fftN, 0.0);
    QVector<double> imag(fftN, 0.0);
    int copyLen = qMin(n, fftN);
    for (int i = 0; i < copyLen; ++i) {
        real[i] = signal[i];
    }

    const_cast<SpectralWhitening*>(this)->fft(real, imag, false);

    QVector<double> psd(fftN);
    for (int k = 0; k < fftN; ++k) {
        psd[k] = (real[k] * real[k] + imag[k] * imag[k]) / fftN;
    }
    return psd;
}

/**
 * @brief 基2 FFT(原地)
 *
 * Cooley-Tukey基2 FFT算法, 支持正向和逆向变换。
 * 逆向变换通过共轭+正向FFT+共轭+除以N实现。
 *
 * @param real 实部(原地修改)
 * @param imag 虚部(原地修改)
 * @param inverse true为逆变换, false为正变换
 */
void SpectralWhitening::fft(QVector<double>& real, QVector<double>& imag, bool inverse) const
{
    int n = real.size();
    if (n <= 1) {
        return;
    }

    /* 比特反转重排 */
    int j = 0;
    for (int i = 1; i < n; ++i) {
        int bit = n >> 1;
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

    /* 蝶形运算 */
    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len <<= 1) {
        double angle = sign * 2.0 * M_PI / len;
        double wReal = std::cos(angle);
        double wImag = std::sin(angle);

        for (int i = 0; i < n; i += len) {
            double curReal = 1.0;
            double curImag = 0.0;
            for (int k = 0; k < len / 2; ++k) {
                int evenIdx = i + k;
                int oddIdx = i + k + len / 2;

                double tReal = curReal * real[oddIdx] - curImag * imag[oddIdx];
                double tImag = curReal * imag[oddIdx] + curImag * real[oddIdx];

                real[oddIdx] = real[evenIdx] - tReal;
                imag[oddIdx] = imag[evenIdx] - tImag;
                real[evenIdx] += tReal;
                imag[evenIdx] += tImag;

                double newCurReal = curReal * wReal - curImag * wImag;
                double newCurImag = curReal * wImag + curImag * wReal;
                curReal = newCurReal;
                curImag = newCurImag;
            }
        }
    }

    /* 逆变换需除以N */
    if (inverse) {
        double invN = 1.0 / n;
        for (int i = 0; i < n; ++i) {
            real[i] *= invN;
            imag[i] *= invN;
        }
    }
}

/**
 * @brief 重置统计信息
 */
void SpectralWhitening::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
