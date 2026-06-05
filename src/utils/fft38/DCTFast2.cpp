/**
 * @file DCTFast2.cpp
 * @brief 快速DCT实现 - 基于FFT约化的一维与二维DCT变换
 *
 * 利用DCT与DFT的关系，将N点DCT-II映射为2N点FFT，
 * 通过重新排列输入实现O(N log N)复杂度。
 * 二维DCT通过行列分离实现: 先逐行变换，再逐列变换。
 */

#include "utils/fft38/DCTFast2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认大小(256)
 * @param parent 父QObject
 */
DCTFast2::DCTFast2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置DCT变换大小
 * @param n 变换长度，建议为2的幂
 */
void DCTFast2::setSize(int n)
{
    m_n = qMax(2, n);
}

/**
 * @brief 基础DFT实现(用于无法使用FFT时的回退)
 * @param re 实部数组(输入/输出)
 * @param im 虚部数组(输入/输出)
 * @param n 长度
 * @param inverse 是否为逆变换
 */
static void dft(QVector<double>& re, QVector<double>& im, int n, bool inverse)
{
    QVector<double> reOut(n, 0.0);
    QVector<double> imOut(n, 0.0);

    double sign = inverse ? 1.0 : -1.0;
    double scale = 1.0 / n;

    for (int k = 0; k < n; ++k) {
        double sumRe = 0.0, sumIm = 0.0;
        for (int t = 0; t < n; ++t) {
            double angle = sign * 2.0 * M_PI * k * t / n;
            sumRe += re[t] * qCos(angle) - im[t] * qSin(angle);
            sumIm += re[t] * qSin(angle) + im[t] * qCos(angle);
        }
        if (inverse) {
            reOut[k] = sumRe * scale;
            imOut[k] = sumIm * scale;
        } else {
            reOut[k] = sumRe;
            imOut[k] = sumIm;
        }
    }
    re = reOut;
    im = imOut;
}

/**
 * @brief Cooley-Tukey radix-2 FFT
 * @param re 实部数组
 * @param im 虚部数组
 * @param inverse 是否逆变换
 */
static void fft(QVector<double>& re, QVector<double>& im, bool inverse)
{
    int n = re.size();

    /* 位反转排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = (inverse ? 2.0 : -2.0) * M_PI / len;
        double wRe = qCos(angle);
        double wIm = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;

                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];

                re[v] = re[u] - tRe;
                im[v] = im[u] - tIm;
                re[u] += tRe;
                im[u] += tIm;

                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < n; ++i) {
            re[i] /= n;
            im[i] /= n;
        }
    }
}

/**
 * @brief 正向DCT-II变换
 *
 * DCT-II公式: X[k] = sum_{n=0}^{N-1} x[n] * cos(pi*(2n+1)*k/(2N))
 * 通过将N点输入重排为2N点序列后做FFT实现。
 *
 * @param input 输入序列(长度为m_n)
 * @return DCT系数(长度为m_n)
 */
QVector<double> DCTFast2::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int N = qMin(m_n, input.size());

    QVector<double> output(N, 0.0);

    if (N < 2) {
        m_stats.totalTransforms++;
        m_stats.totalSamplesProcessed += N;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
        return output;
    }

    /* 重排输入为2N点序列:
       y[n] = x[2n]       for n = 0, 1, ..., N/2-1
       y[n] = x[2N-2n-1]  for n = N/2, ..., N-1
       然后对2N序列做FFT */
    int N2 = N;
    QVector<double> re(N2, 0.0);
    QVector<double> im(N2, 0.0);

    /* 重排 */
    for (int n = 0; n < N2 / 2; ++n) {
        re[n] = input[2 * n];
    }
    for (int n = N2 / 2; n < N2; ++n) {
        re[n] = input[2 * N2 - 2 * n - 1];
    }

    /* 执行FFT */
    bool isPowerOf2 = (N2 & (N2 - 1)) == 0;
    if (isPowerOf2) {
        fft(re, im, false);
    } else {
        dft(re, im, N2, false);
    }

    /* 提取DCT系数 */
    for (int k = 0; k < N; ++k) {
        double angle = M_PI * k / (2.0 * N);
        double cosA = qCos(angle);
        double sinA = qSin(angle);

        /* X[k] = Re{FFT[k]} * 2 * cos(pi*k/(2N)) + Im{FFT[k]} * 2 * sin(pi*k/(2N)) */
        output[k] = 2.0 * (re[k] * cosA + im[k] * sinA);
    }

    /* 第一个系数归一化因子 */
    output[0] /= qSqrt(2.0);

    /* 整体归一化 */
    double normFactor = qSqrt(2.0 / N) / 2.0;
    for (int k = 0; k < N; ++k)
        output[k] *= normFactor;

    output[0] *= qSqrt(2.0);

    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformComplete(N);
    return output;
}

/**
 * @brief 逆DCT变换(DCT-III)
 *
 * DCT-III是DCT-II的逆变换，通过转置公式实现。
 *
 * @param coeffs DCT系数
 * @return 重建的时域信号
 */
QVector<double> DCTFast2::inverse(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    const int N = qMin(m_n, coeffs.size());

    QVector<double> output(N, 0.0);

    if (N < 2) {
        m_timeSum += timer.elapsed();
        return output;
    }

    /* DCT-III: x[n] = sum_{k=0}^{N-1} c[k] * X[k] * cos(pi*k*(2n+1)/(2N))
       其中 c[0] = 1/sqrt(2), c[k] = 1 (k>0) */
    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k) {
            double c = (k == 0) ? qSqrt(2.0) : 1.0;
            sum += c * coeffs[k] * qCos(M_PI * k * (2 * n + 1) / (2.0 * N));
        }
        output[n] = sum * qSqrt(2.0 / N) / 2.0;
    }

    m_timeSum += timer.elapsed();
    return output;
}

/**
 * @brief 二维正向DCT变换
 *
 * 行列分离法: 先对每行做一维DCT，再对每列做一维DCT。
 *
 * @param matrix 输入矩阵(rows x cols)
 * @return 二维DCT系数矩阵
 */
QVector<QVector<double>> DCTFast2::forward2D(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    const int rows = matrix.size();
    const int cols = (rows > 0) ? matrix[0].size() : 0;

    if (rows < 2 || cols < 2) {
        m_stats.totalTransforms++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
        return matrix;
    }

    /* 第一步: 对每行做一维DCT */
    int savedN = m_n;
    QVector<QVector<double>> rowDCT(rows);
    for (int r = 0; r < rows; ++r) {
        m_n = cols;
        rowDCT[r] = forward(matrix[r]);
    }

    /* 第二步: 对每列做一维DCT */
    QVector<QVector<double>> result(rows, QVector<double>(cols, 0.0));
    for (int c = 0; c < cols; ++c) {
        /* 提取列 */
        QVector<double> col(rows);
        for (int r = 0; r < rows; ++r)
            col[r] = rowDCT[r][c];

        m_n = rows;
        QVector<double> colDCT = forward(col);

        /* 写回结果矩阵 */
        for (int r = 0; r < rows; ++r)
            result[r][c] = colDCT[r];
    }

    m_n = savedN;

    m_timeSum += timer.elapsed();
    return result;
}

/**
 * @brief 二维逆DCT变换
 * @param matrix DCT系数矩阵
 * @return 重建的时域矩阵
 */
QVector<QVector<double>> DCTFast2::inverse2D(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    const int rows = matrix.size();
    const int cols = (rows > 0) ? matrix[0].size() : 0;

    if (rows < 2 || cols < 2) {
        m_timeSum += timer.elapsed();
        return matrix;
    }

    /* 逆列变换 */
    int savedN = m_n;
    QVector<QVector<double>> colIDCT(rows, QVector<double>(cols, 0.0));
    for (int c = 0; c < cols; ++c) {
        QVector<double> col(rows);
        for (int r = 0; r < rows; ++r)
            col[r] = matrix[r][c];

        m_n = rows;
        QVector<double> colResult = inverse(col);
        for (int r = 0; r < rows; ++r)
            colIDCT[r][c] = colResult[r];
    }

    /* 逆行变换 */
    QVector<QVector<double>> result(rows, QVector<double>(cols, 0.0));
    for (int r = 0; r < rows; ++r) {
        m_n = cols;
        result[r] = inverse(colIDCT[r]);
    }

    m_n = savedN;

    m_timeSum += timer.elapsed();
    return result;
}

/**
 * @brief 重置所有统计数据
 */
void DCTFast2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
