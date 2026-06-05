/**
 * @file ZoomFFT3.cpp
 * @brief Zoom FFT实现 — 频率缩放变换
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 Zoom FFT（频率缩放 FFT），用于对指定频段进行高分辨率频谱分析。
 * 通过数字下变频 + 低通滤波 + 降采样 + FFT 实现高效的频段聚焦分析。
 */

#include "utils/fft58/ZoomFFT3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认 Zoom FFT 参数
 * @param parent 父QObject对象
 */
ZoomFFT3::ZoomFFT3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ZoomFFT3"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置输入序列长度
 * @param n 输入采样点数
 */
void ZoomFFT3::setInputSize(int n)
{
    m_n = qMax(1, n);
}

/**
 * @brief 设置缩放频段的中心和带宽
 *
 * 中心频率和带宽均使用归一化值 [0.0, 1.0]，
 * 其中 0.0 = 0 Hz，1.0 = 采样率。
 *
 * @param fCenter 中心频率（归一化）
 * @param fSpan 频段宽度（归一化）
 */
void ZoomFFT3::setZoomRange(double fCenter, double fSpan)
{
    m_fCenter = qBound(0.0, fCenter, 1.0);
    m_fSpan = qBound(0.001, fSpan, 1.0);
}

/**
 * @brief 设置输出点数
 *
 * 输出点数决定缩放频段内的频率分辨率。
 *
 * @param m 输出频谱点数
 */
void ZoomFFT3::setOutputSize(int m)
{
    m_m = qMax(1, m);
}

// ──────────────────────────────────────────────
// 正向 Zoom FFT
// ──────────────────────────────────────────────

/**
 * @brief 执行 Zoom FFT 变换
 *
 * 处理步骤：
 * 1. 数字下变频：将感兴趣的频段搬移到基带
 * 2. 低通滤波：滤除带外频率成分
 * 3. 降采样：降低采样率提高相对分辨率
 * 4. FFT：对降采样后的信号进行频谱分析
 *
 * @param re 输入实部
 * @param im 输入虚部
 * @return 缩放频段的幅度谱
 */
QVector<double> ZoomFFT3::forward(const QVector<double>& re, const QVector<double>& im)
{
    QElapsedTimer timer;
    timer.start();

    const int n = qMin(re.size(), im.size());
    if (n == 0) return {};

    // 步骤1：数字下变频
    // 乘以 exp(-j*2*pi*fCenter*t) 将中心频率搬移到基带
    QVector<double> bbRe(n), bbIm(n);
    for (int i = 0; i < n; ++i) {
        double angle = -2.0 * M_PI * m_fCenter * i;
        double cosA = qCos(angle);
        double sinA = qSin(angle);
        // 复数乘法：(re + j*im) * (cosA + j*sinA)
        bbRe[i] = re[i] * cosA - im[i] * sinA;
        bbIm[i] = re[i] * sinA + im[i] * cosA;
    }

    // 步骤2：低通滤波
    // 截止频率 = fSpan / 2
    const double cutoff = m_fSpan / 2.0;
    const int filterLen = qMin(63, n);
    QVector<double> filterCoeff(filterLen);
    double filterSum = 0.0;

    // 设计 sinc 低通滤波器
    for (int k = 0; k < filterLen; ++k) {
        double t = k - (filterLen - 1) / 2.0;
        double sinc = (qAbs(t) < 1e-10) ? 1.0 : qSin(2.0 * M_PI * cutoff * t) / (M_PI * t);
        // Hann 窗
        double win = 0.5 * (1.0 - qCos(2.0 * M_PI * k / (filterLen - 1)));
        filterCoeff[k] = sinc * win;
        filterSum += filterCoeff[k];
    }
    // 归一化
    for (int k = 0; k < filterLen; ++k) {
        filterCoeff[k] /= filterSum;
    }

    // 卷积滤波
    QVector<double> filtRe(n, 0.0), filtIm(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < filterLen; ++k) {
            int idx = i - k + filterLen / 2;
            if (idx >= 0 && idx < n) {
                filtRe[i] += bbRe[idx] * filterCoeff[k];
                filtIm[i] += bbIm[idx] * filterCoeff[k];
            }
        }
    }

    // 步骤3：降采样
    // 降采样率 = n / m_m（简化为整数倍）
    int decFactor = qMax(1, n / (2 * m_m));
    int decLen = n / decFactor;
    QVector<double> decRe(decLen), decIm(decLen);
    for (int i = 0; i < decLen; ++i) {
        decRe[i] = filtRe[i * decFactor];
        decIm[i] = filtIm[i * decFactor];
    }

    // 步骤4：FFT（DFT 简化实现）
    const int M = qMin(m_m, decLen);
    QVector<double> magnitude(M, 0.0);

    for (int k = 0; k < M; ++k) {
        double Xre = 0.0, Xim = 0.0;
        for (int i = 0; i < decLen; ++i) {
            double angle = -2.0 * M_PI * k * i / decLen;
            Xre += decRe[i] * qCos(angle) - decIm[i] * qSin(angle);
            Xim += decRe[i] * qSin(angle) + decIm[i] * qCos(angle);
        }
        magnitude[k] = qSqrt(Xre * Xre + Xim * Xim);
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalPoints += M;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, M);
    return magnitude;
}

// ──────────────────────────────────────────────
// 频率分辨率查询
// ──────────────────────────────────────────────

/**
 * @brief 计算当前 Zoom FFT 的频率分辨率
 *
 * 频率分辨率 = 带宽 / 输出点数
 *
 * @return 频率分辨率（归一化）
 */
double ZoomFFT3::frequencyResolution() const
{
    if (m_m <= 0) return 0.0;
    return m_fSpan / m_m;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含变换次数、总点数和平均耗时的Stats结构
 */
ZoomFFT3::Stats ZoomFFT3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void ZoomFFT3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
