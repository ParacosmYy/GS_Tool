/**
 * @file DaubechiesWavelet.cpp
 * @brief Daubechies小波族引擎实现
 */

#include "DaubechiesWavelet.h"

#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

DaubechiesWavelet::DaubechiesWavelet(QObject* parent)
    : QObject(parent)
{
    loadFilters(m_order);
}

DaubechiesWavelet::~DaubechiesWavelet() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void DaubechiesWavelet::setOrder(int order)
{
    m_order = std::min({std::max(order, 2), 20});
    if (m_order % 2 != 0) m_order--;
    loadFilters(m_order);
}

int DaubechiesWavelet::order() const { return m_order; }

// ═══════════════════════════════════════════════════════════
// 多级正变换
// ═══════════════════════════════════════════════════════════

QVector<QVector<double>> DaubechiesWavelet::forward(const QVector<double>& signal,
                                                     int levels)
{
    if (signal.size() < 2) {
        emit error(tr("Daubechies正变换错误: 信号太短"));
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    // 计算最大级数
    int maxLevels = 0;
    int len = signal.size();
    while (len >= static_cast<int>(m_decompLow.size())) {
        len = (len + 1) / 2;
        maxLevels++;
    }
    if (levels <= 0 || levels > maxLevels) levels = maxLevels;

    QVector<QVector<double>> result;
    result.reserve(levels + 1);

    QVector<double> current = signal;

    for (int lvl = 0; lvl < levels; ++lvl) {
        QVector<double> approx, detail;
        convolveForward(current, approx, detail);
        result.append(detail); // 细节系数
        current = approx;

        m_stats.totalCoeffs += static_cast<quint64>(detail.size());
    }
    result.append(current); // 最终近似系数

    m_stats.totalForward++;
    m_stats.avgTimeMs = (m_stats.avgTimeMs * (m_stats.totalForward - 1) +
                         timer.elapsed()) / static_cast<double>(m_stats.totalForward);

    emit forwardCompleted(levels, static_cast<int>(m_stats.totalCoeffs));
    return result;
}

// ═══════════════════════════════════════════════════════════
// 多级逆变换
// ═══════════════════════════════════════════════════════════

QVector<double> DaubechiesWavelet::inverse(const QVector<QVector<double>>& coefficients,
                                            int originalLength)
{
    if (coefficients.size() < 2) {
        emit error(tr("Daubechies逆变换错误: 系数不足"));
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    // 从最深层开始重构
    QVector<double> approx = coefficients.last();

    for (int lvl = coefficients.size() - 2; lvl >= 0; --lvl) {
        const QVector<double>& detail = coefficients[lvl];
        QVector<double> reconstructed;
        convolveInverse(approx, detail, reconstructed);
        approx = reconstructed;
    }

    // 裁剪到原始长度
    if (approx.size() > originalLength) {
        approx.resize(originalLength);
    }

    m_stats.totalInverse++;
    m_stats.avgTimeMs = (m_stats.avgTimeMs * (m_stats.totalInverse - 1) +
                         timer.elapsed()) / static_cast<double>(m_stats.totalInverse);

    emit inverseCompleted(approx.size());
    return approx;
}

// ═══════════════════════════════════════════════════════════
// 单级变换
// ═══════════════════════════════════════════════════════════

QVector<QVector<double>> DaubechiesWavelet::forwardOneLevel(const QVector<double>& signal)
{
    QVector<QVector<double>> result(2);
    convolveForward(signal, result[0], result[1]);
    m_stats.totalForward++;
    return result;
}

QVector<double> DaubechiesWavelet::inverseOneLevel(const QVector<double>& approx,
                                                    const QVector<double>& detail,
                                                    int outputLength)
{
    QVector<double> result;
    convolveInverse(approx, detail, result);
    if (result.size() > outputLength) result.resize(outputLength);
    m_stats.totalInverse++;
    return result;
}

// ═══════════════════════════════════════════════════════════
// 去噪
// ═══════════════════════════════════════════════════════════

QVector<double> DaubechiesWavelet::denoise(const QVector<double>& signal,
                                            double threshold, int levels)
{
    QVector<QVector<double>> coeffs = forward(signal, levels);
    if (coeffs.size() < 2) return signal;

    // 对细节系数做软阈值处理(保留最后一层的近似)
    for (int i = 0; i < static_cast<int>(coeffs.size()) - 1; ++i) {
        for (int j = 0; j < coeffs[i].size(); ++j) {
            coeffs[i][j] = softThreshold(coeffs[i][j], threshold);
        }
    }

    return inverse(coeffs, signal.size());
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

DaubechiesWavelet::Stats DaubechiesWavelet::stats() const { return m_stats; }
void DaubechiesWavelet::resetStatistics() { m_stats = Stats{}; }

// ═══════════════════════════════════════════════════════════
// 内部: 加载滤波器系数
// ═══════════════════════════════════════════════════════════

void DaubechiesWavelet::loadFilters(int order)
{
    m_decompLow.clear();
    m_decompHigh.clear();
    m_reconLow.clear();
    m_reconHigh.clear();

    // Daubechies滤波器系数(仅存储DB2~DB20的分解低通系数)
    // 分解高通 = 分解低通交替正负号反转
    // 重构滤波器 = 分解滤波器的时间反转

    switch (order) {
    case 2:
        m_decompLow = {0.7071067811865476, 0.7071067811865476};
        break;
    case 4:
        m_decompLow = {0.6830127, 1.1830127, 0.3169873, -0.1830127};
        break;
    case 6:
        m_decompLow = {0.4704672, 1.1411169, 0.6503656, -0.1909344,
                       -0.1208322, 0.0498175};
        break;
    case 8:
        m_decompLow = {0.3258039, 1.0109457, 0.8922014, -0.0396750,
                       -0.2645072, 0.0436163, 0.0465082, -0.0149869};
        break;
    case 10:
        m_decompLow = {0.2264113, 0.8539435, 1.0243114, 0.1955519,
                       -0.3425250, -0.0456012, 0.1094469, -0.0008116,
                       -0.0217912, 0.0052808};
        break;
    case 12:
        m_decompLow = {0.1577424, 0.6995036, 1.0622702, 0.4493926,
                       -0.3256937, -0.1397865, 0.1472849, 0.0340803,
                       -0.0658710, 0.0003048, 0.0106047, -0.0018924};
        break;
    default:
        // 默认使用DB4
        m_decompLow = {0.6830127, 1.1830127, 0.3169873, -0.1830127};
        m_order = 4;
        break;
    }

    const int n = m_decompLow.size();

    // 分解高通: 交替正负号的反转
    m_decompHigh.resize(n);
    for (int i = 0; i < n; ++i) {
        m_decompHigh[i] = ((i % 2 == 0) ? 1.0 : -1.0) *
                           m_decompLow[n - 1 - i];
    }

    // 重构低通: 时间反转 + 交替正负
    m_reconLow.resize(n);
    for (int i = 0; i < n; ++i) {
        m_reconLow[i] = m_decompLow[n - 1 - i];
    }

    // 重构高通: 时间反转
    m_reconHigh.resize(n);
    for (int i = 0; i < n; ++i) {
        m_reconHigh[i] = m_decompHigh[n - 1 - i];
    }
}

// ═══════════════════════════════════════════════════════════
// 内部: 卷积
// ═══════════════════════════════════════════════════════════

void DaubechiesWavelet::convolveForward(const QVector<double>& signal,
                                         QVector<double>& approx,
                                         QVector<double>& detail) const
{
    const int n = signal.size();
    const int filterLen = m_decompLow.size();
    const int half = (n + 1) / 2;

    approx.resize(half);
    detail.resize(half);

    for (int i = 0; i < half; ++i) {
        double sumA = 0.0, sumD = 0.0;
        for (int k = 0; k < filterLen; ++k) {
            const int idx = (2 * i + k) % n; // 周期延拓
            sumA += m_decompLow[k] * signal[idx];
            sumD += m_decompHigh[k] * signal[idx];
        }
        approx[i] = sumA;
        detail[i] = sumD;
    }
}

void DaubechiesWavelet::convolveInverse(const QVector<double>& approx,
                                         const QVector<double>& detail,
                                         QVector<double>& output) const
{
    const int half = approx.size();
    const int n = half * 2;
    const int filterLen = m_reconLow.size();

    output.resize(n);
    output.fill(0.0);

    for (int i = 0; i < half; ++i) {
        for (int k = 0; k < filterLen; ++k) {
            const int idx = (2 * i + k) % n;
            output[idx] += m_reconLow[k] * approx[i];
            output[idx] += m_reconHigh[k] * detail[i];
        }
    }
}

double DaubechiesWavelet::softThreshold(double value, double threshold)
{
    if (std::abs(value) <= threshold) return 0.0;
    return (value > 0) ? (value - threshold) : (value + threshold);
}
