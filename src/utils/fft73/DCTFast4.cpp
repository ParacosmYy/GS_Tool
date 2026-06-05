/**
 * @file DCTFast4.cpp
 * @brief 快速离散余弦变换(DCT)实现
 *
 * 实现DCT-II(前向)和DCT-III(逆变换)快速算法，
 * 支持一维和二维DCT变换，适用于图像/音频压缩。
 */

#include "utils/fft73/DCTFast4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
DCTFast4::DCTFast4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置变换大小
 * @param n 变换长度
 */
void DCTFast4::setSize(int n)
{
    m_n = qMax(2, n);
}

/**
 * @brief 前向DCT-II变换
 * @param data 输入数据
 * @return DCT系数
 *
 * DCT-II公式: X[k] = sum_{n=0}^{N-1} x[n] * cos(pi*(2n+1)*k / (2N))
 */
QVector<double> DCTFast4::forward(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalTransforms++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
        return result;
    }

    result = dctII(data);

    qint64 elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalPoints += data.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(data.size());
    return result;
}

/**
 * @brief 逆DCT-III变换
 * @param coeffs DCT系数
 * @return 重建的时域信号
 *
 * DCT-III公式: x[n] = (1/2)X[0] + sum_{k=1}^{N-1} X[k]*cos(pi*k*(2n+1)/(2N))
 */
QVector<double> DCTFast4::inverse(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    if (coeffs.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalTransforms++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
        return result;
    }

    result = dctIII(coeffs);

    qint64 elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalPoints += coeffs.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(coeffs.size());
    return result;
}

/**
 * @brief 二维DCT变换
 * @param data 输入二维矩阵
 * @return 二维DCT系数矩阵
 *
 * 先对每行执行DCT，再对每列执行DCT(行列可分离)。
 */
QVector<QVector<double>> DCTFast4::forward2D(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> result;
    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalTransforms++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
        return result;
    }

    int rows = data.size();
    int cols = data[0].size();

    /* 步骤1: 对每行执行DCT-II */
    QVector<QVector<double>> temp(rows);
    for (int r = 0; r < rows; ++r) {
        temp[r] = dctII(data[r]);
    }

    /* 步骤2: 对每列执行DCT-II */
    result.resize(rows);
    for (int r = 0; r < rows; ++r) {
        result[r].resize(cols, 0.0);
    }

    for (int c = 0; c < cols; ++c) {
        QVector<double> col(rows);
        for (int r = 0; r < rows; ++r) {
            col[r] = temp[r][c];
        }
        QVector<double> dctCol = dctII(col);
        for (int r = 0; r < rows; ++r) {
            result[r][c] = dctCol[r];
        }
    }

    qint64 elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalPoints += rows * cols;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(rows * cols);
    return result;
}

/**
 * @brief 重置统计信息
 */
void DCTFast4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief DCT-II直接计算
 * @param x 输入向量
 * @return DCT-II系数
 *
 * 直接计算DCT-II: X[k] = sum_{n=0}^{N-1} x[n] * cos(pi*(2n+1)*k / (2N))
 * 对于k=0的直流分量不乘归一化系数。
 */
QVector<double> DCTFast4::dctII(const QVector<double>& x)
{
    int N = x.size();
    QVector<double> X(N, 0.0);

    /* 预计算余弦表 */
    QVector<double> cosTable(N * N);
    for (int k = 0; k < N; ++k) {
        for (int n = 0; n < N; ++n) {
            cosTable[k * N + n] = qCos(M_PI * (2 * n + 1) * k / (2.0 * N));
        }
    }

    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n) {
            sum += x[n] * cosTable[k * N + n];
        }
        X[k] = sum;
    }

    return X;
}

/**
 * @brief DCT-III直接计算(逆变换)
 * @param X DCT系数
 * @return 重建的时域信号
 *
 * DCT-III: x[n] = (1/N) * [ X[0]/2 + sum_{k=1}^{N-1} X[k]*cos(pi*k*(2n+1)/(2N)) ]
 */
QVector<double> DCTFast4::dctIII(const QVector<double>& X)
{
    int N = X.size();
    QVector<double> x(N, 0.0);

    /* 预计算余弦表 */
    QVector<double> cosTable(N * N);
    for (int n = 0; n < N; ++n) {
        for (int k = 0; k < N; ++k) {
            cosTable[n * N + k] = qCos(M_PI * k * (2 * n + 1) / (2.0 * N));
        }
    }

    for (int n = 0; n < N; ++n) {
        double sum = X[0] * 0.5;
        for (int k = 1; k < N; ++k) {
            sum += X[k] * cosTable[n * N + k];
        }
        x[n] = sum * 2.0 / N;
    }

    return x;
}
