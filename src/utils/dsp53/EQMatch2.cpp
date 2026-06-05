/**
 * @file EQMatch2.cpp
 * @brief EQ匹配处理器实现，自动计算均衡曲线使目标频谱匹配参考频谱
 *
 * 通过FFT分析比较参考信号和目标信号的频谱差异，
 * 计算所需的EQ校正曲线。适用于音频系统校准、
 * 扬声器响应匹配等场景。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/dsp53/EQMatch2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
EQMatch2::EQMatch2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率，单位Hz，默认44100
 */
void EQMatch2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置FFT窗口大小
 * @param n FFT大小，应为2的幂次
 */
void EQMatch2::setFFTSize(int n)
{
    m_fftSize = qMax(16, n);
    m_eqCurve.clear();
}

/**
 * @brief 设置参考信号频谱
 *
 * 对参考信号进行FFT分析并保存幅度谱作为匹配目标。
 * 使用Hann窗减少频谱泄漏。
 *
 * @param ref 参考信号时域样本
 */
void EQMatch2::setReference(const QVector<double>& ref)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(m_fftSize, ref.size());

    QVector<double> re(m_fftSize, 0.0);
    QVector<double> im(m_fftSize, 0.0);

    /* 应用Hann窗 */
    for (int i = 0; i < n; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_fftSize));
        re[i] = ref[i] * w;
    }

    fft(re, im);

    /* 计算幅度谱(dB) */
    m_refSpectrum.resize(m_fftSize / 2 + 1);
    for (int i = 0; i < m_refSpectrum.size(); ++i) {
        double mag = qSqrt(re[i] * re[i] + im[i] * im[i]);
        m_refSpectrum[i] = 20.0 * qLn(qMax(1e-10, mag)) / qLn(10.0);
    }

    m_refSet = true;
    m_eqCurve.clear();

    Q_UNUSED(timer);
}

/**
 * @brief 对目标信号进行EQ匹配
 *
 * 比较目标信号频谱与参考频谱，计算所需的EQ校正曲线。
 * EQ曲线为目标频谱相对于参考频谱的差值(dB)。
 *
 * @param target 待匹配的目标信号时域样本
 * @return EQ校正曲线(dB)，长度为FFTSize/2+1
 */
QVector<double> EQMatch2::match(const QVector<double>& target)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_refSet) {
        return {};
    }

    int n = qMin(m_fftSize, target.size());

    QVector<double> re(m_fftSize, 0.0);
    QVector<double> im(m_fftSize, 0.0);

    /* 应用Hann窗 */
    for (int i = 0; i < n; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_fftSize));
        re[i] = target[i] * w;
    }

    fft(re, im);

    /* 计算目标幅度谱(dB) */
    int bins = m_fftSize / 2 + 1;
    QVector<double> targetSpec(bins);
    for (int i = 0; i < bins; ++i) {
        double mag = qSqrt(re[i] * re[i] + im[i] * im[i]);
        targetSpec[i] = 20.0 * qLn(qMax(1e-10, mag)) / qLn(10.0);
    }

    /* 计算EQ曲线：参考 - 目标 */
    m_eqCurve.resize(bins);
    double totalError = 0.0;
    for (int i = 0; i < bins; ++i) {
        m_eqCurve[i] = m_refSpectrum[i] - targetSpec[i];
        /* 平滑处理：限制最大增益范围 */
        m_eqCurve[i] = qBound(-24.0, m_eqCurve[i], 24.0);
        totalError += qAbs(m_eqCurve[i]);
    }

    /* 对EQ曲线进行移动平均平滑 */
    const int smoothWidth = 5;
    QVector<double> smoothed(bins);
    for (int i = 0; i < bins; ++i) {
        double sum = 0.0;
        int count = 0;
        for (int j = -smoothWidth; j <= smoothWidth; ++j) {
            int idx = i + j;
            if (idx >= 0 && idx < bins) {
                sum += m_eqCurve[idx];
                ++count;
            }
        }
        smoothed[i] = sum / count;
    }
    m_eqCurve = smoothed;

    /* 更新统计 */
    m_stats.totalMatches++;
    m_stats.totalFrames++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    double avgError = totalError / bins;
    emit matchCompleted(avgError);

    return m_eqCurve;
}

/**
 * @brief 原地FFT实现（Cooley-Tukey迭代算法）
 * @param re 实部数组，同时作为输入和输出
 * @param im 虚部数组，同时作为输入和输出
 */
void EQMatch2::fft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();

    /* 位逆序排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
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

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle);
        double wIm = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uRe = re[i + j];
                double uIm = im[i + j];
                double vRe = curRe * re[i + j + len / 2] - curIm * im[i + j + len / 2];
                double vIm = curRe * im[i + j + len / 2] + curIm * re[i + j + len / 2];

                re[i + j] = uRe + vRe;
                im[i + j] = uIm + vIm;
                re[i + j + len / 2] = uRe - vRe;
                im[i + j + len / 2] = uIm - vIm;

                double newCurRe = curRe * wRe - curIm * wIm;
                double newCurIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
                curIm = newCurIm;
            }
        }
    }
}

/**
 * @brief 重置所有统计数据
 */
void EQMatch2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
