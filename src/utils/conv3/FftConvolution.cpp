/**
 * @file FftConvolution.cpp
 * @brief FFT快速卷积引擎实现 — 重叠相加法频域卷积
 */

#include "utils/conv3/FftConvolution.h"

#include <QtMath>
#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
FftConvolution::FftConvolution(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置卷积模式 @param mode 模式 */
void FftConvolution::setMode(Mode mode)
{
    m_mode = mode;
}

/** @brief 设置分块大小 @param size 块大小 */
void FftConvolution::setBlockSize(int size)
{
    m_blockSize = (size > 0) ? nextPowerOf2(size) : 0;
}

/** @brief 执行卷积 @param signal 输入信号 @param kernel 卷积核 @return 卷积结果 */
QVector<double> FftConvolution::convolve(const QVector<double>& signal,
                                          const QVector<double>& kernel)
{
    if (signal.isEmpty() || kernel.isEmpty()) {
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    /* 短核使用直接卷积 */
    int kernelLen = kernel.size();
    int sigLen = signal.size();

    if (kernelLen <= 32 || sigLen <= 64) {
        /* 直接时域卷积 */
        int fullLen = sigLen + kernelLen - 1;
        result.resize(fullLen);
        for (int i = 0; i < fullLen; ++i) {
            double sum = 0.0;
            for (int j = 0; j < kernelLen; ++j) {
                int si = i - j;
                if (si >= 0 && si < sigLen) {
                    sum += signal[si] * kernel[j];
                }
            }
            result[i] = sum;
        }
    } else {
        /* 频域卷积 */
        int fullLen = sigLen + kernelLen - 1;
        int fftLen = nextPowerOf2(fullLen);

        /* 零填充到fftLen */
        QVector<QComplexDouble> sigFreq(fftLen);
        QVector<QComplexDouble> kerFreq(fftLen);
        for (int i = 0; i < sigLen; ++i) {
            sigFreq[i] = QComplexDouble(signal[i], 0.0);
        }
        for (int i = 0; i < kernelLen; ++i) {
            kerFreq[i] = QComplexDouble(kernel[i], 0.0);
        }

        /* 正变换 */
        fft(sigFreq);
        fft(kerFreq);

        /* 频域乘法 */
        for (int i = 0; i < fftLen; ++i) {
            sigFreq[i] = sigFreq[i] * kerFreq[i];
        }

        /* 逆变换 */
        fft(sigFreq, true);

        /* 提取有效结果 */
        result.resize(fullLen);
        double scale = 1.0 / fftLen;
        for (int i = 0; i < fullLen; ++i) {
            result[i] = sigFreq[i].real() * scale;
        }

        m_stats.totalFftsExecuted += 3;
    }

    result = trimOutput(result, sigLen, kernelLen);

    /* 统计更新 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    ++m_stats.totalConvolutions;
    m_stats.totalSamplesProcessed += static_cast<quint64>(sigLen + kernelLen);
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalConvolutions;

    emit convolutionComplete(result.size(), static_cast<double>(elapsed));
    return result;
}

/** @brief 重叠相加法卷积 @param signal 长信号 @param kernel 卷积核 @return 结果 */
QVector<double> FftConvolution::overlapAdd(const QVector<double>& signal,
                                            const QVector<double>& kernel)
{
    if (signal.isEmpty() || kernel.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    int kernelLen = kernel.size();
    int sigLen = signal.size();
    int fullLen = sigLen + kernelLen - 1;

    /* 确定块大小 */
    int blockSz = m_blockSize;
    if (blockSz <= 0) {
        blockSz = qMax(64, nextPowerOf2(kernelLen * 4));
    }

    /* FFT长度必须 >= blockSize + kernelLen - 1 */
    int fftLen = nextPowerOf2(blockSz + kernelLen - 1);

    /* 预计算核的FFT */
    QVector<QComplexDouble> kerFreq(fftLen);
    for (int i = 0; i < kernelLen; ++i) {
        kerFreq[i] = QComplexDouble(kernel[i], 0.0);
    }
    fft(kerFreq);
    m_stats.totalFftsExecuted += 1;

    /* 输出缓冲 */
    QVector<double> output(fullLen, 0.0);

    /* 分块处理 */
    int pos = 0;
    while (pos < sigLen) {
        int thisBlock = qMin(blockSz, sigLen - pos);

        /* 零填充当前块 */
        QVector<QComplexDouble> blockFreq(fftLen);
        for (int i = 0; i < thisBlock; ++i) {
            blockFreq[i] = QComplexDouble(signal[pos + i], 0.0);
        }

        /* 正变换 + 频域乘法 + 逆变换 */
        fft(blockFreq);
        for (int i = 0; i < fftLen; ++i) {
            blockFreq[i] = blockFreq[i] * kerFreq[i];
        }
        fft(blockFreq, true);

        /* 重叠相加到输出 */
        double scale = 1.0 / fftLen;
        int outStart = pos;
        for (int i = 0; i < fftLen && (outStart + i) < fullLen; ++i) {
            output[outStart + i] += blockFreq[i].real() * scale;
        }

        pos += blockSz;
        ++m_stats.totalBlocksProcessed;
        m_stats.totalFftsExecuted += 2;
    }

    QVector<double> trimmed = trimOutput(output, sigLen, kernelLen);

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    ++m_stats.totalConvolutions;
    m_stats.totalSamplesProcessed += static_cast<quint64>(sigLen + kernelLen);
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalConvolutions;

    emit convolutionComplete(trimmed.size(), static_cast<double>(elapsed));
    return trimmed;
}

/** @brief 互相关(频域) @param a 信号A @param b 信号B @return 互相关结果 */
QVector<double> FftConvolution::crossCorrelate(const QVector<double>& a,
                                                const QVector<double>& b)
{
    if (a.isEmpty() || b.isEmpty()) return {};

    int fullLen = a.size() + b.size() - 1;
    int fftLen = nextPowerOf2(fullLen);

    /* 翻转b用于互相关 */
    QVector<QComplexDouble> aFreq(fftLen);
    QVector<QComplexDouble> bFreq(fftLen);
    for (int i = 0; i < a.size(); ++i) {
        aFreq[i] = QComplexDouble(a[i], 0.0);
    }
    for (int i = 0; i < b.size(); ++i) {
        bFreq[i] = QComplexDouble(b[b.size() - 1 - i], 0.0);
    }

    fft(aFreq);
    fft(bFreq);
    for (int i = 0; i < fftLen; ++i) {
        aFreq[i] = aFreq[i] * bFreq[i];
    }
    fft(aFreq, true);

    QVector<double> result(fullLen);
    double scale = 1.0 / fftLen;
    for (int i = 0; i < fullLen; ++i) {
        result[i] = aFreq[i].real() * scale;
    }

    m_stats.totalFftsExecuted += 3;
    return result;
}

/** @brief 重置统计 */
void FftConvolution::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 基2 FFT @param data 复数数组 @param inverse 逆变换 */
void FftConvolution::fft(QVector<QComplexDouble>& data, bool inverse)
{
    int n = data.size();
    if (n <= 1) return;

    /* 位反转置换 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }

    /* 蝶形运算 */
    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len <<= 1) {
        double angle = sign * 2.0 * M_PI / len;
        QComplexDouble wLen(qCos(angle), qSin(angle));

        for (int i = 0; i < n; i += len) {
            QComplexDouble w(1.0, 0.0);
            for (int j = 0; j < len / 2; ++j) {
                QComplexDouble u = data[i + j];
                QComplexDouble v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w = w * wLen;
            }
        }
    }

    if (inverse) {
        for (auto& d : data) {
            d /= n;
        }
    }
}

/** @brief 下一个2的幂 @param n 输入 @return 2幂 */
int FftConvolution::nextPowerOf2(int n)
{
    if (n <= 1) return 1;
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/** @brief 裁剪输出 @param full 完整结果 @param sigLen 信号长度 @param kerLen 核长度 */
QVector<double> FftConvolution::trimOutput(const QVector<double>& full,
                                            int sigLen, int kerLen) const
{
    switch (m_mode) {
    case Mode::Same: {
        int targetLen = qMax(sigLen, kerLen);
        int start = (full.size() - targetLen) / 2;
        QVector<double> result(targetLen);
        for (int i = 0; i < targetLen; ++i) {
            result[i] = full[start + i];
        }
        return result;
    }
    case Mode::Valid: {
        int validLen = qMax(sigLen, kerLen) - qMin(sigLen, kerLen) + 1;
        if (validLen <= 0) return {};
        int start = qMin(sigLen, kerLen) - 1;
        QVector<double> result(validLen);
        for (int i = 0; i < validLen; ++i) {
            result[i] = full[start + i];
        }
        return result;
    }
    default:
        return full;
    }
}
