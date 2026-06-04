/**
 * @file WaveformGeneratorExport.cpp
 * @brief 数学波形发生器 -- 二进制转换与数据分析方法
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 从 WaveformGenerator.cpp 拆分而来,包含:
 *   - toByteArray(): 采样数据转二进制字节流(8/16/32bit小端序)
 *   - analyze():      波形数据统计分析(峰值/峰峰值/均值/RMS/极值)
 *
 * 核心生成与后处理方法保留在 WaveformGenerator.cpp,
 * 统计计数器方法见 WaveformGeneratorStats.cpp。
 */

#include "utils/waveform/WaveformGenerator.h"

#include <QtMath>

#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
// 后处理: 二进制转换
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 将采样数据转换为二进制字节流(小端序)
 * @param data 采样数据
 * @param bitsPerSample 每采样位宽: 8(int8_t), 16(int16_t), 32(float)
 * @return 字节数组;位宽不支持时返回空
 *
 * 8bit: clamp到[-128,127]后转int8_t
 * 16bit: clamp到[-32768,32767]后转int16_t
 * 32bit: 直接转float(IEEE 754)
 */
QByteArray WaveformGenerator::toByteArray(const QVector<double> &data,
                                          int bitsPerSample) const
{
    if (data.isEmpty()) return {};

    QByteArray bytes;

    switch (bitsPerSample) {
    case 8: {
        bytes.reserve(data.size());
        for (const double v : data) {
            const int rounded = qRound(v);
            const auto clamped = static_cast<int8_t>(
                qBound(-128, rounded, 127));
            bytes.append(reinterpret_cast<const char *>(&clamped), 1);
        }
        break;
    }
    case 16: {
        bytes.reserve(data.size() * 2);
        for (const double v : data) {
            const int rounded = qRound(v);
            const auto clamped = static_cast<int16_t>(
                qBound(-32768, rounded, 32767));
            bytes.append(reinterpret_cast<const char *>(&clamped), 2);
        }
        break;
    }
    case 32: {
        bytes.reserve(data.size() * 4);
        for (const double v : data) {
            const auto f = static_cast<float>(v);
            bytes.append(reinterpret_cast<const char *>(&f), 4);
        }
        break;
    }
    default:
        return {};  /* 不支持的位宽 */
    }

    return bytes;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 分析: 波形数据统计
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 分析波形数据,计算峰值/峰峰值/均值/RMS/极值
 * @param data 采样数据
 * @return 分析结果结构体;空数据返回零值
 */
WaveformGenerator::WaveformAnalysis WaveformGenerator::analyze(
    const QVector<double> &data) const
{
    WaveformAnalysis a{};
    if (data.isEmpty()) return a;

    double sum = 0.0;
    double sumSq = 0.0;
    double minVal = data.first();
    double maxVal = data.first();

    for (const double v : data) {
        sum += v;
        sumSq += v * v;
        if (v < minVal) minVal = v;
        if (v > maxVal) maxVal = v;
    }

    const double n = static_cast<double>(data.size());
    a.min       = minVal;
    a.max       = maxVal;
    a.mean      = sum / n;
    a.rms       = qSqrt(sumSq / n);
    a.peak      = qMax(qFabs(minVal), qFabs(maxVal));
    a.peakToPeak = maxVal - minVal;

    return a;
}
