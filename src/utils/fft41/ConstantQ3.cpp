/**
 * @file ConstantQ3.cpp
 * @brief 常数Q变换3 — 多分辨率时频+Chordino实现
 *
 * 实现常数Q变换(Constant Q Transform)：
 * - 对数频率间隔的滤波器组
 * - 每个八度固定bin数的核函数
 * - 多分辨率时频分析
 * - 色度(Chroma)特征提取
 * - Chordino和弦估计支持
 *
 * CQT相比FFT的核心优势在于对数频率分辨率：
 * 低频段有更精细的频率分辨，高频段有更精细的时间分辨，
 * 这与人耳感知特性高度吻合。常用于音乐信息检索(MIR)、
 * 和弦检测、音高跟踪等场景。
 *
 * 所有运算带有QElapsedTimer计时和统计信息追踪。
 */

#include "utils/fft41/ConstantQ3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 *
 * 默认参数覆盖55Hz~7040Hz（约7个八度，A1~A8），
 * 每八度12个bin（半音分辨率），44100Hz采样率。
 */
ConstantQ3::ConstantQ3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置CQT参数
 * @param minFreq 最低频率（Hz）
 * @param maxFreq 最高频率（Hz）
 * @param binsPerOctave 每八度频程数
 * @param sampleRate 采样率
 * @param threshold 核函数阈值（用于稀疏化）
 */
void ConstantQ3::setParameters(double minFreq, double maxFreq, int binsPerOctave,
                                double sampleRate, double threshold)
{
    m_minFreq = qMax(minFreq, 1.0);
    m_maxFreq = qMax(maxFreq, m_minFreq * 2.0);
    m_binsPerOctave = qMax(binsPerOctave, 1);
    m_sampleRate = qMax(sampleRate, 1.0);
    m_threshold = threshold;

    /* 计算八度数和总bin数 */
    m_octaves = (int)qCeil(qLn(m_maxFreq / m_minFreq) / qLn(2.0));
    m_totalBins = m_octaves * m_binsPerOctave;

    /* 计算FFT大小（基于最低频的Q值） */
    double Q = 1.0 / (qPow(2.0, 1.0 / m_binsPerOctave) - 1.0);
    m_fftSize = (int)qPow(2.0, qCeil(qLn(Q * m_sampleRate / m_minFreq) / qLn(2.0)));

    /* 构建频率表 */
    m_freqs.resize(m_totalBins);
    for (int i = 0; i < m_totalBins; ++i) {
        double octave = i / m_binsPerOctave;
        double bin = i % m_binsPerOctave;
        m_freqs[i] = m_minFreq * qPow(2.0, octave + bin / (double)m_binsPerOctave);
    }

    buildKernels();
}

/**
 * @brief 构建CQT核函数
 *
 * 为每个频率bin构造复指数核，应用Hann窗。
 * 核函数长度与Q值和频率成正比（低频核长，高频核短）。
 */
void ConstantQ3::buildKernels()
{
    double Q = 1.0 / (qPow(2.0, 1.0 / m_binsPerOctave) - 1.0);
    m_kernels.resize(m_totalBins);
    m_fftBins.resize(m_totalBins);

    for (int k = 0; k < m_totalBins; ++k) {
        double freq = m_freqs[k];
        int nk = (int)qRound(Q * m_sampleRate / freq);
        nk = qBound(1, nk, m_fftSize);
        m_fftBins[k] = nk;

        /* 构建复数核：Hann窗 × 复指数 */
        m_kernels[k].resize(nk * 2); /* 实部和虚部交替存储 */
        for (int n = 0; n < nk; ++n) {
            double hann = 0.5 * (1.0 - qCos(2.0 * M_PI * n / nk));
            double angle = 2.0 * M_PI * freq * n / m_sampleRate;
            m_kernels[k][2 * n] = hann * qCos(angle);     /* 实部 */
            m_kernels[k][2 * n + 1] = -hann * qSin(angle); /* 虚部 */
        }
    }
}

/**
 * @brief 执行前向常数Q变换
 * @param signal 输入信号
 * @param hopSize 跳步大小，默认512
 * @return CQT时频矩阵（每帧一个QVector<double>，长度totalBins）
 *
 * 对每帧信号，将每个CQT核与信号做点积，
 * 得到该频率bin的复数能量。
 */
QVector<QVector<double>> ConstantQ3::forward(const QVector<double>& signal, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    int numFrames = qMax(1, (signal.size() - m_fftSize) / qMax(hopSize, 1) + 1);
    QVector<QVector<double>> result(numFrames, QVector<double>(m_totalBins, 0.0));

    for (int frame = 0; frame < numFrames; ++frame) {
        int offset = frame * hopSize;
        for (int k = 0; k < m_totalBins; ++k) {
            int nk = m_fftBins[k];
            double re = 0.0, im = 0.0;

            for (int n = 0; n < nk; ++n) {
                int idx = offset + n;
                if (idx >= signal.size()) break;
                double x = signal[idx];
                re += x * m_kernels[k][2 * n];
                im += x * m_kernels[k][2 * n + 1];
            }

            /* 取模值 */
            result[frame][k] = qSqrt(re * re + im * im);
        }
    }

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.totalFrames += numFrames;
    m_stats.totalBins = m_totalBins;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(numFrames, m_totalBins);
    return result;
}

/**
 * @brief 从CQT频谱提取色度(Chroma)特征
 * @param cqSpectrum CQT时频矩阵
 * @return 色度向量（12维，对应12个半音）
 *
 * 色度特征（也称音高类特征）将所有八度中相同音名
 * 的能量折叠到12个bin中（C, C#, D, ..., B）。
 * 这种表示具有八度不变性，广泛用于和弦检测和音乐匹配。
 *
 * 如果binsPerOctave不等于12，则通过线性映射将bin对齐
 * 到最接近的半音。
 */
QVector<double> ConstantQ3::chroma(const QVector<QVector<double>>& cqSpectrum) const
{
    if (cqSpectrum.isEmpty()) return QVector<double>(12, 0.0);

    /* 累加所有帧的色度能量 */
    QVector<double> chroma(12, 0.0);
    int numFrames = cqSpectrum.size();
    for (const auto& frame : cqSpectrum) {
        for (int k = 0; k < qMin(frame.size(), m_totalBins); ++k) {
            /* 映射到色度bin（0-11，每八度12个半音） */
            int binInOctave = k % m_binsPerOctave;
            /* 如果binsPerOctave=12，直接映射；否则缩放 */
            int chromaBin = (binInOctave * 12) / m_binsPerOctave;
            chromaBin = chromaBin % 12;
            chroma[chromaBin] += frame[k];
        }
    }

    /* 按帧数平均 */
    if (numFrames > 0) {
        for (int i = 0; i < 12; ++i)
            chroma[i] /= numFrames;
    }

    /* 归一化到[0,1]范围 */
    double maxVal = *std::max_element(chroma.begin(), chroma.end());
    if (maxVal > 0.0) {
        for (int i = 0; i < 12; ++i)
            chroma[i] /= maxVal;
    }
    return chroma;
}

/**
 * @brief 获取所有bin的中心频率
 * @return 频率列表（Hz）
 */
QVector<double> ConstantQ3::frequencies() const
{
    return m_freqs;
}

/**
 * @brief 获取每个bin的品质因数Q
 * @return Q值列表
 */
QVector<double> ConstantQ3::qualityFactors() const
{
    double Q = 1.0 / (qPow(2.0, 1.0 / m_binsPerOctave) - 1.0);
    return QVector<double>(m_totalBins, Q);
}

/**
 * @brief 重置所有统计计数器
 */
void ConstantQ3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
