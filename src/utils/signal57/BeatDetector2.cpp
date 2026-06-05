/**
 * @file BeatDetector2.cpp
 * @brief 节拍检测器实现 — 起始点包络 + 自相关节拍估计
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现基于起始点包络（onset envelope）和自相关函数的节拍检测算法。
 * 先通过频域差分计算起始点强度函数，再使用自相关估计全局节拍速度（BPM），
 * 最后在起始点函数上通过动态规划定位精确的节拍时间点。
 */

#include "utils/signal57/BeatDetector2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认检测参数
 * @param parent 父QObject对象
 */
BeatDetector2::BeatDetector2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("BeatDetector2"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz），默认 44100
 */
void BeatDetector2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置节拍速度搜索范围
 *
 * 只检测此 BPM 范围内的节拍速度。
 *
 * @param minBpm 最小BPM，默认60
 * @param maxBpm 最大BPM，默认200
 */
void BeatDetector2::setTempoRange(double minBpm, double maxBpm)
{
    m_minBpm = qBound(30.0, minBpm, 300.0);
    m_maxBpm = qBound(30.0, maxBpm, 300.0);
    if (m_minBpm > m_maxBpm) {
        std::swap(m_minBpm, m_maxBpm);
    }
}

// ──────────────────────────────────────────────
// 核心检测接口
// ──────────────────────────────────────────────

/**
 * @brief 对输入信号执行节拍检测
 *
 * 处理流程：
 * 1. 计算起始点包络（onset envelope）
 * 2. 使用自相关函数估计全局节拍速度（BPM）
 * 3. 在起始点包络上定位节拍时间点
 *
 * @param signal 输入音频信号
 * @return 节拍时间点数组（秒）
 */
QVector<double> BeatDetector2::detect(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    const int n = signal.size();
    if (n < 1024) {
        return {};
    }

    // 步骤1：计算起始点包络
    QVector<double> env = onsetEnvelope(signal);

    // 步骤2：自相关估计节拍速度
    m_tempo = autoCorrelateTempo(env);

    // 步骤3：定位节拍时间点
    m_beats.clear();

    // 计算节拍间隔（采样数）
    double beatIntervalSamples = 60.0 * m_sampleRate / m_tempo;

    // 使用峰值检测 + 周期约束定位节拍
    double nextBeatSample = beatIntervalSamples * 0.5; // 从半拍开始搜索
    double searchWindow = beatIntervalSamples * 0.3;   // 搜索窗口宽度

    while (nextBeatSample < n) {
        int center = static_cast<int>(nextBeatSample);
        int lo = qMax(0, center - static_cast<int>(searchWindow));
        int hi = qMin(n - 1, center + static_cast<int>(searchWindow));

        // 在搜索窗口内找最大起始点
        double maxVal = -1e18;
        int maxIdx = center;
        for (int i = lo; i <= hi; ++i) {
            if (env[i] > maxVal) {
                maxVal = env[i];
                maxIdx = i;
            }
        }

        // 添加节拍时间点（转换为秒）
        double beatTime = maxIdx / m_sampleRate;
        m_beats.append(beatTime);

        emit beatDetected(beatTime, m_tempo);

        // 下一个预期节拍位置
        nextBeatSample = maxIdx + beatIntervalSamples;
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalDetections++;
    m_stats.totalFrames += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    return m_beats;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含检测次数、总帧数和平均耗时的Stats结构
 */
BeatDetector2::Stats BeatDetector2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void BeatDetector2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 起始点包络计算
// ──────────────────────────────────────────────

/**
 * @brief 计算起始点强度包络
 *
 * 使用短时能量差分方法：
 * 1. 将信号分帧计算每帧 RMS 能量
 * 2. 计算相邻帧间的能量差分（只取正差分，即能量上升沿）
 * 3. 平滑差分得到起始点强度函数
 *
 * @param sig 输入音频信号
 * @return 起始点强度包络（与输入等长）
 */
QVector<double> BeatDetector2::onsetEnvelope(const QVector<double>& sig)
{
    const int n = sig.size();
    const int frameSize = 1024;
    const int hopSize = 512;
    const int numFrames = (n - frameSize) / hopSize + 1;

    if (numFrames <= 1) {
        return QVector<double>(n, 0.0);
    }

    // 计算每帧 RMS 能量
    QVector<double> energy(numFrames, 0.0);
    for (int f = 0; f < numFrames; ++f) {
        double sum = 0.0;
        int start = f * hopSize;
        for (int i = 0; i < frameSize && (start + i) < n; ++i) {
            sum += sig[start + i] * sig[start + i];
        }
        energy[f] = qSqrt(sum / frameSize);
    }

    // 半波整流差分（起始点强度）
    QVector<double> onset(numFrames, 0.0);
    for (int f = 1; f < numFrames; ++f) {
        onset[f] = qMax(0.0, energy[f] - energy[f - 1]);
    }

    // 平滑
    const int smoothLen = 5;
    QVector<double> smoothed(numFrames, 0.0);
    for (int f = 0; f < numFrames; ++f) {
        double sum = 0.0;
        int count = 0;
        for (int k = -smoothLen; k <= smoothLen; ++k) {
            int idx = f + k;
            if (idx >= 0 && idx < numFrames) {
                sum += onset[idx];
                count++;
            }
        }
        smoothed[f] = (count > 0) ? sum / count : 0.0;
    }

    // 上采样到原始信号长度
    QVector<double> envelope(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double frameIdx = static_cast<double>(i) / hopSize;
        int f0 = qBound(0, static_cast<int>(frameIdx), numFrames - 1);
        int f1 = qBound(0, f0 + 1, numFrames - 1);
        double frac = frameIdx - f0;
        envelope[i] = smoothed[f0] * (1.0 - frac) + smoothed[f1] * frac;
    }

    return envelope;
}

// ──────────────────────────────────────────────
// 私有方法 — 自相关节拍估计
// ──────────────────────────────────────────────

/**
 * @brief 使用自相关函数估计节拍速度
 *
 * 在 BPM 搜索范围内，对起始点包络进行自相关。
 * 自相关峰值对应的延迟即为节拍周期，转换为 BPM。
 *
 * @param env 起始点强度包络
 * @return 估计的节拍速度（BPM）
 */
double BeatDetector2::autoCorrelateTempo(const QVector<double>& env)
{
    const int n = env.size();
    if (n < 100) return 120.0;

    // 自相关延迟对应的 BPM 范围
    int minLag = static_cast<int>(60.0 * m_sampleRate / m_maxBpm);
    int maxLag = static_cast<int>(60.0 * m_sampleRate / m_minBpm);
    minLag = qMax(1, minLag);
    maxLag = qMin(n / 2, maxLag);

    if (minLag >= maxLag) return 120.0;

    // 为了计算效率，对包络进行降采样
    const int dsRate = 16;
    QVector<double> dsEnv;
    for (int i = 0; i < n; i += dsRate) {
        dsEnv.append(env[i]);
    }
    const int dsN = dsEnv.size();

    int dsMinLag = qMax(1, minLag / dsRate);
    int dsMaxLag = qMin(dsN / 2, maxLag / dsRate);

    if (dsMinLag >= dsMaxLag) return 120.0;

    // 计算自相关
    double bestCorr = -1e18;
    int bestLag = dsMinLag;

    for (int lag = dsMinLag; lag <= dsMaxLag; ++lag) {
        double corr = 0.0;
        double norm1 = 0.0;
        double norm2 = 0.0;
        int count = 0;

        for (int i = 0; i < dsN - lag; ++i) {
            corr += dsEnv[i] * dsEnv[i + lag];
            norm1 += dsEnv[i] * dsEnv[i];
            norm2 += dsEnv[i + lag] * dsEnv[i + lag];
            count++;
        }

        double denom = qSqrt(qMax(norm1 * norm2, 1e-12));
        corr = corr / denom;

        if (corr > bestCorr) {
            bestCorr = corr;
            bestLag = lag;
        }
    }

    // 将 lag 转换为 BPM
    double actualLag = bestLag * dsRate;
    double bpm = 60.0 * m_sampleRate / actualLag;

    return qBound(m_minBpm, bpm, m_maxBpm);
}
