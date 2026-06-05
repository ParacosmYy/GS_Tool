/**
 * @file WaveShaper.cpp
 * @brief 波形塑形/失真处理器实现 — 传输函数查找表 + 过采样 + 抗混叠
 */

#include "utils/dsp14/WaveShaper.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <limits>

/* ========== 构造/配置 ========== */

WaveShaper::WaveShaper(QObject* parent)
    : QObject(parent), m_type(TanhSoft), m_drive(1.0), m_mix(1.0),
      m_oversample(X2), m_timeSum(0.0)
{
    m_transferTable.resize(TABLE_SIZE);
    rebuildTable();
}

void WaveShaper::setDistortionType(DistortionType type)
{
    m_type = type;
    rebuildTable();
}

void WaveShaper::setDrive(double gain)    { m_drive = qBound(0.1, gain, 10.0); }
void WaveShaper::setMix(double mix)       { m_mix = qBound(0.0, mix, 1.0); }
void WaveShaper::setOversample(OversampleFactor factor) { m_oversample = factor; }

void WaveShaper::setCustomTable(const QVector<double>& table)
{
    if (table.size() >= TABLE_SIZE) {
        m_type = Custom;
        for (int i = 0; i < TABLE_SIZE; ++i)
            m_transferTable[i] = qBound(-1.0, table[i], 1.0);
    }
}

/* ========== 查找表重建 ========== */

void WaveShaper::rebuildTable()
{
    for (int i = 0; i < TABLE_SIZE; ++i) {
        /* 将索引映射到[-1, 1] */
        double x = (2.0 * i / (TABLE_SIZE - 1)) - 1.0;
        double driven = x * m_drive;

        switch (m_type) {
        case TanhSoft:
            m_transferTable[i] = std::tanh(driven);
            break;
        case SoftClip:
            /* 三次多项式: (3/2)x - (1/2)x^3, 限幅到[-1,1] */
            if (qAbs(driven) < 1.0)
                m_transferTable[i] = 1.5 * driven - 0.5 * driven * driven * driven;
            else
                m_transferTable[i] = (driven > 0) ? 1.0 : -1.0;
            break;
        case HardClip:
            m_transferTable[i] = qBound(-1.0, driven, 1.0);
            break;
        case Fuzz:
            /* 全波整流 + 削波 */
            m_transferTable[i] = qBound(-1.0, qAbs(driven) * 1.5 - 0.25, 1.0);
            if (driven < 0) m_transferTable[i] = -m_transferTable[i];
            break;
        case TapeSaturation:
            /* 指数压缩: sign(x)*(1 - exp(-|x|*gain)) */
            m_transferTable[i] = (driven >= 0) ? (1.0 - qExp(-driven))
                                               : -(1.0 - qExp(driven));
            break;
        case Custom:
            /* 保持当前表不变 */
            return;
        }
    }
}

/* ========== 传输函数查找 ========== */

double WaveShaper::lookupTransfer(double x) const
{
    /* 线性插值查找 */
    double idx = (x + 1.0) * 0.5 * (TABLE_SIZE - 1);
    idx = qBound(0.0, idx, static_cast<double>(TABLE_SIZE - 1));
    int lo = static_cast<int>(idx);
    int hi = qMin(lo + 1, TABLE_SIZE - 1);
    double frac = idx - lo;
    return m_transferTable[lo] * (1.0 - frac) + m_transferTable[hi] * frac;
}

/* ========== 过采样 ========== */

QVector<double> WaveShaper::upsample(const QVector<double>& input) const
{
    int factor = static_cast<int>(m_oversample);
    int n = input.size();
    QVector<double> out(n * factor, 0.0);

    for (int i = 0; i < n; ++i) {
        out[i * factor] = input[i];
        /* 线性插值填充中间采样 */
        if (factor > 1 && i < n - 1) {
            for (int f = 1; f < factor; ++f) {
                double t = static_cast<double>(f) / factor;
                out[i * factor + f] = input[i] * (1.0 - t) + input[i + 1] * t;
            }
        }
    }
    return out;
}

QVector<double> WaveShaper::downsample(const QVector<double>& input) const
{
    int factor = static_cast<int>(m_oversample);
    int n = input.size() / factor;
    QVector<double> out(n);

    /* 均值滤波降采样 */
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int f = 0; f < factor; ++f)
            sum += input[i * factor + f];
        out[i] = sum / factor;
    }
    return out;
}

/* ========== 抗混叠滤波 ========== */

void WaveShaper::applyAntiAliasFilter(QVector<double>& data) const
{
    /* 简单一阶IIR低通: y[n] = alpha*x[n] + (1-alpha)*y[n-1]
     * alpha根据过采样率调整截止频率 */
    double alpha = 0.5 / static_cast<int>(m_oversample);
    if (data.isEmpty()) return;

    double prev = data[0];
    for (int i = 1; i < data.size(); ++i) {
        data[i] = alpha * data[i] + (1.0 - alpha) * prev;
        prev = data[i];
    }
    /* 反向滤波(零相位) */
    prev = data.last();
    for (int i = data.size() - 2; i >= 0; --i) {
        data[i] = alpha * data[i] + (1.0 - alpha) * prev;
        prev = data[i];
    }
}

/* ========== 单样本处理 ========== */

double WaveShaper::processOne(double sample)
{
    double driven = sample * m_drive;
    double shaped = lookupTransfer(driven);
    return sample * (1.0 - m_mix) + shaped * m_mix;
}

/* ========== 批量处理 ========== */

QVector<double> WaveShaper::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    if (input.isEmpty()) return output;

    int peakClips = 0;
    int n = input.size();

    if (m_oversample == None) {
        /* 无过采样：直接塑形 */
        output.resize(n);
        for (int i = 0; i < n; ++i) {
            double shaped = lookupTransfer(input[i] * m_drive);
            output[i] = input[i] * (1.0 - m_mix) + shaped * m_mix;
            if (qAbs(output[i]) >= 0.999) ++peakClips;
        }
    } else {
        /* 过采样 → 塑形 → 抗混叠 → 降采样 */
        QVector<double> upsampled = upsample(input);

        /* 对过采样信号塑形 */
        for (double& s : upsampled) {
            double shaped = lookupTransfer(s * m_drive);
            s = s * (1.0 - m_mix) + shaped * m_mix;
            if (qAbs(s) >= 0.999) ++peakClips;
        }

        /* 抗混叠低通 */
        applyAntiAliasFilter(upsampled);

        /* 降采样 */
        output = downsample(upsampled);
    }

    /* 软限幅保护 */
    for (double& s : output)
        s = qBound(-1.0, s, 1.0);

    /* 统计更新 */
    ++m_stats.totalBlocksProcessed;
    m_stats.totalSamplesProcessed += n;
    m_stats.totalPeakClips += peakClips;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBlocksProcessed;

    emit blockProcessed(n, peakClips);
    return output;
}

/* ========== 重置统计 ========== */

void WaveShaper::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
