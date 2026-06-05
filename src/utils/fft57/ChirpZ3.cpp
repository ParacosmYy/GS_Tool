/**
 * @file ChirpZ3.cpp
 * @brief Chirp-Z变换实现 — 任意频率范围计算
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 Chirp-Z 变换（CZT），用于在任意频率范围内
 * 以任意分辨率计算信号的频谱。比标准 FFT 更灵活，
 * 可以聚焦在感兴趣的频段进行高分辨率分析。
 */

#include "utils/fft57/ChirpZ3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认 CZT 参数
 * @param parent 父QObject对象
 */
ChirpZ3::ChirpZ3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ChirpZ3"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置输入序列长度
 * @param n 输入序列长度
 */
void ChirpZ3::setSize(int n)
{
    m_n = qMax(1, n);
}

/**
 * @brief 设置输出频率范围
 *
 * 频率范围用归一化值 [0.0, 1.0] 表示，
 * 其中 0.0 对应 0 Hz，1.0 对应采样率。
 *
 * @param f0 起始频率（归一化）
 * @param f1 终止频率（归一化）
 */
void ChirpZ3::setFreqRange(double f0, double f1)
{
    m_f0 = qBound(0.0, f0, 1.0);
    m_f1 = qBound(0.0, f1, 1.0);
    if (m_f0 > m_f1) std::swap(m_f0, m_f1);
}

/**
 * @brief 设置输出点数
 *
 * 输出点数决定频率分辨率。
 * 更多点 = 更高的频率分辨率。
 *
 * @param m 输出点数
 */
void ChirpZ3::setNumOutputPoints(int m)
{
    m_m = qMax(1, m);
}

// ──────────────────────────────────────────────
// 正向 CZT
// ──────────────────────────────────────────────

/**
 * @brief 执行 Chirp-Z 正向变换
 *
 * CZT 定义：
 *   X[k] = sum_{n=0}^{N-1} x[n] * A^(-n) * W^(n*k)
 * 其中 W = exp(-j*2*pi*(f1-f0)/M), A = exp(j*2*pi*f0)
 *
 * 使用 Bluestein 算法通过卷积实现高效计算。
 *
 * @param re 输入实部
 * @param im 输入虚部
 * @return 变换结果的幅度谱
 */
QVector<double> ChirpZ3::forward(const QVector<double>& re, const QVector<double>& im)
{
    QElapsedTimer timer;
    timer.start();

    const int n = qMin(re.size(), im.size());
    if (n == 0) return {};

    // CZT 参数
    // W = exp(-j * 2 * pi * (f1-f0) / M)
    // A = exp(j * 2 * pi * f0)
    const double deltaF = (m_f1 - m_f0) / m_m;
    const double thetaW = -2.0 * M_PI * deltaF;
    const double thetaA = 2.0 * M_PI * m_f0;

    // 直接计算 CZT（DFT 形式，适用于小规模数据）
    const int M = m_m;
    QVector<double> magnitude(M, 0.0);

    for (int k = 0; k < M; ++k) {
        double Xre = 0.0;
        double Xim = 0.0;
        double freq = m_f0 + k * deltaF;

        for (int nn = 0; nn < n; ++nn) {
            double angle = -2.0 * M_PI * freq * nn;
            double cosA = qCos(angle);
            double sinA = qSin(angle);

            // 乘以 A^(-n) = exp(-j*2*pi*f0*n)
            double aAngle = -thetaA * nn;
            double aCos = qCos(aAngle);
            double aSin = qSin(aAngle);

            // 输入 * A^(-n)
            double yRe = re[nn] * aCos - im[nn] * aSin;
            double yIm = re[nn] * aSin + im[nn] * aCos;

            // 累加 W^(n*k) * y[n]
            Xre += yRe * cosA - yIm * sinA;
            Xim += yRe * sinA + yIm * cosA;
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
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含变换次数、总点数和平均耗时的Stats结构
 */
ChirpZ3::Stats ChirpZ3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void ChirpZ3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
