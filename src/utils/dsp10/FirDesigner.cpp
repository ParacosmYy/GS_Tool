/**
 * @file FirDesigner.cpp
 * @brief Parks-McClellan FIR滤波器设计器实现
 */

#include "utils/dsp10/FirDesigner.h"

#include <QtMath>
#include <QtGlobal>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
FirDesigner::FirDesigner(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设计FIR滤波器
 * @param type 滤波器类型
 * @param filterOrder 滤波器阶数
 * @param freqBands 频段边界列表(归一化频率 0~1)
 * @param desiredGains 期望增益
 * @param weights 权重
 * @return 滤波器系数
 */
QVector<double> FirDesigner::design(FirType type,
                                    int filterOrder,
                                    const QVector<QPair<double, double>>& freqBands,
                                    const QVector<double>& desiredGains,
                                    const QVector<double>& weights)
{
    m_timer.start();

    /* 阶数必须为正整数 */
    int N = qMax(1, filterOrder);
    int nBands = freqBands.size();

    /* 构建密集频率网格 */
    QVector<double> gridDesired, gridWeight;
    QVector<double> gridFreq = buildDenseGrid(
        kGridDensity * (N + 1), freqBands, desiredGains,
        weights, gridDesired, gridWeight);

    if (gridFreq.isEmpty()) {
        return QVector<double>(N + 1, 0.0);
    }

    /* Remez交换算法 */
    int nExtremal = N / 2 + 2;
    QVector<double> coeffs = remezExchange(
        gridFreq.size(), gridFreq, gridDesired, gridWeight, nExtremal);

    /* 将脉冲响应从DFT系数还原 */
    QVector<double> h(N + 1, 0.0);
    int halfOrder = N / 2;

    /* 频域采样法还原: h[n] = IDFT of H[k] */
    for (int n = 0; n <= N; ++n) {
        double sum = 0.0;
        for (int k = 0; k < static_cast<int>(coeffs.size()); ++k) {
            double omega = M_PI * k / static_cast<double>(coeffs.size());
            sum += coeffs[k] * std::cos(omega * (n - halfOrder));
        }
        h[n] = sum / static_cast<double>(coeffs.size());
    }

    /* 根据类型调整: 高通/带阻需要奇数阶乘以(-1)^n */
    if (type == FirType::HighPass || type == FirType::BandStop) {
        if (N % 2 == 0) {
            for (int n = 0; n <= N; ++n) {
                h[n] *= (n % 2 == 0) ? 1.0 : -1.0;
            }
        }
    }

    /* 更新统计 */
    m_timeSum += m_timer.elapsed();
    ++m_stats.totalDesigns;
    m_stats.totalCoefficients += (N + 1);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDesigns);

    emit designCompleted(N, kMaxRemezIterations);
    return h;
}

/**
 * @brief 计算频率响应
 * @param coeffs 滤波器系数
 * @param numPoints 采样点数
 * @return (频率数组, 幅度数组)
 */
QPair<QVector<double>, QVector<double>>
FirDesigner::frequencyResponse(const QVector<double>& coeffs,
                                int numPoints) const
{
    QVector<double> freqs(numPoints), mag(numPoints);
    int N = coeffs.size() - 1;
    int halfOrder = N / 2;

    for (int i = 0; i < numPoints; ++i) {
        double w = M_PI * i / static_cast<double>(numPoints - 1);
        freqs[i] = w / M_PI; /* 归一化到[0,1] */

        double realPart = 0.0;
        for (int n = 0; n <= N; ++n) {
            realPart += coeffs[n] * std::cos(w * (n - halfOrder));
        }
        mag[i] = std::abs(realPart);
    }

    return {freqs, mag};
}

/**
 * @brief 应用滤波器到数据序列
 * @param coeffs 滤波器系数
 * @param input 输入数据
 * @return 滤波后数据
 */
QVector<double> FirDesigner::applyFilter(const QVector<double>& coeffs,
                                          const QVector<double>& input) const
{
    if (coeffs.isEmpty() || input.isEmpty()) return {};

    int N = coeffs.size();
    int M = input.size();
    QVector<double> output(M, 0.0);

    /* 直接卷积 */
    for (int i = 0; i < M; ++i) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k) {
            int idx = i - k;
            if (idx >= 0 && idx < M) {
                sum += coeffs[k] * input[idx];
            }
        }
        output[i] = sum;
    }

    return output;
}

/** @brief 重置统计 */
void FirDesigner::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief Remez交换算法核心
 * 迭代寻找最优极值频率集合
 */
QVector<double> FirDesigner::remezExchange(int gridSize,
                                            const QVector<double>& bandEdges,
                                            const QVector<double>& desired,
                                            const QVector<double>& weight,
                                            int extrPrefix)
{
    Q_UNUSED(bandEdges);
    int n = extrPrefix;

    /* 初始化极值点: 均匀分布在网格上 */
    QVector<int> extremal(n);
    for (int i = 0; i < n; ++i) {
        extremal[i] = i * (gridSize - 1) / qMax(1, n - 1);
    }

    QVector<double> x(n), c(n);

    for (int iter = 0; iter < kMaxRemezIterations; ++iter) {
        m_stats.totalIterations++;

        /* 提取极值点频率 */
        for (int i = 0; i < n; ++i) {
            x[i] = (extremal[i] < gridSize) ? desired[qMin(extremal[i], gridSize - 1)]
                                            : desired.back();
        }

        /* 计算barycentric权重 */
        QVector<double> baryW(n, 1.0);
        for (int j = 0; j < n; ++j) {
            for (int i = 0; i < n; ++i) {
                if (i != j) {
                    double denom = x[j] - x[i];
                    if (std::abs(denom) > 1e-15) {
                        baryW[j] /= denom;
                    }
                }
            }
        }

        /* 计算delta (最优等纹波振幅) */
        double sumNum = 0.0, sumDen = 0.0;
        for (int i = 0; i < n; ++i) {
            double val = desired[qMin(extremal[i], gridSize - 1)];
            double sign = (i % 2 == 0) ? 1.0 : -1.0;
            sumNum += baryW[i] * val;
            sumDen += baryW[i] * sign / weight[qMin(extremal[i], gridSize - 1)];
        }
        double delta = (std::abs(sumDen) > 1e-15) ? sumNum / sumDen : 0.0;

        /* 计算c数组 */
        for (int i = 0; i < n; ++i) {
            double sign = (i % 2 == 0) ? 1.0 : -1.0;
            int idx = qMin(extremal[i], gridSize - 1);
            c[i] = desired[idx] - sign * delta / weight[idx];
        }

        /* 寻找新的极值点 */
        QVector<int> newExtremal(n);
        int newCount = 0;
        for (int i = 0; i < gridSize && newCount < n; ++i) {
            double interpVal = lagrangeInterp(x, c, desired[i]);
            double error = std::abs(interpVal - desired[i]);
            /* 选择误差最大的点 */
            if (newCount == 0 || error > std::abs(
                lagrangeInterp(x, c, desired[newExtremal[qMax(0, newCount - 1)]])
                - desired[newExtremal[qMax(0, newCount - 1)]])) {
                if (newCount < n) {
                    newExtremal[newCount++] = i;
                }
            }
        }

        /* 填充剩余极值点 */
        while (newCount < n) {
            newExtremal[newCount++] = (newCount < gridSize)
                ? newCount : gridSize - 1;
        }

        extremal = newExtremal;
    }

    /* 最终系数 */
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        int idx = qBound(0, extremal[i], gridSize - 1);
        result[i] = desired[idx];
    }

    return result;
}

/**
 * @brief 构建密集频率网格
 * 在各频段上均匀采样并拼接
 */
QVector<double> FirDesigner::buildDenseGrid(
    int gridSize,
    const QVector<QPair<double, double>>& freqBands,
    const QVector<double>& desiredGains,
    const QVector<double>& weights,
    QVector<double>& outDesired,
    QVector<double>& outWeight) const
{
    outDesired.clear();
    outWeight.clear();
    QVector<double> grid;

    int pointsPerBand = qMax(8, gridSize / qMax(1, freqBands.size()));

    for (int b = 0; b < freqBands.size(); ++b) {
        double lo = qBound(0.0, freqBands[b].first, 1.0);
        double hi = qBound(0.0, freqBands[b].second, 1.0);
        if (lo >= hi) continue;

        double step = (hi - lo) / static_cast<double>(pointsPerBand - 1);
        double gain = (b < desiredGains.size()) ? desiredGains[b] : 1.0;
        double w = (b < weights.size()) ? weights[b] : 1.0;

        for (int i = 0; i < pointsPerBand; ++i) {
            grid.append(lo + step * i);
            outDesired.append(gain);
            outWeight.append(w);
        }
    }

    return grid;
}

/**
 * @brief 拉格朗日插值(barycentric形式)
 */
double FirDesigner::lagrangeInterp(const QVector<double>& x,
                                    const QVector<double>& y,
                                    double xVal) const
{
    int n = qMin(x.size(), y.size());
    if (n == 0) return 0.0;
    if (n == 1) return y[0];

    double num = 0.0, den = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = xVal - x[i];
        if (std::abs(diff) < 1e-15) return y[i];
        double w = 1.0 / diff;
        num += w * y[i];
        den += w;
    }

    return (std::abs(den) > 1e-15) ? num / den : 0.0;
}
