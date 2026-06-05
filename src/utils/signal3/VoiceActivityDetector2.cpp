/**
 * @file VoiceActivityDetector2.cpp
 * @brief VoiceActivityDetector2 实现 — 基于噪声估计与SNR的语音活动检测
 *
 * 算法流程:
 * 1. 计算每帧RMS能量
 * 2. 非语音帧用于指数移动平均更新噪声估计
 * 3. SNR = 10*log10(frameEnergy / noiseLevel)
 * 4. 判定: energy > noiseLevel * threshold 或 SNR > snrThreshold → 语音
 * 5. hangover帧: 语音结束后保留N帧，避免尾部截断
 */

#include "utils/signal3/VoiceActivityDetector2.h"

#include <QtMath>
#include <algorithm>

// ── 常量定义 ──

/** @brief 噪声估计平滑系数(越小更新越慢，噪声估计越稳定) */
static constexpr double NOISE_ALPHA = 0.95;

/** @brief 初始噪声估计值(避免启动时零除) */
static constexpr double INITIAL_NOISE_FLOOR = 1e-6;

/** @brief SNR判定门限(dB) */
static constexpr double SNR_THRESHOLD_DB = 3.0;

/** @brief 最小能量值(防止log(0)) */
static constexpr double MIN_ENERGY = 1e-12;

// ── 构造 / 析构 ──

/**
 * @brief 构造函数，初始化检测参数和内部状态
 * @param frameSize  每帧采样点数
 * @param sampleRate 采样率
 * @param parent     父QObject
 */
VoiceActivityDetector2::VoiceActivityDetector2(int frameSize, int sampleRate,
                                               QObject* parent)
    : QObject(parent)
    , m_frameSize(qMax(64, frameSize))
    , m_sampleRate(qMax(8000, sampleRate))
    , m_noiseThreshold(2.0)
    , m_hangoverFrames(10)
    , m_hangoverRemaining(0)
    , m_noiseLevel(INITIAL_NOISE_FLOOR)
    , m_snr(0.0)
    , m_lastEnergy(0.0)
    , m_wasVoiced(false)
{
    setObjectName(QStringLiteral("VoiceActivityDetector2"));
    m_timer.start();
}

VoiceActivityDetector2::~VoiceActivityDetector2() = default;

// ── 核心检测 ──

/**
 * @brief 处理一帧音频数据
 *
 * 1. 计算帧RMS能量
 * 2. 基于能量阈值和SNR判定语音活动
 * 3. 非语音帧用于更新噪声估计
 * 4. hangover机制延迟语音结束判定
 * 5. 检测语音状态变化并发射相应信号
 * @param frame 音频帧采样数据
 * @return true = 语音帧, false = 静音/噪声帧
 */
bool VoiceActivityDetector2::process(const QVector<double>& frame)
{
    m_timer.restart();

    /* 步骤1: 计算帧RMS能量 */
    double energy = computeEnergy(frame);
    m_lastEnergy = energy;

    /* 步骤2: 计算SNR */
    double snrLinear = energy / qMax(m_noiseLevel, MIN_ENERGY);
    m_snr = 10.0 * qLn(qMax(snrLinear, MIN_ENERGY)) / qLn(10.0);

    /* 步骤3: 综合判定语音活动 */
    bool energyVoiced = (energy > m_noiseLevel * m_noiseThreshold);
    bool snrVoiced = (m_snr > SNR_THRESHOLD_DB);
    bool isVoiced = energyVoiced || snrVoiced;

    /* 步骤4: hangover机制 — 语音结束后延迟N帧才切换为静音 */
    if (isVoiced) {
        m_hangoverRemaining = m_hangoverFrames;
    } else if (m_hangoverRemaining > 0) {
        --m_hangoverRemaining;
        isVoiced = true; // hangover期间保持语音状态
    }

    /* 步骤5: 更新噪声估计(仅非语音帧参与) */
    updateNoiseEstimate(energy, isVoiced);

    /* 步骤6: 检测状态变化并发射信号 */
    if (isVoiced && !m_wasVoiced) {
        emit voiceStarted();
    } else if (!isVoiced && m_wasVoiced) {
        emit voiceStopped();
    }
    m_wasVoiced = isVoiced;

    /* 步骤7: 更新统计 */
    ++m_stats.totalFrames;
    if (isVoiced) {
        ++m_stats.voicedFrames;
    }

    double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6; // ns→ms
    quint64 n = m_stats.totalFrames;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * static_cast<double>(n - 1) / static_cast<double>(n)
        + elapsed / static_cast<double>(n);

    emit frameProcessed(isVoiced, energy, m_snr);
    return isVoiced;
}

// ── 参数配置 ──

/**
 * @brief 设置噪声门限倍数
 * @param threshold 门限倍数(需 > 0)
 */
void VoiceActivityDetector2::setNoiseThreshold(double threshold)
{
    m_noiseThreshold = qMax(0.1, threshold);
}

/**
 * @brief 设置hangover帧数
 * @param frames 延迟帧数(需 >= 0)
 */
void VoiceActivityDetector2::setHangoverFrames(int frames)
{
    m_hangoverFrames = qMax(0, frames);
}

// ── 状态查询 ──

/** @brief 获取当前噪声估计水平 */
double VoiceActivityDetector2::noiseLevel() const
{
    return m_noiseLevel;
}

/** @brief 获取最近一帧的SNR(dB) */
double VoiceActivityDetector2::snr() const
{
    return m_snr;
}

/** @brief 获取帧大小 */
int VoiceActivityDetector2::frameSize() const
{
    return m_frameSize;
}

/** @brief 获取采样率 */
int VoiceActivityDetector2::sampleRate() const
{
    return m_sampleRate;
}

// ── 统计 ──

/**
 * @brief 获取当前统计数据快照
 * @return Stats结构体副本
 */
VoiceActivityDetector2::Stats VoiceActivityDetector2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计计数器和内部检测状态
 */
void VoiceActivityDetector2::resetStatistics()
{
    m_stats = Stats{};
    m_noiseLevel = INITIAL_NOISE_FLOOR;
    m_snr = 0.0;
    m_lastEnergy = 0.0;
    m_wasVoiced = false;
    m_hangoverRemaining = 0;
}

// ── 私有方法 ──

/**
 * @brief 计算帧RMS能量
 *
 * RMS = sqrt(sum(x^2) / N)，衡量帧内信号幅度。
 * @param frame 音频帧数据
 * @return RMS能量值
 */
double VoiceActivityDetector2::computeEnergy(const QVector<double>& frame) const
{
    if (frame.isEmpty()) {
        return 0.0;
    }

    double sumSq = 0.0;
    int count = qMin(frame.size(), m_frameSize);
    for (int i = 0; i < count; ++i) {
        sumSq += frame[i] * frame[i];
    }
    return qSqrt(sumSq / static_cast<double>(count));
}

/**
 * @brief 更新噪声估计
 *
 * 使用指数移动平均(EMA): noise = alpha * noise + (1-alpha) * energy
 * 仅在非语音帧时更新，避免语音能量污染噪声估计。
 * @param energy   当前帧能量
 * @param isVoiced 当前帧是否为语音
 */
void VoiceActivityDetector2::updateNoiseEstimate(double energy, bool isVoiced)
{
    if (!isVoiced) {
        m_noiseLevel = NOISE_ALPHA * m_noiseLevel
                     + (1.0 - NOISE_ALPHA) * qMax(energy, MIN_ENERGY);
    }
}
