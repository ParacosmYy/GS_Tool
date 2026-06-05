/**
 * @file PhaseVocoder4.cpp
 * @brief 相位声码器实现 — STFT分析 + 相位展开 + 时间拉伸/变调 + 重叠相加
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 实现基于STFT的相位声码器，支持:
 * 1. 时间拉伸 — 不改变音调，改变信号时长
 * 2. 变调处理 — 不改变时长，改变音高
 * 3. 单帧处理 — 逐帧STFT/ISTFT处理
 *
 * 核心算法: 相位展开(phase unwrapping)保持帧间相位连续性，
 * 通过调整综合跳跃大小实现时间尺度修改。
 */

#include "utils/dsp79/PhaseVocoder4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 常量定义
// ──────────────────────────────────────────────

/** @brief 默认FFT大小 */
static constexpr int kDefaultFftSize = 2048;

/** @brief 默认跳跃大小 */
static constexpr int kDefaultHopSize = 512;

/** @brief 默认拉伸因子 */
static constexpr double kDefaultStretchFactor = 1.0;

/** @brief 2*PI常量 */
static constexpr double kTwoPi = 2.0 * M_PI;

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化相位声码器
 * @param parent 父QObject对象
 *
 * 默认参数: fftSize=2048, hopSize=512, stretchFactor=1.0
 */
PhaseVocoder4::PhaseVocoder4(QObject* parent)
    : QObject(parent)
    , m_fftSize(kDefaultFftSize)
    , m_hopSize(kDefaultHopSize)
    , m_stretchFactor(kDefaultStretchFactor)
{
    setObjectName(QStringLiteral("PhaseVocoder4"));

    // 初始化相位存储
    int fftBins = m_fftSize / 2 + 1;
    m_prevPhase.resize(fftBins, 0.0);
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置STFT参数
 *
 * 初始化相位声码器的分析参数。
 * fftSize必须大于hopSize，通常hopSize = fftSize/4。
 *
 * @param fftSize FFT窗口大小，建议为2的幂 [256, 8192]
 * @param hopSize 分析跳跃大小 [64, fftSize/2]
 * @param stretchFactor 拉伸因子，0.25~4.0
 */
void PhaseVocoder4::initialize(int fftSize, int hopSize, double stretchFactor)
{
    m_fftSize = qBound(256, fftSize, 8192);
    m_hopSize = qBound(64, hopSize, m_fftSize / 2);
    m_stretchFactor = qBound(0.25, stretchFactor, 4.0);

    // 重置相位存储
    int fftBins = m_fftSize / 2 + 1;
    m_prevPhase.resize(fftBins, 0.0);
    std::fill(m_prevPhase.begin(), m_prevPhase.end(), 0.0);
}

// ──────────────────────────────────────────────
// 时间拉伸
// ──────────────────────────────────────────────

/**
 * @brief 时间拉伸 — 不改变音调，改变信号时长
 *
 * 处理流程:
 * 1. 生成汉宁窗函数
 * 2. 对输入信号逐帧STFT（分析跳跃 = hopSize）
 * 3. 相位展开: 计算真实频率偏移并累积输出相位
 * 4. 以综合跳跃 = hopSize * factor 进行重叠相加
 * 5. 归一化并输出拉伸后的信号
 *
 * @param input 输入音频信号
 * @param factor 拉伸因子 (>1拉伸, <1压缩, =1不变)
 * @return 时间拉伸后的音频信号
 */
QVector<double> PhaseVocoder4::timeStretch(const QVector<double>& input, double factor)
{
    QElapsedTimer timer;
    timer.start();

    const int inLen = input.size();
    if (inLen == 0) {
        m_stats.totalFramesProcessed++;
        const double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesProcessed;
        return {};
    }

    factor = qBound(0.25, factor, 4.0);

    // 生成汉宁窗
    QVector<double> window = generateHannWindow(m_fftSize);

    // 计算帧数
    int numFrames = qMax(1, static_cast<int>(
        qCeil(static_cast<double>(inLen - m_fftSize) / m_hopSize)) + 1);

    // 综合跳跃 = 分析跳跃 * 拉伸因子
    int synthHop = qMax(1, static_cast<int>(qRound(m_hopSize * factor)));

    // 输出缓冲区
    int outLen = (numFrames - 1) * synthHop + m_fftSize;
    QVector<double> outBuf(outLen, 0.0);
    QVector<double> winSum(outLen, 0.0);

    // 相位存储
    int fftBins = m_fftSize / 2 + 1;
    QVector<double> prevAnalysisPhase(fftBins, 0.0);
    QVector<double> prevSynthesisPhase(fftBins, 0.0);

    // 期望的相位增量（每分析跳跃对应的频率变化）
    double omegaBase = kTwoPi * m_hopSize / m_fftSize;

    for (int frame = 0; frame < numFrames; ++frame) {
        int startSample = frame * m_hopSize;

        // 提取帧并加窗
        QVector<double> frameData(m_fftSize, 0.0);
        for (int i = 0; i < m_fftSize; ++i) {
            int idx = startSample + i;
            frameData[i] = (idx < inLen) ? input[idx] * window[i] : 0.0;
        }

        // DFT分析
        QVector<double> magnitude(fftBins, 0.0);
        QVector<double> phase(fftBins, 0.0);
        computeDFT(frameData, magnitude, phase);

        // 相位展开
        QVector<double> synthPhase(fftBins, 0.0);
        double totalPhaseCorrection = 0.0;

        for (int k = 0; k < fftBins; ++k) {
            // 计算相位差
            double dPhi = phase[k] - prevAnalysisPhase[k];

            // 减去期望的相位增量
            double expected = omegaBase * k;
            double deviation = dPhi - expected;

            // 缠绕到 [-pi, pi]
            deviation -= kTwoPi * qRound(deviation / kTwoPi);

            // 真实频率 = 期望频率 + 偏差
            double trueFreq = expected + deviation;

            // 输出相位 = 上次综合相位 + 真实频率 * factor
            synthPhase[k] = prevSynthesisPhase[k] + trueFreq * factor;
            totalPhaseCorrection += qAbs(deviation);
        }

        prevAnalysisPhase = phase;
        prevSynthesisPhase = synthPhase;

        // 逆DFT重建时域帧
        QVector<double> synthFrame(m_fftSize, 0.0);
        computeIDFT(magnitude, synthPhase, synthFrame);

        // 加窗并重叠相加
        int synthStart = frame * synthHop;
        for (int i = 0; i < m_fftSize && synthStart + i < outLen; ++i) {
            outBuf[synthStart + i] += synthFrame[i] * window[i];
            winSum[synthStart + i] += window[i] * window[i];
        }

        // 发射帧处理信号
        double avgCorrection = totalPhaseCorrection / fftBins;
        emit frameProcessed(avgCorrection);
    }

    // 归一化: 除以窗函数重叠和
    for (int i = 0; i < outLen; ++i) {
        if (winSum[i] > 1e-8) {
            outBuf[i] /= winSum[i];
        }
    }

    // 更新统计
    m_stats.totalFramesProcessed++;
    m_stats.totalStretchOperations++;
    const double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesProcessed;

    return outBuf;
}

// ──────────────────────────────────────────────
// 变调处理
// ──────────────────────────────────────────────

/**
 * @brief 变调处理 — 不改变时长，改变音高
 *
 * 实现方法: 先按 pitchRatio 进行时间拉伸，
 * 再按 1/pitchRatio 重采样恢复原始时长。
 * 音高变化 = 12 * log2(pitchRatio) 半音。
 *
 * @param input 输入音频信号
 * @param semitones 音高变化量(半音)，正=升调，负=降调
 * @return 变调后的音频信号
 */
QVector<double> PhaseVocoder4::pitchShift(const QVector<double>& input, double semitones)
{
    QElapsedTimer timer;
    timer.start();

    const int inLen = input.size();
    if (inLen == 0) {
        m_stats.totalFramesProcessed++;
        const double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesProcessed;
        return {};
    }

    // 半音 -> 频率比例
    double pitchRatio = qPow(2.0, semitones / 12.0);

    // 步骤1: 按pitchRatio进行时间拉伸（复用timeStretch，临时替换factor）
    double savedFactor = m_stretchFactor;
    m_stretchFactor = pitchRatio;
    QVector<double> stretched = timeStretch(input, pitchRatio);
    m_stretchFactor = savedFactor;

    // 步骤2: 重采样恢复原始时长
    QVector<double> output = resample(stretched, inLen);

    // 更新统计
    m_stats.totalFramesProcessed++;
    const double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesProcessed;

    return output;
}

// ──────────────────────────────────────────────
// 单帧处理
// ──────────────────────────────────────────────

/**
 * @brief 处理单帧并输出
 *
 * 对单帧数据进行DFT分析、相位展开和逆DFT重建。
 * 适用于实时流处理场景。
 *
 * @param frame 输入帧（长度应等于fftSize）
 * @return 处理后的帧数据
 */
QVector<double> PhaseVocoder4::processFrame(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = frame.size();
    if (n == 0) {
        m_stats.totalFramesProcessed++;
        const double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesProcessed;
        return {};
    }

    int fftBins = m_fftSize / 2 + 1;

    // 加窗
    QVector<double> window = generateHannWindow(m_fftSize);
    QVector<double> windowed(m_fftSize, 0.0);
    for (int i = 0; i < qMin(n, m_fftSize); ++i) {
        windowed[i] = frame[i] * window[i];
    }

    // DFT分析
    QVector<double> magnitude(fftBins, 0.0);
    QVector<double> phase(fftBins, 0.0);
    computeDFT(windowed, magnitude, phase);

    // 相位展开
    double omegaBase = kTwoPi * m_hopSize / m_fftSize;
    QVector<double> newPhase(fftBins, 0.0);
    double totalCorrection = 0.0;

    for (int k = 0; k < fftBins; ++k) {
        double dPhi = phase[k] - m_prevPhase[k];
        double expected = omegaBase * k;
        double deviation = dPhi - expected;
        deviation -= kTwoPi * qRound(deviation / kTwoPi);

        double trueFreq = expected + deviation;
        newPhase[k] = m_prevPhase[k] + trueFreq * m_stretchFactor;
        totalCorrection += qAbs(deviation);
    }

    m_prevPhase = newPhase;

    // 逆DFT
    QVector<double> output(m_fftSize, 0.0);
    computeIDFT(magnitude, newPhase, output);

    // 更新统计
    m_stats.totalFramesProcessed++;
    const double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesProcessed;

    emit frameProcessed(totalCorrection / fftBins);
    return output;
}

/**
 * @brief 重置所有累计统计信息
 */
void PhaseVocoder4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    std::fill(m_prevPhase.begin(), m_prevPhase.end(), 0.0);
}

// ──────────────────────────────────────────────
// 私有方法 — DFT/IDFT
// ──────────────────────────────────────────────

/**
 * @brief 计算离散傅里叶变换(DFT)
 *
 * 提取频域的幅度和相位信息。
 * 仅计算前 N/2+1 个频率bin（实信号对称性）。
 *
 * @param input 加窗后的时域帧（长度 = fftSize）
 * @param magnitude [out] 幅度谱
 * @param phase [out] 相位谱
 */
void PhaseVocoder4::computeDFT(const QVector<double>& input,
                                 QVector<double>& magnitude,
                                 QVector<double>& phase) const
{
    const int N = m_fftSize;
    const int bins = N / 2 + 1;

    for (int k = 0; k < bins; ++k) {
        double re = 0.0;
        double im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -kTwoPi * k * n / N;
            re += input[n] * qCos(angle);
            im += input[n] * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im);
        phase[k] = qAtan2(im, re);
    }
}

/**
 * @brief 计算逆离散傅里叶变换(IDFT)
 *
 * 从频域幅度和相位重建时域信号。
 * 使用余弦展开形式（实值输出）。
 *
 * @param magnitude 幅度谱
 * @param phase 相位谱
 * @param output [out] 重建的时域帧
 */
void PhaseVocoder4::computeIDFT(const QVector<double>& magnitude,
                                  const QVector<double>& phase,
                                  QVector<double>& output) const
{
    const int N = m_fftSize;
    const int bins = N / 2 + 1;

    for (int n = 0; n < N; ++n) {
        double val = 0.0;

        // 直流分量
        val += magnitude[0] * qCos(phase[0]);

        // 正频率分量（对称共轭，实部贡献2倍）
        for (int k = 1; k < bins - 1; ++k) {
            double angle = kTwoPi * k * n / N + phase[k];
            val += 2.0 * magnitude[k] * qCos(angle);
        }

        // Nyquist分量
        if (bins > 1) {
            val += magnitude[bins - 1] * qCos(phase[bins - 1] + M_PI * n);
        }

        output[n] = val / N;
    }
}

// ──────────────────────────────────────────────
// 私有方法 — 窗函数
// ──────────────────────────────────────────────

/**
 * @brief 生成汉宁窗函数
 *
 * w(n) = 0.5 * (1 - cos(2*pi*n / (N-1)))
 * 周期对称，适合STFT分析。
 *
 * @param size 窗长度
 * @return 汉宁窗系数数组
 */
QVector<double> PhaseVocoder4::generateHannWindow(int size) const
{
    QVector<double> win(size);
    for (int i = 0; i < size; ++i) {
        win[i] = 0.5 * (1.0 - qCos(kTwoPi * i / (size - 1)));
    }
    return win;
}

// ──────────────────────────────────────────────
// 私有方法 — 重采样
// ──────────────────────────────────────────────

/**
 * @brief 线性插值重采样
 *
 * 将信号重采样到目标长度，使用线性插值。
 *
 * @param input 输入信号
 * @param targetLen 目标长度
 * @return 重采样后的信号
 */
QVector<double> PhaseVocoder4::resample(
    const QVector<double>& input, int targetLen) const
{
    if (input.isEmpty() || targetLen <= 0) return {};

    const int inLen = input.size();
    QVector<double> output(targetLen, 0.0);
    double ratio = static_cast<double>(inLen) / targetLen;

    for (int i = 0; i < targetLen; ++i) {
        double srcPos = i * ratio;
        int idx0 = static_cast<int>(qFloor(srcPos));
        int idx1 = idx0 + 1;
        double frac = srcPos - idx0;

        idx0 = qBound(0, idx0, inLen - 1);
        idx1 = qBound(0, idx1, inLen - 1);

        output[i] = input[idx0] * (1.0 - frac) + input[idx1] * frac;
    }

    return output;
}
