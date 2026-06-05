/**
 * @file WaveformFilterCompute.cpp
 * @brief 波形数字滤波器算法实现 -- 8种DSP滤波算法核心计算
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 本文件实现: filterMovingAverage / filterMedian / filterExponential /
 *             filterLowPass / filterHighPass / filterBandPass / filterBandStop /
 *             filterSavitzkyGolay 共8种滤波算法。
 * 构造/配置/分发/级联/统计见 WaveformFilter.cpp。
 */

#include "chart/filter/WaveformFilter.h"

#include <algorithm>
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════════
// 滑动平均滤波
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 滑动平均滤波
 * @param data 输入数据
 * @param windowSize 窗口大小(>=1)
 * @return 滤波后数据
 *
 * 每个输出点 = 窗口内采样值的算术平均,边界处对称填充。
 */
QVector<double> WaveformFilter::filterMovingAverage(const QVector<double> &data, int windowSize)
{
    if (data.isEmpty() || windowSize < 1) {
        return data;
    }

    const int n = data.size();
    const int half = windowSize / 2;
    QVector<double> result(n);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        int count = 0;
        for (int j = -half; j <= half; ++j) {
            const int idx = std::clamp(i + j, 0, n - 1);
            sum += data[idx];
            ++count;
        }
        result[i] = sum / static_cast<double>(count);
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 中值滤波
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 中值滤波
 * @param data 输入数据
 * @param windowSize 窗口大小(奇数)
 * @return 滤波后数据
 *
 * 每个输出点 = 窗口内采样值的中位数,边界处对称填充。
 */
QVector<double> WaveformFilter::filterMedian(const QVector<double> &data, int windowSize)
{
    if (data.isEmpty() || windowSize < 1) {
        return data;
    }

    const int n = data.size();
    const int half = windowSize / 2;
    QVector<double> result(n);

    for (int i = 0; i < n; ++i) {
        QVector<double> window;
        window.reserve(windowSize);
        for (int j = -half; j <= half; ++j) {
            const int idx = std::clamp(i + j, 0, n - 1);
            window.append(data[idx]);
        }
        std::sort(window.begin(), window.end());
        result[i] = window[window.size() / 2];
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 指数移动平均(EMA)
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 指数移动平均(EMA)
 * @param data 输入数据
 * @param alpha 平滑因子(0~1),值越大跟踪越快,平滑越弱
 * @return 滤波后数据
 *
 * 递推公式: y[n] = alpha * x[n] + (1-alpha) * y[n-1]
 */
QVector<double> WaveformFilter::filterExponential(const QVector<double> &data, double alpha)
{
    if (data.isEmpty()) {
        return data;
    }

    const int n = data.size();
    QVector<double> result(n);
    result[0] = data[0];

    for (int i = 1; i < n; ++i) {
        result[i] = alpha * data[i] + (1.0 - alpha) * result[i - 1];
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 低通RC滤波(单极IIR)
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 低通RC滤波(单极IIR)
 * @param data 输入数据
 * @param cutoffFreq 截止频率(Hz)
 * @param sampleRate 采样率(Sa/s)
 * @return 滤波后数据
 *
 * RC时间常数: tau = 1/(2*pi*fc), alpha = dt/(tau+dt)
 * 递推: y[n] = alpha*x[n] + (1-alpha)*y[n-1]
 */
QVector<double> WaveformFilter::filterLowPass(const QVector<double> &data, double cutoffFreq,
                                              double sampleRate)
{
    if (data.isEmpty() || cutoffFreq <= 0.0 || sampleRate <= 0.0) {
        return data;
    }

    const double rc = 1.0 / (2.0 * M_PI * cutoffFreq);
    const double dt = 1.0 / sampleRate;
    const double alpha = dt / (rc + dt);

    const int n = data.size();
    QVector<double> result(n);
    result[0] = data[0];

    for (int i = 1; i < n; ++i) {
        result[i] = alpha * data[i] + (1.0 - alpha) * result[i - 1];
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 高通RC滤波(单极IIR)
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 高通RC滤波(单极IIR)
 * @param data 输入数据
 * @param cutoffFreq 截止频率(Hz)
 * @param sampleRate 采样率(Sa/s)
 * @return 滤波后数据
 *
 * 递推: y[n] = alpha*(y[n-1] + x[n] - x[n-1])
 */
QVector<double> WaveformFilter::filterHighPass(const QVector<double> &data, double cutoffFreq,
                                               double sampleRate)
{
    if (data.isEmpty() || cutoffFreq <= 0.0 || sampleRate <= 0.0) {
        return data;
    }

    const double rc = 1.0 / (2.0 * M_PI * cutoffFreq);
    const double dt = 1.0 / sampleRate;
    const double alpha = rc / (rc + dt);

    const int n = data.size();
    QVector<double> result(n);
    result[0] = data[0];

    for (int i = 1; i < n; ++i) {
        result[i] = alpha * (result[i - 1] + data[i] - data[i - 1]);
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 带通滤波
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 带通滤波 -- 先高通后低通级联
 * @param data 输入数据
 * @param lowCutoff 低截止频率(Hz)
 * @param highCutoff 高截止频率(Hz)
 * @param sr 采样率(Sa/s)
 * @return 带通滤波结果
 */
QVector<double> WaveformFilter::filterBandPass(const QVector<double> &data, double lowCutoff,
                                               double highCutoff, double sr)
{
    if (data.isEmpty() || lowCutoff <= 0.0 || highCutoff <= lowCutoff || sr <= 0.0) {
        return data;
    }
    /* 先通过高通去除低频,再通过低通去除高频 */
    QVector<double> hp = filterHighPass(data, lowCutoff, sr);
    return filterLowPass(hp, highCutoff, sr);
}

// ═══════════════════════════════════════════════════════════════════════════════
// 带阻滤波
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 带阻滤波 -- 低通+高通差分组合
 * @param data 输入数据
 * @param lowCutoff 低截止频率(Hz)
 * @param highCutoff 高截止频率(Hz)
 * @param sr 采样率(Sa/s)
 * @return 带阻滤波结果
 *
 * y = lowPass(x, lowCutoff) + highPass(x, highCutoff)
 * 等效于从原信号中去除带通分量。
 */
QVector<double> WaveformFilter::filterBandStop(const QVector<double> &data, double lowCutoff,
                                               double highCutoff, double sr)
{
    if (data.isEmpty() || lowCutoff <= 0.0 || highCutoff <= lowCutoff || sr <= 0.0) {
        return data;
    }
    QVector<double> lp = filterLowPass(data, lowCutoff, sr);
    QVector<double> hp = filterHighPass(data, highCutoff, sr);

    const int n = data.size();
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = lp[i] + hp[i];
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Savitzky-Golay 多项式拟合滤波
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief Savitzky-Golay多项式拟合滤波
 * @param data 输入数据
 * @param windowSize 窗口大小(奇数, >= polyOrder+1)
 * @param polyOrder 多项式阶数(>=1)
 * @return 滤波后数据
 *
 * 使用最小二乘法拟合局部多项式,取中心点作为输出。
 * 边界处使用对称填充。通过法方程(J^T*J)*c = J^T*y 求解卷积系数。
 */
QVector<double> WaveformFilter::filterSavitzkyGolay(const QVector<double> &data, int windowSize,
                                                    int polyOrder)
{
    if (data.isEmpty() || windowSize < 3 || polyOrder < 1) {
        return data;
    }
    if (polyOrder >= windowSize) {
        polyOrder = windowSize - 1;
    }

    const int n = data.size();
    const int half = windowSize / 2;

    /* 构建 Vandermonde 矩阵 J[i][j] = x^j (x从-half到half) */
    const int cols = polyOrder + 1;
    QVector<QVector<double>> J(windowSize, QVector<double>(cols, 0.0));
    for (int i = 0; i < windowSize; ++i) {
        const double x = static_cast<double>(i - half);
        J[i][0] = 1.0;
        for (int j = 1; j < cols; ++j) {
            J[i][j] = J[i][j - 1] * x;
        }
    }

    /* 计算 J^T * J (cols x cols) */
    QVector<QVector<double>> JtJ(cols, QVector<double>(cols, 0.0));
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < cols; ++j) {
            double sum = 0.0;
            for (int k = 0; k < windowSize; ++k) {
                sum += J[k][i] * J[k][j];
            }
            JtJ[i][j] = sum;
        }
    }

    /* 高斯消元法求 JtJ 的逆矩阵: 构建增广矩阵 [JtJ | I] */
    QVector<QVector<double>> aug(cols, QVector<double>(2 * cols, 0.0));
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < cols; ++j) {
            aug[i][j] = JtJ[i][j];
        }
        aug[i][cols + i] = 1.0;
    }

    /* 选主元高斯消元 */
    for (int col = 0; col < cols; ++col) {
        int maxRow = col;
        for (int row = col + 1; row < cols; ++row) {
            if (std::abs(aug[row][col]) > std::abs(aug[maxRow][col])) {
                maxRow = row;
            }
        }
        std::swap(aug[col], aug[maxRow]);

        const double pivot = aug[col][col];
        if (std::abs(pivot) < 1e-12) {
            return data;  /* 奇异矩阵,回退原数据 */
        }
        for (int j = 0; j < 2 * cols; ++j) {
            aug[col][j] /= pivot;
        }
        for (int row = 0; row < cols; ++row) {
            if (row == col) {
                continue;
            }
            const double factor = aug[row][col];
            for (int j = 0; j < 2 * cols; ++j) {
                aug[row][j] -= factor * aug[col][j];
            }
        }
    }

    /* 提取逆矩阵 */
    QVector<QVector<double>> invJtJ(cols, QVector<double>(cols, 0.0));
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < cols; ++j) {
            invJtJ[i][j] = aug[i][cols + j];
        }
    }

    /* 计算卷积核: conv_coeff[k] = sum_{j} invJtJ[0][j] * J[k][j] */
    QVector<double> convCoeff(windowSize, 0.0);
    for (int k = 0; k < windowSize; ++k) {
        double val = 0.0;
        for (int j = 0; j < cols; ++j) {
            val += invJtJ[0][j] * J[k][j];
        }
        convCoeff[k] = val;
    }

    /* 应用卷积核 */
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < windowSize; ++k) {
            const int idx = std::clamp(i + k - half, 0, n - 1);
            sum += convCoeff[k] * data[idx];
        }
        result[i] = sum;
    }
    return result;
}
