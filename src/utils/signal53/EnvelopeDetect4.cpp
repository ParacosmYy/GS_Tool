/**
 * @file EnvelopeDetect4.cpp
 * @brief 信号包络检测器实现
 *
 * 实现多种信号包络检测方法，包括Hilbert变换法和峰值检测法。
 * 包络检测广泛用于音频振幅分析、调制解调、心电信号处理等领域。
 *
 * Hilbert变换法: 通过构造解析信号 z(t) = x(t) + j*H{x(t)}，
 * 包络为解析信号的模 |z(t)|。
 *
 * 峰值检测法: 通过寻找局部极大值并线性插值构建包络。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/signal53/EnvelopeDetect4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
EnvelopeDetect4::EnvelopeDetect4(QObject* parent)
    : QObject(parent)
    , m_method("hilbert")
    , m_sampleRate(44100.0)
    , m_peak(0.0)
    , m_rms(0.0)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置包络检测方法
 * @param method 方法名称，支持 "hilbert"（Hilbert变换法）和 "peak"（峰值检测法）
 */
void EnvelopeDetect4::setMethod(const QString& method)
{
    if (method == "hilbert" || method == "peak") {
        m_method = method;
    }
}

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz），必须为正值
 */
void EnvelopeDetect4::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 检测信号包络
 *
 * 根据当前设置的方法（Hilbert变换法或峰值检测法）计算输入信号的包络。
 * 同时计算信号的峰值电平和RMS电平。
 *
 * @param signal 输入信号
 * @return 包络信号，长度与输入相同
 */
QVector<double> EnvelopeDetect4::detect(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> envelope;

    if (signal.isEmpty()) {
        return envelope;
    }

    const int n = signal.size();
    envelope.resize(n);

    /* 根据选择的方法计算包络 */
    if (m_method == "hilbert") {
        envelope = hilbertEnvelope(signal);
    } else {
        envelope = peakEnvelope(signal);
    }

    /* 计算峰值电平（包络最大值） */
    m_peak = 0.0;
    for (int i = 0; i < envelope.size(); ++i) {
        m_peak = qMax(m_peak, envelope[i]);
    }

    /* 计算RMS电平 */
    double sumSq = 0.0;
    for (int i = 0; i < n; ++i) {
        sumSq += signal[i] * signal[i];
    }
    m_rms = qSqrt(sumSq / n);

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalDetections++;
    m_stats.totalSamples += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionCompleted(n, m_peak);
    return envelope;
}

/**
 * @brief 使用Hilbert变换法计算包络
 *
 * 通过FFT实现Hilbert变换:
 * 1. 对输入信号做FFT
 * 2. 将正频率分量乘以2，直流和奈奎斯特频率不变，负频率置零
 * 3. 做逆FFT得到解析信号的虚部
 * 4. 包络 = sqrt(原信号^2 + Hilbert变换^2)
 *
 * @param sig 输入信号
 * @return Hilbert包络信号
 */
QVector<double> EnvelopeDetect4::hilbertEnvelope(const QVector<double>& sig) const
{
    const int n = sig.size();
    if (n == 0) {
        return sig;
    }

    /* 寻找大于等于n的最小2的幂次 */
    int fftLen = 1;
    while (fftLen < n) {
        fftLen <<= 1;
    }

    /* 准备FFT输入 */
    QVector<double> re(fftLen, 0.0);
    QVector<double> im(fftLen, 0.0);

    for (int i = 0; i < n; ++i) {
        re[i] = sig[i];
    }

    /* 位反转排列 */
    int j = 0;
    for (int i = 1; i < fftLen; ++i) {
        int bit = fftLen >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    /* 正向FFT（Cooley-Tukey基2） */
    for (int len = 2; len <= fftLen; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle);
        double wIm = qSin(angle);

        for (int i = 0; i < fftLen; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int k = 0; k < len / 2; ++k) {
                int u = i + k;
                int v = i + k + len / 2;
                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];
                re[v] = re[u] - tRe;
                im[v] = im[u] - tIm;
                re[u] += tRe;
                im[u] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }

    /* Hilbert变换: 正频率*2，负频率置零 */
    for (int i = 1; i < fftLen / 2; ++i) {
        re[i] *= 2.0;
        im[i] *= 2.0;
    }
    for (int i = fftLen / 2 + 1; i < fftLen; ++i) {
        re[i] = 0.0;
        im[i] = 0.0;
    }

    /* 逆FFT */
    for (int i = 0; i < fftLen; ++i) {
        im[i] = -im[i];
    }

    /* 再次位反转 */
    j = 0;
    for (int i = 1; i < fftLen; ++i) {
        int bit = fftLen >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    for (int len = 2; len <= fftLen; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle);
        double wIm = qSin(angle);
        for (int i = 0; i < fftLen; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int k = 0; k < len / 2; ++k) {
                int u = i + k;
                int v = i + k + len / 2;
                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];
                re[v] = re[u] - tRe;
                im[v] = im[u] - tIm;
                re[u] += tRe;
                im[u] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }

    /* 计算包络: |x(t) + j*h(t)| = sqrt(x^2 + h^2) */
    QVector<double> env(n);
    for (int i = 0; i < n; ++i) {
        double realPart = sig[i];
        double imagPart = -im[i] / fftLen;
        env[i] = qSqrt(realPart * realPart + imagPart * imagPart);
    }

    return env;
}

/**
 * @brief 使用峰值检测法计算包络
 *
 * 寻找信号中的局部极大值，然后在相邻峰值之间进行线性插值，
 * 构建平滑的上包络。对负半周取绝对值后同样处理。
 *
 * @param sig 输入信号
 * @return 峰值包络信号
 */
QVector<double> EnvelopeDetect4::peakEnvelope(const QVector<double>& sig) const
{
    const int n = sig.size();
    if (n == 0) {
        return sig;
    }

    /* 取绝对值 */
    QVector<double> absSig(n);
    for (int i = 0; i < n; ++i) {
        absSig[i] = qAbs(sig[i]);
    }

    /* 寻找局部峰值点 */
    QVector<int> peakIdx;
    peakIdx.append(0);  ///< 起始点

    for (int i = 1; i < n - 1; ++i) {
        if (absSig[i] >= absSig[i - 1] && absSig[i] >= absSig[i + 1]) {
            /* 去除过于密集的峰值（至少间隔3个采样） */
            if (peakIdx.isEmpty() || (i - peakIdx.last()) >= 3) {
                peakIdx.append(i);
            } else if (absSig[i] > absSig[peakIdx.last()]) {
                peakIdx.last() = i;  ///< 替换为更高的峰值
            }
        }
    }
    peakIdx.append(n - 1);  ///< 终止点

    /* 在峰值之间线性插值 */
    QVector<double> env(n, 0.0);
    for (int p = 0; p < peakIdx.size() - 1; ++p) {
        int i0 = peakIdx[p];
        int i1 = peakIdx[p + 1];
        double v0 = absSig[i0];
        double v1 = absSig[i1];

        for (int i = i0; i <= i1; ++i) {
            double t = (i1 > i0) ? static_cast<double>(i - i0) / (i1 - i0) : 0.0;
            env[i] = v0 + t * (v1 - v0);
        }
    }

    return env;
}

/**
 * @brief 重置所有统计计数器
 */
void EnvelopeDetect4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
