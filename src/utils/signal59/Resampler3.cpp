/**
 * @file Resampler3.cpp
 * @brief 重采样器实现 — sinc 插值重采样
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现基于 sinc 插值的高质量重采样器。
 * 支持任意采样率转换，使用加窗 sinc 滤波器
 * 避免频域混叠和成像失真。
 */

#include "utils/signal59/Resampler3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认重采样参数
 * @param parent 父QObject对象
 */
Resampler3::Resampler3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("Resampler3"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置源采样率
 * @param sr 源采样率（Hz）
 */
void Resampler3::setSourceRate(double sr)
{
    m_source = qMax(1.0, sr);
}

/**
 * @brief 设置目标采样率
 * @param tr 目标采样率（Hz）
 */
void Resampler3::setTargetRate(double tr)
{
    m_target = qMax(1.0, tr);
}

/**
 * @brief 设置重采样质量
 *
 * 质量级别影响滤波器长度：
 * - 1: 快速（短滤波器，低质量）
 * - 5: 标准（中等滤波器）
 * - 10: 高质量（长滤波器，高计算量）
 *
 * @param q 质量级别，范围 [1, 10]
 */
void Resampler3::setQuality(int q)
{
    m_quality = qBound(1, q, 10);
}

// ──────────────────────────────────────────────
// 核心处理接口
// ──────────────────────────────────────────────

/**
 * @brief 对输入信号执行重采样
 *
 * 使用 sinc 插值将信号从源采样率转换到目标采样率。
 * 支持上采样和下采样。
 *
 * @param input 输入采样数据
 * @return 重采样后的数据
 */
QVector<double> Resampler3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int inLen = input.size();
    if (inLen == 0) return {};

    const double resampleRatio = m_target / m_source;
    const int outLen = static_cast<int>(qCeil(inLen * resampleRatio));

    if (outLen <= 0) return {};

    QVector<double> output = sincInterpolate(input, resampleRatio);

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalResamples++;
    m_stats.totalSamples += inLen;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalResamples;

    emit resamplingCompleted(inLen, output.size());
    return output;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含重采样次数、总采样数和平均耗时的Stats结构
 */
Resampler3::Stats Resampler3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void Resampler3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — sinc 插值
// ──────────────────────────────────────────────

/**
 * @brief 使用 sinc 插值进行重采样
 *
 * 对输出信号的每个采样点，通过 sinc 插值
 * 从输入信号中计算对应的值。
 *
 * sinc 插值公式：
 * y(t) = sum_{n} x[n] * sinc(t - n) * w(t - n)
 * 其中 w 为窗函数（Blackman 窗）。
 *
 * @param sig 输入信号
 * @param ratio 重采样比率（目标/源）
 * @return 重采样后的信号
 */
QVector<double> Resampler3::sincInterpolate(const QVector<double>& sig, double ratio)
{
    const int inLen = sig.size();
    const int outLen = static_cast<int>(qCeil(inLen * ratio));
    QVector<double> output(outLen, 0.0);

    // 滤波器半长度（由质量参数决定）
    const int halfLen = m_quality * 8;
    const double cutoff = qMin(1.0, 1.0 / ratio);

    // 预计算 sinc 滤波器系数表（用于加速）
    const int tableSize = 1024;
    QVector<double> sincTable(tableSize + 1);
    for (int i = 0; i <= tableSize; ++i) {
        double t = static_cast<double>(i) / tableSize;
        sincTable[i] = (t < 1e-10) ? 1.0 : qSin(M_PI * cutoff * t) / (M_PI * t);
    }

    for (int i = 0; i < outLen; ++i) {
        // 输出位置对应的输入位置
        double t = static_cast<double>(i) / ratio;

        int center = static_cast<int>(qFloor(t));
        double frac = t - center;

        double sum = 0.0;

        for (int k = -halfLen; k <= halfLen; ++k) {
            int idx = center + k;
            if (idx < 0 || idx >= inLen) continue;

            double dt = frac - static_cast<double>(k);

            // 查表获取 sinc 值
            double absDt = qAbs(dt);
            int tableIdx = qMin(tableSize, static_cast<int>(absDt * tableSize));
            double sincVal = sincTable[tableIdx];

            // Blackman 窗
            double window = 1.0;
            if (absDt > 1e-10) {
                double normPos = absDt / halfLen;
                window = 0.42 - 0.5 * qCos(2.0 * M_PI * normPos)
                        + 0.08 * qCos(4.0 * M_PI * normPos);
                if (normPos > 1.0) window = 0.0;
            }

            sum += sig[idx] * sincVal * window;
        }

        output[i] = sum;
    }

    return output;
}

// ──────────────────────────────────────────────
// 私有方法 — 滤波器设计
// ──────────────────────────────────────────────

/**
 * @brief 设计加窗 sinc 低通滤波器
 *
 * @param len 滤波器长度
 * @param cutoff 归一化截止频率 [0, 0.5]
 * @return 滤波器系数
 */
QVector<double> Resampler3::designFilter(int len, double cutoff) const
{
    QVector<double> h(len, 0.0);
    int mid = len / 2;

    double sum = 0.0;
    for (int i = 0; i < len; ++i) {
        double t = i - mid;
        double sinc = (qAbs(t) < 1e-10) ? 1.0 : qSin(2.0 * M_PI * cutoff * t) / (M_PI * t);
        // Blackman 窗
        double win = 0.42 - 0.5 * qCos(2.0 * M_PI * i / (len - 1))
                    + 0.08 * qCos(4.0 * M_PI * i / (len - 1));
        h[i] = sinc * win;
        sum += h[i];
    }

    // 归一化
    for (int i = 0; i < len; ++i) {
        h[i] /= sum;
    }

    return h;
}
