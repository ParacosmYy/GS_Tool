/**
 * @file DaubechiesD4.cpp
 * @brief Daubechies-4小波变换实现 — 正/逆变换+阈值去噪
 */

#include "DaubechiesD4.h"

#include <QElapsedTimer>
#include <cmath>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

DaubechiesD4::DaubechiesD4(QObject* parent)
    : QObject(parent)
{
}

DaubechiesD4::~DaubechiesD4() = default;

// ═══════════════════════════════════════════════════════════
// 正向变换
// ═══════════════════════════════════════════════════════════

QVector<double> DaubechiesD4::forward(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) {
        return {};
    }

    /* 复制并补齐到2的幂 */
    QVector<double> data = signal;
    int n = padToPowerOf2(data);

    /* 逐层分解: 每层将当前长度减半 */
    int len = n;
    while (len >= 4) {
        forwardPass(data, len);
        len /= 2;
    }

    /* 更新统计 */
    m_stats.totalTransforms++;
    const qint64 elapsed = timer.elapsed();
    const auto cnt = m_stats.totalTransforms;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * (cnt - 1) / cnt +
        static_cast<double>(elapsed) / cnt;

    emit forwardCompleted(data.size());
    return data;
}

// ═══════════════════════════════════════════════════════════
// 逆向变换
// ═══════════════════════════════════════════════════════════

QVector<double> DaubechiesD4::inverse(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    if (coeffs.isEmpty()) {
        return {};
    }

    QVector<double> data = coeffs;
    int n = data.size();

    /* 逐层重建: 从最小尺度开始 */
    int len = 4;
    while (len <= n) {
        inversePass(data, len);
        len *= 2;
    }

    /* 更新统计 */
    m_stats.totalTransforms++;
    const qint64 elapsed = timer.elapsed();
    const auto cnt = m_stats.totalTransforms;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * (cnt - 1) / cnt +
        static_cast<double>(elapsed) / cnt;

    emit inverseCompleted(data.size());
    return data;
}

// ═══════════════════════════════════════════════════════════
// 阈值去噪
// ═══════════════════════════════════════════════════════════

QVector<double> DaubechiesD4::denoise(const QVector<double>& signal,
                                       double threshold,
                                       ThresholdType type)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) {
        return {};
    }

    /* 正向变换 */
    QVector<double> coeffs = forward(signal);

    /* 对细节系数(后半部分各层)应用阈值 */
    int n = coeffs.size();
    int half = n / 2;

    /* 第一层细节系数 */
    for (int i = half; i < n; ++i) {
        double val = coeffs[i];
        if (type == ThresholdType::Soft) {
            if (std::abs(val) <= threshold) {
                coeffs[i] = 0.0;
            } else if (val > 0) {
                coeffs[i] = val - threshold;
            } else {
                coeffs[i] = val + threshold;
            }
        } else {
            if (std::abs(val) <= threshold) {
                coeffs[i] = 0.0;
            }
        }
    }

    /* 逆向变换 */
    QVector<double> result = inverse(coeffs);

    /* 截断到原始长度 */
    if (result.size() > signal.size()) {
        result.resize(signal.size());
    }

    m_stats.totalTransforms++;
    const qint64 elapsed = timer.elapsed();
    const auto cnt = m_stats.totalTransforms;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * (cnt - 1) / cnt +
        static_cast<double>(elapsed) / cnt;

    emit denoiseCompleted(result.size());
    return result;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

DaubechiesD4::Stats DaubechiesD4::stats() const
{
    return m_stats;
}

void DaubechiesD4::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部实现
// ═══════════════════════════════════════════════════════════

int DaubechiesD4::padToPowerOf2(QVector<double>& data) const
{
    int n = data.size();
    int p = 1;
    while (p < n) {
        p *= 2;
    }
    /* 零填充 */
    if (p > n) {
        data.resize(p, 0.0);
    }
    return p;
}

void DaubechiesD4::forwardPass(QVector<double>& data, int len)
{
    int half = len / 2;
    QVector<double> approx(half, 0.0);
    QVector<double> detail(half, 0.0);

    for (int i = 0; i < half; ++i) {
        int idx = 2 * i;
        double s0 = data[idx];
        double s1 = data[(idx + 1) % len];
        double s2 = data[(idx + 2) % len];
        double s3 = data[(idx + 3) % len];

        /* 低通(近似) */
        approx[i] = S_LP[0] * s0 + S_LP[1] * s1 +
                     S_LP[2] * s2 + S_LP[3] * s3;
        /* 高通(细节) */
        detail[i] = S_HP[0] * s0 + S_HP[1] * s1 +
                     S_HP[2] * s2 + S_HP[3] * s3;
    }

    /* 写回: 近似在前半, 细节在后半 */
    for (int i = 0; i < half; ++i) {
        data[i] = approx[i];
        data[half + i] = detail[i];
    }
}

void DaubechiesD4::inversePass(QVector<double>& data, int len)
{
    int half = len / 2;
    QVector<double> result(len, 0.0);

    for (int i = 0; i < half; ++i) {
        double a = data[i];           /* 近似系数 */
        double d = data[half + i];    /* 细节系数 */

        int idx = 2 * i;
        /* 逆滤波器: 由分解滤波器的逆矩阵得到 */
        result[idx % len]           += S_LP[2] * a + S_HP[2] * d;
        result[(idx + 1) % len]     += S_LP[3] * a + S_HP[3] * d;
        result[(idx + 2) % len]     += S_LP[0] * a + S_HP[0] * d;
        result[(idx + 3) % len]     += S_LP[1] * a + S_HP[1] * d;
    }

    for (int i = 0; i < len; ++i) {
        data[i] = result[i];
    }
}
