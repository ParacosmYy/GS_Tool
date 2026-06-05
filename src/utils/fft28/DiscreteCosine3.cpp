/**
 * @file DiscreteCosine3.cpp
 * @brief 离散余弦变换增强实现 — DCT-I/II/III/IV/快速/2D
 */

#include "utils/fft28/DiscreteCosine3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
DiscreteCosine3::DiscreteCosine3(QObject* parent)
    : QObject(parent)
{
}

/** @brief 正向DCT @param input 输入信号 @param type DCT类型 @return 变换结果 */
QVector<double> DiscreteCosine3::forward(const QVector<double>& input,
                                          DctType type)
{
    QElapsedTimer timer;
    timer.start();
    int n = input.size();
    if (n == 0) return {};

    QVector<double> result;
    switch (type) {
    case DctType::TypeI:
        result = dctTypeI(input, false);
        break;
    case DctType::TypeII:
        result = (n >= 4 && (n & (n - 1)) == 0)
            ? fastDctII(input) : dctTypeII(input, false);
        break;
    case DctType::TypeIII:
        result = (n >= 4 && (n & (n - 1)) == 0)
            ? fastDctIII(input) : dctTypeIII(input, false);
        break;
    case DctType::TypeIV:
        result = dctTypeIV(input, false);
        break;
    }

    double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformComplete(n, type);
    return result;
}

/** @brief 逆向DCT @param input 频域系数 @param type DCT类型 @return 时域信号 */
QVector<double> DiscreteCosine3::inverse(const QVector<double>& input,
                                          DctType type)
{
    if (input.isEmpty()) return {};

    /* DCT-I和DCT-IV自逆; DCT-II逆=DCT-III(带缩放) */
    switch (type) {
    case DctType::TypeI:
    case DctType::TypeIV:
        return forward(input, type);
    case DctType::TypeII:
        return forward(input, DctType::TypeIII);
    case DctType::TypeIII:
        return forward(input, DctType::TypeII);
    }
    return {};
}

/** @brief 2D-DCT @param matrix 输入矩阵 @param rows 行数 @param cols 列数 @return 变换矩阵 */
QVector<double> DiscreteCosine3::forward2D(const QVector<double>& matrix,
                                            int rows, int cols)
{
    if (rows <= 0 || cols <= 0) return {};
    int total = rows * cols;
    if (matrix.size() != total) return {};

    /* 先按行变换 */
    QVector<double> temp(total);
    for (int r = 0; r < rows; ++r) {
        QVector<double> row(cols);
        for (int c = 0; c < cols; ++c) {
            row[c] = matrix[r * cols + c];
        }
        QVector<double> rowDct = forward(row, DctType::TypeII);
        for (int c = 0; c < cols; ++c) {
            temp[r * cols + c] = rowDct[c];
        }
    }

    /* 再按列变换 */
    QVector<double> result(total);
    for (int c = 0; c < cols; ++c) {
        QVector<double> col(rows);
        for (int r = 0; r < rows; ++r) {
            col[r] = temp[r * cols + c];
        }
        QVector<double> colDct = forward(col, DctType::TypeII);
        for (int r = 0; r < rows; ++r) {
            result[r * cols + c] = colDct[r];
        }
    }

    return result;
}

/** @brief 2D-IDCT @param matrix 频域矩阵 @param rows 行数 @param cols 列数 @return 时域矩阵 */
QVector<double> DiscreteCosine3::inverse2D(const QVector<double>& matrix,
                                            int rows, int cols)
{
    if (rows <= 0 || cols <= 0) return {};
    int total = rows * cols;
    if (matrix.size() != total) return {};

    /* 先按行逆变换 */
    QVector<double> temp(total);
    for (int r = 0; r < rows; ++r) {
        QVector<double> row(cols);
        for (int c = 0; c < cols; ++c) {
            row[c] = matrix[r * cols + c];
        }
        QVector<double> rowIdct = forward(row, DctType::TypeIII);
        for (int c = 0; c < cols; ++c) {
            temp[r * cols + c] = rowIdct[c];
        }
    }

    /* 再按列逆变换 */
    QVector<double> result(total);
    for (int c = 0; c < cols; ++c) {
        QVector<double> col(rows);
        for (int r = 0; r < rows; ++r) {
            col[r] = temp[r * cols + c];
        }
        QVector<double> colIdct = forward(col, DctType::TypeIII);
        for (int r = 0; r < rows; ++r) {
            result[r * cols + c] = colIdct[r];
        }
    }

    return result;
}

/** @brief 能量集中比 @param coeffs DCT系数 @param keepRatio 保留比例 @return 保留能量占比 */
double DiscreteCosine3::energyConcentration(const QVector<double>& coeffs,
                                             double keepRatio) const
{
    if (coeffs.isEmpty()) return 0.0;

    double totalEnergy = 0.0;
    for (double c : coeffs) {
        totalEnergy += c * c;
    }
    if (totalEnergy <= 0.0) return 0.0;

    /* 按绝对值降序排列 */
    QVector<double> sorted = coeffs;
    std::sort(sorted.begin(), sorted.end(),
              [](double a, double b) { return qAbs(a) > qAbs(b); });

    int keepCount = qMax(1, static_cast<int>(sorted.size() * keepRatio));
    double keptEnergy = 0.0;
    for (int i = 0; i < keepCount; ++i) {
        keptEnergy += sorted[i] * sorted[i];
    }

    return keptEnergy / totalEnergy;
}

/** @brief DCT-I @param input 输入 @param inverse 是否逆变换 @return 结果 */
QVector<double> DiscreteCosine3::dctTypeI(const QVector<double>& input,
                                           bool inverse) const
{
    int n = input.size();
    if (n < 2) return input;
    QVector<double> output(n);

    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = M_PI * i * k / (n - 1);
            sum += input[i] * qCos(angle);
        }
        output[k] = sum;
    }
    return output;
}

/** @brief DCT-II(朴素) @param input 输入 @param inverse 是否逆变换 @return 结果 */
QVector<double> DiscreteCosine3::dctTypeII(const QVector<double>& input,
                                            bool inverse) const
{
    int n = input.size();
    QVector<double> output(n);

    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = M_PI * (i + 0.5) * k / n;
            sum += input[i] * qCos(angle);
        }
        output[k] = sum;
    }
    return output;
}

/** @brief DCT-III(朴素) @param input 输入 @param inverse 是否逆变换 @return 结果 */
QVector<double> DiscreteCosine3::dctTypeIII(const QVector<double>& input,
                                             bool inverse) const
{
    int n = input.size();
    QVector<double> output(n);

    for (int k = 0; k < n; ++k) {
        double sum = input[0] * 0.5;
        for (int i = 1; i < n; ++i) {
            double angle = M_PI * i * (k + 0.5) / n;
            sum += input[i] * qCos(angle);
        }
        output[k] = sum * 2.0 / n;
    }
    return output;
}

/** @brief DCT-IV @param input 输入 @param inverse 是否逆变换 @return 结果 */
QVector<double> DiscreteCosine3::dctTypeIV(const QVector<double>& input,
                                            bool inverse) const
{
    int n = input.size();
    QVector<double> output(n);

    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = M_PI * (i + 0.5) * (k + 0.5) / n;
            sum += input[i] * qCos(angle);
        }
        output[k] = sum;
    }
    return output;
}

/** @brief 快速DCT-II(基于FFT, 长度须为2的幂) @param input 输入 @return 结果 */
QVector<double> DiscreteCosine3::fastDctII(const QVector<double>& input) const
{
    int n = input.size();
    /* 重排: 偶数索引在前, 奇数索引逆序在后 */
    QVector<double> reord(n);
    int half = n / 2;
    for (int i = 0; i < half; ++i) {
        reord[i] = input[2 * i];
        reord[n - 1 - i] = input[2 * i + 1];
    }
    if (n % 2 == 1) {
        reord[half] = input[n - 1];
    }

    /* N点FFT(内联基2) */
    QVector<double> real = reord, imag(n, 0.0);
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }
    for (int len = 2; len <= n; len *= 2) {
        double ang = -2.0 * M_PI / len;
        double wR = qCos(ang), wI = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = e + len / 2;
                double tR = cR * real[o] - cI * imag[o];
                double tI = cR * imag[o] + cI * real[o];
                real[o] = real[e] - tR; imag[o] = imag[e] - tI;
                real[e] += tR; imag[e] += tI;
                double nR = cR * wR - cI * wI;
                cI = cR * wI + cI * wR; cR = nR;
            }
        }
    }

    /* 后处理: 乘以旋转因子 */
    QVector<double> result(n);
    for (int k = 0; k < n; ++k) {
        double angle = M_PI * k / (2.0 * n);
        result[k] = 2.0 * (real[k] * qCos(angle) + imag[k] * qSin(angle));
    }
    return result;
}

/** @brief 快速DCT-III @param input 输入 @return 结果 */
QVector<double> DiscreteCosine3::fastDctIII(const QVector<double>& input) const
{
    int n = input.size();
    QVector<double> pre(n);
    pre[0] = input[0] * 0.5;
    for (int i = 1; i < n; ++i) {
        double angle = M_PI * i / (2.0 * n);
        pre[i] = input[i] * qCos(angle);
    }

    /* 简单IDCT-III: 利用DCT-II对称性 */
    QVector<double> result = fastDctII(pre);
    for (int i = 0; i < n; ++i) {
        result[i] *= 2.0 / n;
    }
    return result;
}

/** @brief 重置统计 */
void DiscreteCosine3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_energyRatioSum = 0.0;
}
