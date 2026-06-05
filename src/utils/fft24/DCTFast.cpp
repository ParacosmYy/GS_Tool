/**
 * @file DCTFast.cpp
 * @brief 快速DCT实现 — FFT基/TypeII/TypeIII/2D/量化
 */

#include "utils/fft24/DCTFast.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
DCTFast::DCTFast(QObject* parent)
    : QObject(parent)
{
}

/** @brief 一维DCT变换 @param input 输入数据 @param type DCT类型 @return 变换结果 */
QVector<double> DCTFast::transform(const QVector<double>& input, DCTType type)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return {};

    int n = input.size();
    QVector<double> result = input;

    switch (type) {
    case DCTType::TypeII:
    case DCTType::TypeIIScaled:
        /* 使用AAN快速算法(适合小尺寸)或FFT方法 */
        if (n <= 64 && ((n & (n - 1)) == 0)) {
            aanDCTII(result);
        } else {
            fftBasedDCTII(result);
        }
        if (type == DCTType::TypeIIScaled) {
            /* 正交缩放 */
            result[0] *= qSqrt(1.0 / (4.0 * n));
            for (int i = 1; i < n; ++i) {
                result[i] *= qSqrt(1.0 / (2.0 * n));
            }
        }
        break;
    case DCTType::TypeIII:
        fftBasedDCTIII(result);
        break;
    }

    double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalElementsProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);
    if (n > m_stats.maxTransformSize) m_stats.maxTransformSize = n;

    emit transformComplete(n, static_cast<int>(type));
    return result;
}

/** @brief 一维逆DCT @param input DCT系数 @param type DCT类型 @return 逆变换结果 */
QVector<double> DCTFast::inverseTransform(const QVector<double>& input,
                                           DCTType type)
{
    if (input.isEmpty()) return {};

    switch (type) {
    case DCTType::TypeII:
        return transform(input, DCTType::TypeIII);
    case DCTType::TypeIII:
    case DCTType::TypeIIScaled:
        return transform(input, DCTType::TypeII);
    }
    return {};
}

/** @brief 2D DCT @param input 行优先2D数据 @param rows 行数 @param cols 列数 @return 2D DCT系数 */
QVector<double> DCTFast::transform2D(const QVector<double>& input,
                                      int rows, int cols)
{
    QElapsedTimer timer;
    timer.start();

    if (input.size() < rows * cols) return {};

    QVector<double> result(rows * cols);

    /* 行变换 */
    for (int r = 0; r < rows; ++r) {
        QVector<double> row(cols);
        for (int c = 0; c < cols; ++c) row[c] = input[r * cols + c];
        row = transform(row, DCTType::TypeII);
        for (int c = 0; c < cols; ++c) result[r * cols + c] = row[c];
    }

    /* 列变换 */
    for (int c = 0; c < cols; ++c) {
        QVector<double> col(rows);
        for (int r = 0; r < rows; ++r) col[r] = result[r * cols + c];
        col = transform(col, DCTType::TypeII);
        for (int r = 0; r < rows; ++r) result[r * cols + c] = col[r];
    }

    double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalElementsProcessed += rows * cols;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);
    int sz = qMax(rows, cols);
    if (sz > m_stats.maxTransformSize) m_stats.maxTransformSize = sz;

    return result;
}

/** @brief 2D逆DCT @param input 2D系数 @param rows 行数 @param cols 列数 @return 逆变换 */
QVector<double> DCTFast::inverseTransform2D(const QVector<double>& input,
                                             int rows, int cols)
{
    if (input.size() < rows * cols) return {};

    QVector<double> result(rows * cols);

    /* 行逆变换 */
    for (int r = 0; r < rows; ++r) {
        QVector<double> row(cols);
        for (int c = 0; c < cols; ++c) row[c] = input[r * cols + c];
        row = transform(row, DCTType::TypeIII);
        for (int c = 0; c < cols; ++c) result[r * cols + c] = row[c];
    }

    /* 列逆变换 */
    for (int c = 0; c < cols; ++c) {
        QVector<double> col(rows);
        for (int r = 0; r < rows; ++r) col[r] = result[r * cols + c];
        col = transform(col, DCTType::TypeIII);
        for (int r = 0; r < rows; ++r) result[r * cols + c] = col[r];
    }

    /* 归一化 */
    double scale = 4.0 / (rows * cols);
    for (auto& v : result) v *= scale;
    return result;
}

/** @brief 量化DCT系数 @param coeffs 系数 @param quantTable 量化表 @return 量化后系数 */
QVector<double> DCTFast::quantize(const QVector<double>& coeffs,
                                   const QVector<double>& quantTable)
{
    int n = qMin(coeffs.size(), quantTable.size());
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        double q = qMax(1.0, quantTable[i]);
        result[i] = qRound(coeffs[i] / q) * q;
    }
    return result;
}

/** @brief 重置统计 */
void DCTFast::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief FFT基DCT-II @param data 数据(原地) */
void DCTFast::fftBasedDCTII(QVector<double>& data)
{
    int n = data.size();
    int fftSize = nextPowerOf2(2 * n);

    /* 重排: 偶数索引在前, 奇数索引反转在后 */
    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            real[i / 2] = data[i];
        } else {
            real[n - 1 - (i - 1) / 2] = data[i];
        }
    }

    baseFFT(real, imag);

    /* 后处理: 乘以旋转因子 */
    data.resize(n);
    for (int k = 0; k < n; ++k) {
        double angle = M_PI * k / (2.0 * n);
        double re = real[k] * qCos(angle) + imag[k] * qSin(angle);
        data[k] = re * 2.0;
    }
}

/** @brief FFT基DCT-III @param data 数据(原地) */
void DCTFast::fftBasedDCTIII(QVector<double>& data)
{
    int n = data.size();
    int fftSize = nextPowerOf2(2 * n);

    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);

    /* 预处理: 乘以旋转因子 */
    real[0] = data[0];
    for (int k = 1; k < n; ++k) {
        double angle = M_PI * k / (2.0 * n);
        real[k] = data[k] * qCos(angle);
        imag[k] = -data[k] * qSin(angle);
    }

    baseFFT(real, imag);

    /* 提取结果 */
    data.resize(n);
    for (int i = 0; i < n; ++i) {
        int idx = (i % 2 == 0) ? i / 2 : n - 1 - (i - 1) / 2;
        data[idx] = real[i] / n;
    }
}

/** @brief AAN快速DCT-II @param data 数据(原地) */
void DCTFast::aanDCTII(QVector<double>& data)
{
    int n = data.size();
    if (n == 1) return;
    if (n == 2) {
        double a = data[0] + data[1];
        double b = data[0] - data[1];
        data[0] = a;
        data[1] = b;
        return;
    }

    /* 递归: 分为偶数/奇数子序列 */
    QVector<double> even(n / 2), odd(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        even[i] = data[i] + data[n - 1 - i];
        odd[i] = (data[i] - data[n - 1 - i])
            / (2.0 * qCos(M_PI * (2 * i + 1) / (2.0 * n)));
    }

    aanDCTII(even);
    aanDCTII(odd);

    for (int k = 0; k < n / 2; ++k) {
        data[k] = even[k];
        data[k + n / 2] = odd[k];
    }

    /* 合并 */
    for (int k = 0; k < n / 2; ++k) {
        double a = data[k];
        double b = data[k + n / 2];
        data[2 * k] = a + b;
        if (2 * k + 1 < n) data[2 * k + 1] = a - b;
    }
}

/** @brief 基2 FFT @param real 实部 @param imag 虚部 */
void DCTFast::baseFFT(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    imag.fill(0.0);

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
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double cRe = 1.0, cIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = e + len / 2;
                double tRe = cRe * real[o] - cIm * imag[o];
                double tIm = cRe * imag[o] + cIm * real[o];
                real[o] = real[e] - tRe;
                imag[o] = imag[e] - tIm;
                real[e] += tRe;
                imag[e] += tIm;
                double nRe = cRe * wRe - cIm * wIm;
                double nIm = cRe * wIm + cIm * wRe;
                cRe = nRe; cIm = nIm;
            }
        }
    }
}

/** @brief 下一个2的幂 @param n 输入 @return 2的幂 */
int DCTFast::nextPowerOf2(int n) const
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}
