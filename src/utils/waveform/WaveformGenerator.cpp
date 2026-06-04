/**
 * @file WaveformGenerator.cpp
 * @brief 数学波形发生器核心实现 -- 9种波形生成+噪声叠加+归一化+二进制转换
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现正弦/方波/三角/锯齿/白噪声/粉红噪声/扫频/脉冲/直流波形,
 * 以及 addNoise / normalize 后处理方法。
 * 二进制转换与分析方法见 WaveformGeneratorExport.cpp。
 * 统计计数器方法见 WaveformGeneratorStats.cpp。
 */

#include "utils/waveform/WaveformGenerator.h"

#include <QRandomGenerator>
#include <QtMath>

#include <algorithm>
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════════
// 构造函数
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 构造波形发生器
 * @param parent 父对象,纳入QObject父子树自动管理生命周期
 */
WaveformGenerator::WaveformGenerator(QObject *parent)
    : QObject(parent)
{
}

// ═══════════════════════════════════════════════════════════════════════════════
// 核心: 统一生成入口
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 按参数结构体生成波形,并根据noiseLevel自动叠加噪声
 * @param params 生成参数(波形类型/频率/振幅/偏移/相位/采样率/采样数/噪声强度)
 * @return 采样值向量;参数无效时返回空向量
 *
 * 当 params.sampleCount <= 0 或 params.sampleRate <= 0 时返回空并递增错误计数。
 * 噪声叠加仅在 noiseLevel > 0 时执行,使用 addNoise() 高斯噪声方法。
 */
QVector<double> WaveformGenerator::generate(const GenParams &params)
{
    /* 参数校验 */
    if (params.sampleCount <= 0 || params.sampleRate <= 0.0) {
        ++m_stats.errorCount;
        return {};
    }

    QVector<double> result;

    switch (params.type) {
    case Sine:
        result = generateSine(params.frequency, params.amplitude, params.offset,
                              params.sampleRate, params.sampleCount, params.phase);
        break;
    case Square:
        result = generateSquare(params.frequency, params.amplitude, params.offset,
                                params.sampleRate, params.sampleCount, params.phase);
        break;
    case Triangle:
        result = generateTriangle(params.frequency, params.amplitude, params.offset,
                                  params.sampleRate, params.sampleCount, params.phase);
        break;
    case Sawtooth:
        result = generateSawtooth(params.frequency, params.amplitude, params.offset,
                                  params.sampleRate, params.sampleCount, params.phase);
        break;
    case WhiteNoise:
        result = generateWhiteNoise(params.amplitude, params.offset,
                                    params.sampleCount);
        break;
    case PinkNoise:
        result = generatePinkNoise(params.amplitude, params.offset,
                                   params.sampleCount);
        break;
    case Chirp:
        result = generateChirp(params.frequency, params.chirpEndFreq,
                               params.amplitude, params.offset,
                               params.sampleRate, params.sampleCount);
        break;
    case Impulse:
        result = generateImpulse(params.amplitude, params.offset,
                                 params.sampleCount);
        break;
    case DC:
        result = generateDC(params.amplitude, params.offset,
                            params.sampleCount);
        break;
    }

    /* 按需叠加噪声 */
    if (params.noiseLevel > 0.0 && !result.isEmpty()) {
        result = addNoise(result, params.noiseLevel);
    }

    /* 更新统计 */
    ++m_stats.totalGenerations;
    m_stats.totalSamples += static_cast<quint64>(result.size());

    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 波形生成: 正弦波
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 生成正弦波: y(t) = amp*sin(2*pi*f*t + phase) + offset
 */
QVector<double> WaveformGenerator::generateSine(double freqHz, double amp,
                                                double offset, double sampleRate,
                                                int count, double phase)
{
    if (count <= 0 || sampleRate <= 0.0) return {};

    QVector<double> data;
    data.reserve(count);

    const double omega = 2.0 * M_PI * freqHz;
    const double dt = 1.0 / sampleRate;

    for (int i = 0; i < count; ++i) {
        const double t = i * dt;
        data.append(amp * qSin(omega * t + phase) + offset);
    }
    return data;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 波形生成: 方波
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 生成方波: y(t) = amp*sign(sin(2*pi*f*t + phase)) + offset
 *
 * 使用三角函数周期特性判断正负半周,避免浮点 sign() 的边界抖动。
 */
QVector<double> WaveformGenerator::generateSquare(double freqHz, double amp,
                                                  double offset, double sampleRate,
                                                  int count, double phase)
{
    if (count <= 0 || sampleRate <= 0.0) return {};

    QVector<double> data;
    data.reserve(count);

    const double period = 1.0 / freqHz;
    const double dt = 1.0 / sampleRate;

    for (int i = 0; i < count; ++i) {
        const double t = i * dt;
        /* 将时间相位映射到 [0, period) 区间,判断正/负半周 */
        const double phaseInPeriod = std::fmod(t + phase / (2.0 * M_PI * freqHz), period);
        const double normalized = phaseInPeriod / period;
        data.append(amp * (normalized < 0.5 ? 1.0 : -1.0) + offset);
    }
    return data;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 波形生成: 三角波
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 生成三角波: y(t) = amp*(2*|2*(f*t + phi/(2pi))/T - floor(2*f*t+phi/(2pi)) - 1| - 1) + offset
 *
 * 简化公式: 将周期相位映射到 [-1, +1] 的线性折线。
 */
QVector<double> WaveformGenerator::generateTriangle(double freqHz, double amp,
                                                    double offset, double sampleRate,
                                                    int count, double phase)
{
    if (count <= 0 || sampleRate <= 0.0) return {};

    QVector<double> data;
    data.reserve(count);

    const double period = 1.0 / freqHz;
    const double dt = 1.0 / sampleRate;
    const double phaseOffset = phase / (2.0 * M_PI * freqHz);

    for (int i = 0; i < count; ++i) {
        const double t = i * dt;
        /* 归一化到 [0, 1) 周期位置 */
        const double pos = std::fmod((t + phaseOffset) / period, 1.0);
        /* 三角函数映射: [0,0.5]→[0,1]→[0.5,1]→[1,0] */
        const double tri = 2.0 * qFabs(2.0 * pos - 1.0) - 1.0;
        data.append(amp * tri + offset);
    }
    return data;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 波形生成: 锯齿波
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 生成锯齿波: y(t) = amp*2*(frac(f*t+phi/(2pi)) - 0.5) + offset
 *
 * frac(x) = x - floor(x), 映射到 [-1, +1) 的线性斜坡。
 */
QVector<double> WaveformGenerator::generateSawtooth(double freqHz, double amp,
                                                    double offset, double sampleRate,
                                                    int count, double phase)
{
    if (count <= 0 || sampleRate <= 0.0) return {};

    QVector<double> data;
    data.reserve(count);

    const double period = 1.0 / freqHz;
    const double dt = 1.0 / sampleRate;
    const double phaseOffset = phase / (2.0 * M_PI * freqHz);

    for (int i = 0; i < count; ++i) {
        const double t = i * dt;
        const double frac = (t + phaseOffset) / period;
        const double saw = 2.0 * (frac - qFloor(frac)) - 1.0;
        data.append(amp * saw + offset);
    }
    return data;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 波形生成: 白噪声
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 生成均匀分布白噪声: U(-amp, amp) + offset
 *
 * 使用 QRandomGenerator::global()->generateDouble() 产生 [0,1) 伪随机数。
 */
QVector<double> WaveformGenerator::generateWhiteNoise(double amp, double offset,
                                                      int count)
{
    if (count <= 0) return {};

    QVector<double> data;
    data.reserve(count);

    auto *rng = QRandomGenerator::global();
    for (int i = 0; i < count; ++i) {
        /* 映射 [0,1) → [-amp, +amp) */
        const double sample = amp * (2.0 * rng->generateDouble() - 1.0);
        data.append(sample + offset);
    }
    return data;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 波形生成: 粉红噪声 (Voss-McCartney算法)
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 生成粉红噪声(1/f噪声),使用Voss-McCartney算法
 *
 * 算法原理: 维护16行随机数,第i行每2^i个采样更新一次,
 * 每个采样点对所有行求和,产生近似1/f功率谱密度。
 * 最后归一化到 [-amp, +amp] 并加上offset。
 */
QVector<double> WaveformGenerator::generatePinkNoise(double amp, double offset,
                                                     int count)
{
    if (count <= 0) return {};

    static constexpr int NUM_ROWS = 16;  ///< 算法行数(覆盖16个倍频程)
    static constexpr double DIVISOR = static_cast<double>(NUM_ROWS + 1);

    QVector<double> data;
    data.reserve(count);

    auto *rng = QRandomGenerator::global();

    /* 初始化各行随机值 */
    double rows[NUM_ROWS];
    for (int r = 0; r < NUM_ROWS; ++r) {
        rows[r] = 2.0 * rng->generateDouble() - 1.0;
    }

    double sum = 0.0;
    for (int r = 0; r < NUM_ROWS; ++r) {
        sum += rows[r];
    }

    for (int i = 0; i < count; ++i) {
        /* 用位运算确定哪些行需要更新(最低设置的bit位) */
        int idx = i;
        if (idx == 0) {
            idx = NUM_ROWS;  /* 首次更新所有行 */
        }
        for (int r = 0; r < NUM_ROWS; ++r) {
            if (idx & (1 << r)) {
                sum -= rows[r];
                rows[r] = 2.0 * rng->generateDouble() - 1.0;
                sum += rows[r];
            }
        }

        /* 归一化到 [-1, +1] 再缩放 */
        const double sample = (sum / DIVISOR) * amp;
        data.append(sample + offset);
    }
    return data;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 波形生成: 线性扫频 (Chirp)
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 生成线性扫频信号: 频率从startFreq线性扫到endFreq
 *
 * 瞬时频率: f(t) = f0 + (f1 - f0) * t / T
 * 瞬时相位: phi(t) = 2*pi*(f0*t + (f1-f0)*t^2 / (2*T))
 * 波形公式: y(t) = amp*sin(phi(t)) + offset
 */
QVector<double> WaveformGenerator::generateChirp(double startFreq, double endFreq,
                                                 double amp, double offset,
                                                 double sampleRate, int count)
{
    if (count <= 0 || sampleRate <= 0.0) return {};

    QVector<double> data;
    data.reserve(count);

    const double duration = static_cast<double>(count - 1) / sampleRate;
    const double freqSlope = (endFreq - startFreq) / qMax(duration, 1e-12);
    const double dt = 1.0 / sampleRate;

    for (int i = 0; i < count; ++i) {
        const double t = i * dt;
        /* 线性Chirp瞬时相位积分 */
        const double phase = 2.0 * M_PI * (startFreq * t + 0.5 * freqSlope * t * t);
        data.append(amp * qSin(phase) + offset);
    }
    return data;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 波形生成: 脉冲
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 生成脉冲信号: 首采样点为amp+offset,其余为offset
 */
QVector<double> WaveformGenerator::generateImpulse(double amp, double offset,
                                                   int count)
{
    if (count <= 0) return {};

    QVector<double> data;
    data.reserve(count);

    data.append(amp + offset);
    for (int i = 1; i < count; ++i) {
        data.append(offset);
    }
    return data;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 波形生成: 直流
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 生成直流信号: 所有采样点恒等于amp + offset
 */
QVector<double> WaveformGenerator::generateDC(double amp, double offset, int count)
{
    if (count <= 0) return {};
    return QVector<double>(count, amp + offset);
}

// ═══════════════════════════════════════════════════════════════════════════════
// 后处理: 噪声叠加
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 向数据叠加高斯白噪声 (Box-Muller变换)
 * @param data 原始采样数据
 * @param level 噪声强度,表示相对于数据标准差的叠加比例
 * @return 叠加噪声后的数据副本
 *
 * 使用Box-Muller变换从均匀分布生成高斯分布噪声:
 * Z = sqrt(-2*ln(U1)) * cos(2*pi*U2), 其中U1,U2~U(0,1)。
 */
QVector<double> WaveformGenerator::addNoise(const QVector<double> &data,
                                            double level) const
{
    if (data.isEmpty() || level <= 0.0) return data;

    /* 计算原始数据的标准差作为噪声缩放基准 */
    double mean = 0.0;
    for (const double v : data) {
        mean += v;
    }
    mean /= data.size();

    double variance = 0.0;
    for (const double v : data) {
        variance += (v - mean) * (v - mean);
    }
    variance /= data.size();
    const double stdDev = qSqrt(qMax(variance, 1e-12));

    const double sigma = level * stdDev;
    auto *rng = QRandomGenerator::global();

    QVector<double> result;
    result.reserve(data.size());

    for (int i = 0; i < data.size(); i += 2) {
        /* Box-Muller: 一次产生两个高斯样本 */
        const double u1 = qMax(rng->generateDouble(), 1e-15);  /* 避免log(0) */
        const double u2 = rng->generateDouble();
        const double mag = sigma * qSqrt(-2.0 * qLn(u1));
        const double z0 = mag * qCos(2.0 * M_PI * u2);
        result.append(data[i] + z0);

        if (i + 1 < data.size()) {
            const double z1 = mag * qSin(2.0 * M_PI * u2);
            result.append(data[i + 1] + z1);
        }
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 后处理: 归一化
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 归一化数据到 [-1, +1] 范围
 * @param data 原始采样数据
 * @return 归一化后的数据副本;全零数据返回原值
 *
 * 公式: y = x / max(|x|), 若max(|x|)趋近于零则不缩放。
 */
QVector<double> WaveformGenerator::normalize(const QVector<double> &data) const
{
    if (data.isEmpty()) return data;

    double peak = 0.0;
    for (const double v : data) {
        peak = qMax(peak, qFabs(v));
    }

    if (peak < 1e-15) return data;  /* 全零或极小值,不缩放 */

    QVector<double> result;
    result.reserve(data.size());
    const double scale = 1.0 / peak;

    for (const double v : data) {
        result.append(v * scale);
    }
    return result;
}

