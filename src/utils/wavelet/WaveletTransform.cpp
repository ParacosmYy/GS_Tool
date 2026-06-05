/**
 * @file WaveletTransform.cpp
 * @brief 小波变换引擎实现 — Haar/db2/db4/CDF97 DWT/IDWT
 */

#include "utils/wavelet/WaveletTransform.h"

#include <QtMath>
#include <algorithm>

WaveletTransform::WaveletTransform(QObject* parent)
    : QObject(parent), m_type(WaveletType::Haar), m_levels(3), m_energySum(0.0) {}

void WaveletTransform::setWaveletType(WaveletType type) { m_type = type; }
void WaveletTransform::setLevels(int levels) { m_levels = qMax(1, levels); }

/** @brief 多级分解 @param data 信号 @return 各级结果 */
QList<WaveletTransform::Decomposition> WaveletTransform::decompose(
    const QVector<double>& data)
{
    QList<Decomposition> result;
    if (data.isEmpty()) return result;

    QVector<double> current = data;
    for (int l = 0; l < m_levels; ++l) {
        if (current.size() < 4) break;
        Decomposition d = decomposeOneLevel(current);
        d.level = l + 1;
        result.append(d);
        current = d.approximation;
    }

    m_stats.totalTransforms++;
    m_stats.totalPointsProcessed += static_cast<quint64>(data.size());

    emit decomposed(result.size());
    return result;
}

/** @brief 单级DWT @param data 输入 @return 分解 */
WaveletTransform::Decomposition WaveletTransform::decomposeOneLevel(
    const QVector<double>& data)
{
    Decomposition result;
    int n = data.size();
    int half = n / 2;

    QVector<double> lowD, highD;
    QVector<double> lowR, highR;
    getWaveletCoeffs(lowD, highD, lowR, highR);

    int filtLen = lowD.size();
    result.approximation.resize(half);
    result.detail.resize(half);

    for (int i = 0; i < half; ++i) {
        double lowSum = 0.0, highSum = 0.0;
        for (int k = 0; k < filtLen; ++k) {
            int idx = (2 * i + k) % n;
            lowSum += data[idx] * lowD[k];
            highSum += data[idx] * highD[k];
        }
        result.approximation[i] = lowSum;
        result.detail[i] = highSum;
    }

    return result;
}

/** @brief IDWT重构 @param decomp 分解 @return 重构信号 */
QVector<double> WaveletTransform::reconstruct(
    const QList<Decomposition>& decomp)
{
    if (decomp.isEmpty()) return {};

    QVector<double> lowD, highD, lowR, highR;
    getWaveletCoeffs(lowD, highD, lowR, highR);
    int filtLen = lowR.size();

    /* 从最深层开始重构 */
    QVector<double> current = decomp.last().approximation;
    for (int l = decomp.size() - 1; l >= 0; --l) {
        const QVector<double>& approx = (l == 0) ? current : decomp[l - 1].approximation;
        const QVector<double>& detail = decomp[l].detail;

        int half = approx.size();
        int n = half * 2;
        QVector<double> output(n, 0.0);

        for (int i = 0; i < half; ++i) {
            for (int k = 0; k < filtLen; ++k) {
                int idx = 2 * i + k;
                if (idx < n) {
                    output[idx] += approx[i] * lowR[k] + detail[i] * highR[k];
                }
            }
        }
        current = output;
    }

    m_stats.totalReconstructions++;
    emit reconstructed(current.size());
    return current;
}

/** @brief 去噪 @param data 信号 @param threshold 阈值 @return 去噪信号 */
QVector<double> WaveletTransform::denoise(const QVector<double>& data,
                                            double threshold)
{
    QList<Decomposition> decomp = decompose(data);

    /* 对细节系数软阈值 */
    for (auto& d : decomp) {
        for (auto& v : d.detail) {
            if (qAbs(v) < threshold) {
                v = 0.0;
            } else {
                v = (v > 0 ? 1.0 : -1.0) * (qAbs(v) - threshold);
            }
        }
    }

    return reconstruct(decomp);
}

void WaveletTransform::resetStatistics()
{
    m_stats = Stats{};
    m_energySum = 0.0;
}

/** @brief 获取小波系数 */
void WaveletTransform::getWaveletCoeffs(
    QVector<double>& lowDecomp, QVector<double>& highDecomp,
    QVector<double>& lowRecon, QVector<double>& highRecon) const
{
    switch (m_type) {
    case WaveletType::Haar:
        lowDecomp  = {1.0/qSqrt(2.0), 1.0/qSqrt(2.0)};
        highDecomp = {1.0/qSqrt(2.0), -1.0/qSqrt(2.0)};
        lowRecon   = {1.0/qSqrt(2.0), 1.0/qSqrt(2.0)};
        highRecon  = {1.0/qSqrt(2.0), -1.0/qSqrt(2.0)};
        break;
    case WaveletType::DB2:
        /* Daubechies 2 系数(简化) */
        lowDecomp  = {0.4829629, 0.8365163, 0.2241438, -0.1294095};
        highDecomp = {-0.1294095, -0.2241438, 0.8365163, -0.4829629};
        lowRecon   = {-0.1294095, 0.2241438, 0.8365163, 0.4829629};
        highRecon  = {-0.4829629, 0.8365163, -0.2241438, -0.1294095};
        break;
    case WaveletType::DB4:
        /* Daubechies 4 系数(简化8点) */
        lowDecomp  = {0.230378, 0.714847, 0.630881, -0.027983,
                      -0.187035, 0.030841, 0.032883, -0.010597};
        highDecomp = {-0.010597, -0.032883, 0.030841, 0.187035,
                      -0.027983, -0.630881, 0.714847, -0.230378};
        lowRecon   = {-0.010597, 0.032883, 0.030841, -0.187035,
                      -0.027983, 0.630881, 0.714847, 0.230378};
        highRecon  = {-0.230378, 0.714847, -0.630881, -0.027983,
                      0.187035, 0.030841, -0.032883, -0.010597};
        break;
    case WaveletType::CDF97:
        /* CDF 9/7 (简化) */
        lowDecomp  = {0.026749, -0.016864, -0.078223, 0.266864,
                      0.602949, 0.266864, -0.078223, -0.016864, 0.026749};
        highDecomp = {0.0, 0.091272, -0.057544, -0.591272,
                      1.115087, -0.591272, -0.057544, 0.091272, 0.0};
        lowRecon   = highDecomp;
        highRecon  = lowDecomp;
        break;
    }
}
