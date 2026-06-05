/**
 * @file HarmonicProduct2.cpp
 * @brief 谐波乘积谱基频检测实现 — HPS基频估计
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现谐波乘积谱（Harmonic Product Spectrum, HPS）算法进行基频检测。
 * 将频谱按不同下采样因子压缩后逐点相乘，
 * 基频位置的乘积值最大，从而检测出基频。
 */

#include "utils/signal58/HarmonicProduct2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认 HPS 参数
 * @param parent 父QObject对象
 */
HarmonicProduct2::HarmonicProduct2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("HarmonicProduct2"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz）
 */
void HarmonicProduct2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置谐波数量
 *
 * 谐波数决定下采样的层数。
 * 更多谐波可以提高检测精度，但对缺失谐波更敏感。
 *
 * @param n 谐波数量，范围 [2, 8]
 */
void HarmonicProduct2::setNumHarmonics(int n)
{
    m_numHarm = qBound(2, n, 8);
}

/**
 * @brief 设置 FFT 大小
 * @param n FFT 窗口大小
 */
void HarmonicProduct2::setFFTSize(int n)
{
    m_fftSize = qMax(64, n);
}

// ──────────────────────────────────────────────
// 核心检测接口
// ──────────────────────────────────────────────

/**
 * @brief 对频域帧执行基频检测
 *
 * HPS 算法步骤：
 * 1. 对输入帧计算幅度谱
 * 2. 对幅度谱进行 R=1,2,...,numHarmonics 倍下采样
 * 3. 将所有下采样谱逐点相乘得到 HPS
 * 4. HPS 最大值位置对应的频率即为基频
 *
 * @param frame 输入时域帧
 * @return 检测到的基频（Hz），0.0 表示未检测到
 */
double HarmonicProduct2::detect(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = frame.size();
    if (n < 4) {
        m_fundFreq = 0.0;
        m_hps.clear();
        return 0.0;
    }

    // 步骤1：计算幅度谱
    const int fftN = qMin(n, m_fftSize);
    const int numBins = fftN / 2 + 1;
    QVector<double> magnitude(numBins, 0.0);

    for (int k = 0; k < numBins; ++k) {
        double re = 0.0;
        double im = 0.0;
        for (int i = 0; i < fftN; ++i) {
            double sample = (i < n) ? frame[i] : 0.0;
            // Hann 窗
            double win = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (fftN - 1)));
            double angle = -2.0 * M_PI * k * i / fftN;
            re += sample * win * qCos(angle);
            im += sample * win * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im);
    }

    // 步骤2：计算 HPS
    const int hpsLen = numBins / m_numHarm;
    m_hps.resize(hpsLen, 1.0);

    for (int k = 0; k < hpsLen; ++k) {
        m_hps[k] = magnitude[k]; // R=1 层

        // 乘以下采样层
        for (int r = 2; r <= m_numHarm; ++r) {
            int idx = k * r;
            if (idx < numBins) {
                m_hps[k] *= magnitude[idx];
            } else {
                m_hps[k] = 0.0;
                break;
            }
        }
    }

    // 步骤3：找 HPS 峰值
    // 限制搜索范围在 [50Hz, 2000Hz] 对应的 bin
    const int minBin = qMax(1, static_cast<int>(50.0 * fftN / m_sampleRate));
    const int maxBin = qMin(hpsLen - 1, static_cast<int>(2000.0 * fftN / m_sampleRate));

    double maxVal = 0.0;
    int maxIdx = 0;
    for (int k = minBin; k <= maxBin; ++k) {
        if (m_hps[k] > maxVal) {
            maxVal = m_hps[k];
            maxIdx = k;
        }
    }

    // 步骤4：转换为频率
    if (maxIdx > 0 && maxVal > 1e-10) {
        m_fundFreq = static_cast<double>(maxIdx) * m_sampleRate / fftN;

        // 抛物线插值精化峰值位置
        if (maxIdx > 0 && maxIdx < hpsLen - 1) {
            double alpha = m_hps[maxIdx - 1];
            double beta  = m_hps[maxIdx];
            double gamma = m_hps[maxIdx + 1];
            double denom = alpha - 2.0 * beta + gamma;
            if (qAbs(denom) > 1e-15) {
                double delta = 0.5 * (alpha - gamma) / denom;
                delta = qBound(-0.5, delta, 0.5);
                m_fundFreq = (maxIdx + delta) * m_sampleRate / fftN;
            }
        }
    } else {
        m_fundFreq = 0.0;
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalDetections++;
    m_stats.totalFrames++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    // 计算置信度
    double confidence = 0.0;
    if (maxVal > 0.0) {
        // 计算峰值与均值的比值作为置信度
        double meanHps = 0.0;
        for (int k = minBin; k <= maxBin; ++k) {
            meanHps += m_hps[k];
        }
        meanHps /= (maxBin - minBin + 1);
        confidence = (meanHps > 1e-15) ? maxVal / meanHps : 0.0;
    }

    emit pitchDetected(m_fundFreq, confidence);
    return m_fundFreq;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含检测次数、总帧数和平均耗时的Stats结构
 */
HarmonicProduct2::Stats HarmonicProduct2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void HarmonicProduct2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
