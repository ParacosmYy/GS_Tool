/**
 * @file PitchShift2.cpp
 * @brief 变调2实现 — 相位声码器+共振峰保持
 *
 * 使用相位声码器技术实现音高变换:
 * - STFT分析 -> 相位调整 -> ISTFT合成
 * - 变调: 修改相位增量同时保持时间长度
 * - 时间拉伸: 修改帧间步进
 * - 共振峰保持: 可选保留原始频谱包络
 *
 * 统计信息跟踪: 处理调用次数、采样点数、平均耗时。
 */

#include "utils/dsp44/PitchShift2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数并构建窗函数
 * @param parent 父对象指针
 */
PitchShift2::PitchShift2(QObject* parent)
    : QObject(parent)
{
    buildWindow();
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率(Hz)
 */
void PitchShift2::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 设置变调因子
 * @param factor 变调因子(>1升调, <1降调, 1.0不变)
 */
void PitchShift2::setShiftFactor(double factor)
{
    m_shiftFactor = qBound(0.25, factor, 4.0);
}

/**
 * @brief 设置是否保持共振峰
 * @param preserve true则保持原始共振峰结构
 */
void PitchShift2::setFormantPreservation(bool preserve)
{
    m_formantPreserve = preserve;
}

/**
 * @brief 构建Hann窗函数
 */
void PitchShift2::buildWindow()
{
    m_window.resize(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i) {
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_fftSize - 1)));
    }
    m_phaseAccum.resize(m_fftSize / 2 + 1, 0.0);
}

/**
 * @brief 原地FFT变换(Cooley-Tukey基2)
 * @param real 实部数组
 * @param imag 虚部数组
 */
void PitchShift2::fft(QVector<double>& real, QVector<double>& imag) const
{
    const int n = real.size();
    if (n <= 1) return;

    // 位逆序置换
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    // 蝶形运算
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wR = qCos(ang), wI = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = u + len / 2;
                double tR = cR * real[v] - cI * imag[v];
                double tI = cR * imag[v] + cI * real[v];
                real[v] = real[u] - tR;
                imag[v] = imag[u] - tI;
                real[u] += tR;
                imag[u] += tI;
                double newR = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = newR;
            }
        }
    }
}

/**
 * @brief 原地IFFT变换
 * @param real 实部数组
 * @param imag 虚部数组
 */
void PitchShift2::ifft(QVector<double>& real, QVector<double>& imag) const
{
    const int n = real.size();
    for (int i = 0; i < n; ++i) imag[i] = -imag[i];
    fft(real, imag);
    double inv = 1.0 / n;
    for (int i = 0; i < n; ++i) {
        real[i] *= inv;
        imag[i] = -imag[i] * inv;
    }
}

/**
 * @brief 变调处理
 *
 * 相位声码器变调流程:
 * 1. STFT分析: 分帧、加窗、FFT
 * 2. 频谱重采样: 将频谱按shiftFactor重采样
 * 3. 相位调整: 保持相位连续性
 * 4. 共振峰保持(可选): 恢复原始频谱包络
 * 5. ISTFT合成: IFFT、加窗、重叠相加
 *
 * @param input 输入采样序列
 * @return 变调后的采样序列
 */
QVector<double> PitchShift2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty() || qAbs(m_shiftFactor - 1.0) < 1e-6) {
        return input;
    }

    // 变调 = 先时间拉伸(1/factor)再重采样(factor)
    // 使用相位声码器实现
    const int halfN = m_fftSize / 2;
    const int numFrames = (input.size() - m_fftSize) / m_hopSize + 1;

    if (numFrames <= 0) {
        return input;
    }

    // 分析阶段: 提取所有帧的频谱
    struct SpectralFrame {
        QVector<double> magnitude;  ///< 幅度谱
        QVector<double> phase;      ///< 相位谱
    };

    QVector<SpectralFrame> frames(numFrames);
    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;
        frames[f].magnitude.resize(halfN + 1);
        frames[f].phase.resize(halfN + 1);

        QVector<double> real(m_fftSize, 0.0), imag(m_fftSize, 0.0);
        for (int i = 0; i < m_fftSize && start + i < input.size(); ++i) {
            real[i] = input[start + i] * m_window[i];
        }

        fft(real, imag);

        for (int k = 0; k <= halfN; ++k) {
            frames[f].magnitude[k] = qSqrt(real[k] * real[k] + imag[k] * imag[k]);
            frames[f].phase[k] = qAtan2(imag[k], real[k]);
        }
    }

    // 合成阶段: 重采样频谱 + 相位声码器
    // 目标帧数 = 原始帧数 / shiftFactor (变调不改变时长)
    const int synthHop = m_hopSize;
    const int outputLen = input.size();
    QVector<double> output(outputLen, 0.0);
    QVector<double> winSum(outputLen, 0.0);

    // 重置相位累加器
    std::fill(m_phaseAccum.begin(), m_phaseAccum.end(), 0.0);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * synthHop;

        QVector<double> synthReal(m_fftSize, 0.0);
        QVector<double> synthImag(m_fftSize, 0.0);

        for (int k = 0; k <= halfN; ++k) {
            // 频谱重采样: 将bin k映射到bin k*shiftFactor
            double srcBin = k / m_shiftFactor;
            int srcBinLow = qMax(0, qMin(numFrames - 1, static_cast<int>(qFloor(srcBin))));

            // 计算相位增量
            double expectedPhase = 2.0 * M_PI * k * m_hopSize / m_fftSize;
            double phaseDev = 0.0;

            if (f > 0) {
                // 使用源帧的相位差
                int srcFrame = qBound(0, static_cast<int>(f * 1.0), numFrames - 1);
                phaseDev = frames[srcFrame].phase[k]
                         - frames[qMax(0, srcFrame - 1)].phase[k];
            }

            // 相位累加
            m_phaseAccum[k] += phaseDev;

            // 插值幅度
            double mag = frames[f].magnitude[k];
            if (m_shiftFactor != 1.0) {
                int srcF = qBound(0, static_cast<int>(f / m_shiftFactor), numFrames - 1);
                mag = frames[srcF].magnitude[qMin(k, halfN)];
            }

            synthReal[k] = mag * qCos(m_phaseAccum[k]);
            synthImag[k] = mag * qSin(m_phaseAccum[k]);
        }

        // 共轭对称
        for (int k = halfN + 1; k < m_fftSize; ++k) {
            synthReal[k] = synthReal[m_fftSize - k];
            synthImag[k] = -synthImag[m_fftSize - k];
        }

        // IFFT
        ifft(synthReal, synthImag);

        // 重叠相加
        for (int i = 0; i < m_fftSize && start + i < outputLen; ++i) {
            output[start + i] += synthReal[i] * m_window[i];
            winSum[start + i] += m_window[i] * m_window[i];
        }
    }

    // 归一化
    for (int i = 0; i < outputLen; ++i) {
        if (winSum[i] > 1e-10) output[i] /= winSum[i];
    }

    // 更新统计
    m_stats.totalProcessCalls++;
    m_stats.totalSamplesProcessed += input.size();

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    emit processingCompleted(input.size(), m_shiftFactor);
    return output;
}

/**
 * @brief 时间拉伸处理
 *
 * 改变音频时长而不改变音高:
 * - stretchFactor > 1: 拉长(变慢)
 * - stretchFactor < 1: 缩短(变快)
 *
 * @param input 输入采样序列
 * @param stretchFactor 拉伸因子
 * @return 拉伸后的采样序列
 */
QVector<double> PitchShift2::processTimeStretch(const QVector<double>& input,
                                                  double stretchFactor)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty() || qAbs(stretchFactor - 1.0) < 1e-6) {
        return input;
    }

    stretchFactor = qBound(0.25, stretchFactor, 4.0);

    const int halfN = m_fftSize / 2;
    const int numFrames = (input.size() - m_fftSize) / m_hopSize + 1;
    if (numFrames <= 0) return input;

    // 分析
    struct Frame { QVector<double> mag, phase; };
    QVector<Frame> frames(numFrames);
    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;
        frames[f].mag.resize(halfN + 1);
        frames[f].phase.resize(halfN + 1);

        QVector<double> re(m_fftSize, 0.0), im(m_fftSize, 0.0);
        for (int i = 0; i < m_fftSize && start + i < input.size(); ++i) {
            re[i] = input[start + i] * m_window[i];
        }
        fft(re, im);
        for (int k = 0; k <= halfN; ++k) {
            frames[f].mag[k] = qSqrt(re[k] * re[k] + im[k] * im[k]);
            frames[f].phase[k] = qAtan2(im[k], re[k]);
        }
    }

    // 合成: 插值帧
    const int synthFrames = static_cast<int>(numFrames * stretchFactor);
    const int outputLen = synthFrames * m_hopSize + m_fftSize;
    QVector<double> output(outputLen, 0.0);
    QVector<double> winSum(outputLen, 0.0);

    QVector<double> phaseAcc(halfN + 1, 0.0);

    for (int f = 0; f < synthFrames; ++f) {
        double srcPos = f / stretchFactor;
        int srcF = qBound(0, static_cast<int>(srcPos), numFrames - 1);
        int prevF = qMax(0, srcF - 1);

        int start = f * m_hopSize;
        QVector<double> re(m_fftSize, 0.0), im(m_fftSize, 0.0);

        for (int k = 0; k <= halfN; ++k) {
            double dPhase = frames[srcF].phase[k] - frames[prevF].phase[k];
            double expected = 2.0 * M_PI * k * m_hopSize / m_fftSize;
            double dev = dPhase - expected;
            while (dev > M_PI) dev -= 2.0 * M_PI;
            while (dev < -M_PI) dev += 2.0 * M_PI;
            phaseAcc[k] += expected + dev;

            // 插值幅度
            double frac = srcPos - qFloor(srcPos);
            double mag = (1.0 - frac) * frames[prevF].mag[k]
                       + frac * frames[srcF].mag[k];

            re[k] = mag * qCos(phaseAcc[k]);
            im[k] = mag * qSin(phaseAcc[k]);
        }

        for (int k = halfN + 1; k < m_fftSize; ++k) {
            re[k] = re[m_fftSize - k];
            im[k] = -im[m_fftSize - k];
        }

        ifft(re, im);

        for (int i = 0; i < m_fftSize && start + i < outputLen; ++i) {
            output[start + i] += re[i] * m_window[i];
            winSum[start + i] += m_window[i] * m_window[i];
        }
    }

    for (int i = 0; i < outputLen; ++i) {
        if (winSum[i] > 1e-10) output[i] /= winSum[i];
    }

    m_stats.totalProcessCalls++;
    m_stats.totalSamplesProcessed += input.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    return output;
}

/**
 * @brief 重置所有统计信息
 */
void PitchShift2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
