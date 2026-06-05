/**
 * @file SpectralSubtract3.cpp
 * @brief 谱减法降噪算法实现
 *
 * 实现经典的谱减法(Spectral Subtraction)降噪。首先通过
 * 噪声帧学习噪声功率谱，然后在频域中从含噪信号减去噪声
 * 估计（带过减因子），最后通过IFFT重建时域信号。
 * 使用QElapsedTimer计时。
 */

#include "utils/signal51/SpectralSubtract3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @class SpectralSubtract3
 * @brief 谱减法降噪器
 *
 * 处理流程：
 * 1. learnNoise() 学习噪声功率谱（多帧平均）
 * 2. process() 对含噪帧执行：FFT -> 谱减 -> IFFT
 * 3. 谱减使用过减因子和频谱下限防止音乐噪声
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
SpectralSubtract3::SpectralSubtract3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz）
 */
void SpectralSubtract3::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置FFT大小
 * @param n FFT点数，建议为2的幂
 */
void SpectralSubtract3::setFFTSize(int n)
{
    m_fftSize = qMax(4, n);
    m_noiseLearned = false;
    m_noiseProfile.clear();
}

/**
 * @brief 设置过减因子
 * @param factor 过减因子，大于1.0增强降噪但可能引入音乐噪声
 */
void SpectralSubtract3::setOversubtraction(double factor)
{
    m_oversub = qMax(0.0, factor);
}

/**
 * @brief 设置频谱下限（dB）
 * @param db 频谱下限，防止过度减谱导致负功率
 */
void SpectralSubtract3::setFloor(double db)
{
    m_floor = db;
}

/**
 * @brief 学习噪声功率谱
 *
 * 计算输入噪声帧的功率谱并存储为噪声模板。
 * 后续process()调用将基于此模板进行谱减。
 *
 * @param noiseFrame 纯噪声帧（长度应为FFT大小）
 */
void SpectralSubtract3::learnNoise(const QVector<double>& noiseFrame)
{
    int n = qMin(noiseFrame.size(), m_fftSize);

    QVector<double> re(n, 0.0), im(n, 0.0);
    for (int i = 0; i < n; ++i) {
        re[i] = noiseFrame[i];
    }

    fft(re, im);

    /* 计算功率谱 |X(k)|^2 */
    m_noiseProfile.resize(n);
    for (int i = 0; i < n; ++i) {
        m_noiseProfile[i] = re[i] * re[i] + im[i] * im[i];
    }

    m_noiseLearned = true;
}

/**
 * @brief 对含噪帧执行谱减降噪
 *
 * 1. FFT变换到频域
 * 2. 计算功率谱并减去过减因子*噪声功率谱
 * 3. 应用频谱下限防止负值
 * 4. 重建相位信息并IFFT回时域
 *
 * @param frame 含噪帧（长度应为FFT大小）
 * @return 降噪后的时域帧
 */
QVector<double> SpectralSubtract3::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(frame.size(), m_fftSize);

    if (!m_noiseLearned || n == 0) {
        m_stats.totalProcessCalls++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalProcessCalls > 0)
            ? m_timeSum / m_stats.totalProcessCalls : 0.0;
        return frame;
    }

    /* FFT变换 */
    QVector<double> re(n, 0.0), im(n, 0.0);
    for (int i = 0; i < n; ++i) {
        re[i] = frame[i];
    }
    fft(re, im);

    /* 保存原始相位 */
    QVector<double> phase(n);
    for (int i = 0; i < n; ++i) {
        phase[i] = qAtan2(im[i], re[i]);
    }

    /* 计算含噪信号功率谱 */
    double totalReduction = 0.0;
    double floorLin = qPow(10.0, m_floor / 10.0);  /* dB转线性 */

    for (int i = 0; i < n; ++i) {
        double power = re[i] * re[i] + im[i] * im[i];

        /* 谱减：减去过减后的噪声功率 */
        double noisePow = (i < m_noiseProfile.size()) ? m_noiseProfile[i] : 0.0;
        double enhanced = power - m_oversub * noisePow;

        /* 应用频谱下限 */
        double floorVal = floorLin * noisePow;
        if (enhanced < floorVal) {
            enhanced = floorVal;
        }

        if (power > 0) {
            totalReduction += 1.0 - enhanced / power;
        }

        /* 用增强后的幅度重建复数频谱 */
        double mag = qSqrt(qMax(0.0, enhanced));
        re[i] = mag * qCos(phase[i]);
        im[i] = mag * qSin(phase[i]);
    }

    /* IFFT逆变换回时域 */
    ifft(re, im);

    QVector<double> output(n);
    for (int i = 0; i < n; ++i) {
        output[i] = re[i];
    }

    m_stats.totalProcessCalls++;
    m_stats.totalFrames++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalProcessCalls > 0)
        ? m_timeSum / m_stats.totalProcessCalls : 0.0;

    double avgReduction = (n > 0) ? totalReduction / n : 0.0;
    emit processingCompleted(avgReduction);
    return output;
}

/**
 * @brief 重置统计数据
 */
void SpectralSubtract3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 基2 FFT变换（Cooley-Tukey）
 * @param re 实部数组（输入/输出）
 * @param im 虚部数组（输入/输出）
 */
void SpectralSubtract3::fft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    if (n <= 1) return;

    /* 比特反转重排 */
    int bits = 0;
    for (int temp = n; temp > 1; temp >>= 1) bits++;
    for (int i = 0; i < n; ++i) {
        int j = 0;
        int x = i;
        for (int b = 0; b < bits; ++b) { j = (j << 1) | (x & 1); x >>= 1; }
        if (j > i) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle);
        double wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];
                re[v] = re[u] - tRe;
                im[v] = im[u] - tIm;
                re[u] += tRe;
                im[u] += tIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }
}

/**
 * @brief 基2 IFFT逆变换
 * @param re 实部数组（输入/输出）
 * @param im 虚部数组（输入/输出）
 */
void SpectralSubtract3::ifft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    /* 共轭 */
    for (int i = 0; i < n; ++i) im[i] = -im[i];
    /* FFT */
    fft(re, im);
    /* 共轭并归一化 */
    for (int i = 0; i < n; ++i) {
        re[i] /= n;
        im[i] = -im[i] / n;
    }
}
