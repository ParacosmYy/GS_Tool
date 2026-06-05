/**
 * @file PitchTracker2.cpp
 * @brief 音高追踪器实现 — 基于自相关/YIN/倒频谱的多方法基频检测
 *
 * 实现音频信号的基频(F0)追踪，支持三种检测方法:
 * - 方法0: 自相关法(ACF) — 经典基频检测，适合稳态信号
 * - 方法1: YIN算法 — 基于差函数的改进自相关法，精度更高
 * - 方法2: 倒频谱法(CEP) — 通过对数功率谱逆FFT提取基频
 *
 * 特性:
 * - 可配置采样率、帧长、帧移
 * - 输出频率+置信度+时间戳
 * - 逐帧检测与整段追踪
 * - 统计处理帧数、检测到的音高帧数、平均耗时
 */

#include "utils/signal40/PitchTracker2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <cmath>
#include <algorithm>

/* ===== 公有方法实现 ===== */

/**
 * @brief 构造函数 — 初始化音高追踪器
 * @param parent QObject父对象
 */
PitchTracker2::PitchTracker2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)，典型值: 44100, 48000
 */
void PitchTracker2::setSampleRate(double rate)
{
    if (rate > 0.0) {
        m_sampleRate = rate;
    }
}

/**
 * @brief 设置分析帧长
 * @param size 帧长(采样点数)，建议2的幂次(如2048)
 */
void PitchTracker2::setFrameSize(int size)
{
    m_frameSize = qMax(64, size);
}

/**
 * @brief 设置帧移(相邻帧之间的间隔)
 * @param hop 帧移(采样点数)，典型值 = 帧长/4
 */
void PitchTracker2::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

/**
 * @brief 设置音高检测方法
 * @param method 检测方法: 0=自相关, 1=YIN, 2=倒频谱
 */
void PitchTracker2::setMethod(int method)
{
    m_method = qBound(0, method, 2);
}

/**
 * @brief 追踪整段音频的音高变化
 *
 * 将音频按帧移分帧，逐帧检测基频，返回所有帧的音高信息。
 *
 * @param audio 完整音频信号
 * @return 每帧的音高检测结果(频率、置信度、时间戳)
 */
QList<PitchTracker2::Pitch> PitchTracker2::track(const QVector<double>& audio)
{
    QElapsedTimer timer;
    timer.start();

    QList<Pitch> result;

    if (audio.isEmpty() || audio.size() < m_frameSize) {
        m_stats.totalFrames++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            (m_stats.totalFrames > 0) ? m_timeSum / m_stats.totalFrames : 0.0;
        return result;
    }

    /* 逐帧处理 */
    int frameCount = (audio.size() - m_frameSize) / m_hopSize + 1;

    for (int f = 0; f < frameCount; ++f) {
        int start = f * m_hopSize;

        /* 提取当前帧 */
        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize; ++i) {
            frame[i] = audio[start + i];
        }

        /* 检测当前帧的基频 */
        double freq = detectFrame(frame);

        /* 计算置信度(基于帧能量) */
        double energy = 0.0;
        for (int i = 0; i < m_frameSize; ++i) {
            energy += frame[i] * frame[i];
        }
        double rms = qSqrt(energy / m_frameSize);
        double confidence = qMin(1.0, rms * 10.0);  /* RMS归一化为置信度 */

        /* 构造音高结果 */
        Pitch pitch;
        pitch.frequency = freq;
        pitch.confidence = (freq > 0.0) ? confidence : 0.0;
        pitch.time = start / m_sampleRate;

        result.append(pitch);

        /* 更新统计 */
        m_stats.totalFrames++;
        if (freq > 0.0) {
            m_stats.totalPitchesDetected++;
        }

        emit frameProcessed(f, freq, pitch.confidence);
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalFrames > 0) ? m_timeSum / m_stats.totalFrames : 0.0;

    return result;
}

/**
 * @brief 检测单帧音频的基频
 *
 * 根据当前设置的方法(m_method)选择检测算法:
 * - 0: 自相关法 — 在滞后区间找最大相关峰
 * - 1: YIN法 — 差函数+累积均值归一化
 * - 2: 倒频谱法 — 对数谱的IFFT峰值
 *
 * @param frame 音频帧数据
 * @return 检测到的基频(Hz)，0表示未检测到
 */
double PitchTracker2::detectFrame(const QVector<double>& frame)
{
    if (frame.size() < m_frameSize) return 0.0;

    double freq = 0.0;

    switch (m_method) {
    case 0:
        freq = detectACF(frame);
        break;
    case 1:
        freq = detectYIN(frame);
        break;
    case 2:
        freq = detectCepstrum(frame);
        break;
    default:
        freq = detectACF(frame);
        break;
    }

    return freq;
}

/**
 * @brief 重置统计信息
 */
void PitchTracker2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ===== 私有方法实现 ===== */

/**
 * @brief 自相关法检测基频
 *
 * 计算信号的短时自相关函数，在基频搜索范围内找最大峰值。
 * 搜索范围由最小/最大频率决定:
 * - minLag = sampleRate / maxFreq (默认500Hz)
 * - maxLag = sampleRate / minFreq (默认50Hz)
 *
 * @param frame 音频帧
 * @return 基频(Hz)
 */
double PitchTracker2::detectACF(const QVector<double>& frame) const
{
    const int n = frame.size();

    /* 基频搜索范围 */
    const double minFreq = 50.0;
    const double maxFreq = 500.0;
    const int minLag = qMax(2, static_cast<int>(m_sampleRate / maxFreq));
    const int maxLag = qMin(n / 2, static_cast<int>(m_sampleRate / minFreq));

    /* 计算自相关 */
    QVector<double> acf(maxLag + 1, 0.0);
    for (int lag = 0; lag <= maxLag; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < n - lag; ++i) {
            sum += frame[i] * frame[i + lag];
        }
        acf[lag] = sum;
    }

    /* 在搜索范围内找最大峰值 */
    double maxVal = 0.0;
    int bestLag = 0;
    for (int lag = minLag; lag <= maxLag; ++lag) {
        if (acf[lag] > maxVal) {
            maxVal = acf[lag];
            bestLag = lag;
        }
    }

    /* 阈值检查 — 峰值必须大于零延迟自相关的一定比例 */
    if (bestLag > 0 && maxVal > acf[0] * 0.3) {
        return m_sampleRate / bestLag;
    }

    return 0.0;
}

/**
 * @brief YIN算法检测基频
 *
 * YIN算法步骤:
 * 1. 计算差函数 d(t) = sum((x[i] - x[i+t])^2)
 * 2. 累积均值归一化差函数 d'(t) = d(t) / ((1/t) * sum(d(j)))
 * 3. 找到d'低于阈值的谷值(绝对阈值=0.15)
 * 4. 抛物线插值精确化基频估计
 *
 * @param frame 音频帧
 * @return 基频(Hz)
 */
double PitchTracker2::detectYIN(const QVector<double>& frame) const
{
    const int n = frame.size();
    const int halfN = n / 2;

    const double minFreq = 50.0;
    const double maxFreq = 500.0;
    const int minLag = qMax(2, static_cast<int>(m_sampleRate / maxFreq));
    const int maxLag = qMin(halfN, static_cast<int>(m_sampleRate / minFreq));

    /* 步骤1: 差函数 */
    QVector<double> diff(maxLag + 1, 0.0);
    for (int lag = 0; lag <= maxLag; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < halfN; ++i) {
            double d = frame[i] - frame[i + lag];
            sum += d * d;
        }
        diff[lag] = sum;
    }

    /* 步骤2: 累积均值归一化 */
    QVector<double> cmndf(maxLag + 1, 0.0);
    cmndf[0] = 1.0;
    double runningSum = 0.0;
    for (int lag = 1; lag <= maxLag; ++lag) {
        runningSum += diff[lag];
        cmndf[lag] = (runningSum > 0.0) ? diff[lag] * lag / runningSum : 1.0;
    }

    /* 步骤3: 找到低于阈值的第一个谷值 */
    const double threshold = 0.15;
    int bestLag = 0;
    for (int lag = minLag; lag <= maxLag; ++lag) {
        if (cmndf[lag] < threshold) {
            /* 找到谷底 */
            while (lag + 1 <= maxLag && cmndf[lag + 1] < cmndf[lag]) {
                lag++;
            }
            bestLag = lag;
            break;
        }
    }

    /* 如果没找到，取全局最小 */
    if (bestLag == 0) {
        double minVal = 1e10;
        for (int lag = minLag; lag <= maxLag; ++lag) {
            if (cmndf[lag] < minVal) {
                minVal = cmndf[lag];
                bestLag = lag;
            }
        }
        if (minVal > 0.5) return 0.0;  /* 置信度不足 */
    }

    /* 步骤4: 抛物线插值精化 */
    if (bestLag > 1 && bestLag < maxLag) {
        double s0 = cmndf[bestLag - 1];
        double s1 = cmndf[bestLag];
        double s2 = cmndf[bestLag + 1];
        double shift = (s0 - s2) / (2.0 * (s0 - 2.0 * s1 + s2));
        if (qAbs(shift) < 1.0) {
            bestLag = static_cast<int>(bestLag + shift);
        }
    }

    return (bestLag > 0) ? m_sampleRate / bestLag : 0.0;
}

/**
 * @brief 倒频谱法检测基频
 *
 * 倒频谱 = IFFT(log(|FFT(x)|^2))
 * 基频对应于倒频谱中的峰值位置。
 *
 * @param frame 音频帧
 * @return 基频(Hz)
 */
double PitchTracker2::detectCepstrum(const QVector<double>& frame) const
{
    const int n = frame.size();
    const int halfN = n / 2;

    /* 步骤1: 计算功率谱 */
    QVector<double> powerSpectrum(halfN, 0.0);
    for (int k = 0; k < halfN; ++k) {
        double real = 0.0, imag = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = 2.0 * M_PI * k * i / n;
            real += frame[i] * qCos(angle);
            imag -= frame[i] * qSin(angle);
        }
        powerSpectrum[k] = real * real + imag * imag;
    }

    /* 步骤2: 取对数 */
    QVector<double> logSpectrum(halfN, 0.0);
    for (int k = 0; k < halfN; ++k) {
        logSpectrum[k] = qLn(qMax(powerSpectrum[k], 1e-30));
    }

    /* 步骤3: 对对数功率谱做DFT(倒频谱) */
    const double minFreq = 50.0;
    const double maxFreq = 500.0;
    const int minQuefrency = qMax(1, static_cast<int>(m_sampleRate / maxFreq));
    const int maxQuefrency = qMin(halfN / 2, static_cast<int>(m_sampleRate / minFreq));

    QVector<double> cepstrum(maxQuefrency + 1, 0.0);
    for (int q = minQuefrency; q <= maxQuefrency; ++q) {
        double sum = 0.0;
        for (int k = 0; k < halfN; ++k) {
            sum += logSpectrum[k] * qCos(2.0 * M_PI * k * q / halfN);
        }
        cepstrum[q] = sum;
    }

    /* 步骤4: 在搜索范围内找峰值 */
    double maxVal = -1e10;
    int bestQuefrency = 0;
    for (int q = minQuefrency; q <= maxQuefrency; ++q) {
        if (cepstrum[q] > maxVal) {
            maxVal = cepstrum[q];
            bestQuefrency = q;
        }
    }

    if (bestQuefrency > 0) {
        return m_sampleRate / bestQuefrency;
    }

    return 0.0;
}
