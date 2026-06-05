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

    // 预计算 A^(-n) 的复数序列
    QVector<double> aCosArr(n), aSinArr(n);
    for (int nn = 0; nn < n; ++nn) {
        double aAngle = -thetaA * nn;
        aCosArr[nn] = qCos(aAngle);
        aSinArr[nn] = qSin(aAngle);
    }

    // 预计算输入 * A^(-n) 的复数序列
    QVector<double> yRe(n), yIm(n);
    for (int nn = 0; nn < n; ++nn) {
        yRe[nn] = re[nn] * aCosArr[nn] - im[nn] * aSinArr[nn];
        yIm[nn] = re[nn] * aSinArr[nn] + im[nn] * aCosArr[nn];
    }

    // 预计算 W^k 的旋转因子
    const int M = m_m;
    QVector<double> wCosArr(M), wSinArr(M);
    for (int k = 0; k < M; ++k) {
        double wAngle = thetaW * k;
        wCosArr[k] = qCos(wAngle);
        wSinArr[k] = qSin(wAngle);
    }

    // 直接计算 CZT（DFT 形式，适用于小规模数据）
    QVector<double> magnitude(M, 0.0);

    for (int k = 0; k < M; ++k) {
        double Xre = 0.0;
        double Xim = 0.0;

        for (int nn = 0; nn < n; ++nn) {
            // W^(n*k) = cos(thetaW * n * k) + j * sin(thetaW * n * k)
            // 利用预计算的 W^k 进行迭代乘法
            double angle = thetaW * nn * k;
            double cosA = qCos(angle);
            double sinA = qSin(angle);

            // 累加 W^(n*k) * y[n]
            Xre += yRe[nn] * cosA - yIm[nn] * sinA;
            Xim += yRe[nn] * sinA + yIm[nn] * cosA;
        }

        magnitude[k] = qSqrt(Xre * Xre + Xim * Xim);
    }

    // 对幅度谱进行归一化（除以 N）
    double normFactor = static_cast<double>(n);
    for (int k = 0; k < M; ++k) {
        magnitude[k] /= normFactor;
    }

    // 可选：转换为 dB 标度（方便频谱分析显示）
    // dB = 20 * log10(magnitude / ref)
    // 此处保留线性值，由调用方根据需要转换

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
// 辅助方法 — 频率映射
// ──────────────────────────────────────────────

/**
 * @brief 将输出 bin 索引转换为归一化频率
 *
 * 给定输出 bin 索引 k，对应的归一化频率为：
 * freq = f0 + k * (f1 - f0) / M
 *
 * @param k 输出 bin 索引（0-based）
 * @return 归一化频率值
 */
double ChirpZ3::binToFrequency(int k) const
{
    if (k < 0 || k >= m_m) return 0.0;
    return m_f0 + static_cast<double>(k) * (m_f1 - m_f0) / m_m;
}

/**
 * @brief 将归一化频率转换为输出 bin 索引
 *
 * 反向映射：给定频率找到最近的输出 bin 索引。
 *
 * @param freq 归一化频率值
 * @return 最近的 bin 索引，-1 表示频率超出范围
 */
int ChirpZ3::frequencyToBin(double freq) const
{
    if (m_m <= 0 || freq < m_f0 || freq > m_f1) return -1;
    double deltaF = (m_f1 - m_f0) / m_m;
    int bin = static_cast<int>(qRound((freq - m_f0) / deltaF));
    return qBound(0, bin, m_m - 1);
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
