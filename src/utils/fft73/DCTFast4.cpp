/**
 * @file DCTFast4.cpp
 * @brief 快速离散余弦变换(DCT)实现
 *
 * 实现DCT-II和DCT-III的快速算法，支持1D和2D变换。
 * DCT广泛用于图像压缩(JPEG)和音频编码。
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
 * @param n 变换大小
 */
void DCTFast4::setSize(int n)
{
    m_n = qMax(4, n);
}

/**
 * @brief 前向DCT-II变换
 * @param data 输入数据
 * @return DCT系数
 */
QVector<double> DCTFast4::forward(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result = dctII(data);

    // 更新统计信息
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
 */
QVector<double> DCTFast4::inverse(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result = dctIII(coeffs);

    m_timeSum += timer.elapsed();
    return result;
}

/**
 * @brief 2D DCT变换
 * @param data 输入2D数据矩阵
 * @return 2D DCT系数矩阵
 */
QVector<QVector<double>> DCTFast4::forward2D(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return QVector<QVector<double>>();

    int rows = data.size();
    int cols = data[0].size();

    // 先对每行做1D DCT
    QVector<QVector<double>> temp(rows);
    for (int i = 0; i < rows; ++i) {
        temp[i] = dctII(data[i]);
    }

    // 再对每列做1D DCT
    QVector<QVector<double>> result(rows, QVector<double>(cols, 0.0));
    for (int j = 0; j < cols; ++j) {
        QVector<double> col(rows);
        for (int i = 0; i < rows; ++i) col[i] = temp[i][j];
        QVector<double> colDCT = dctII(col);
        for (int i = 0; i < rows; ++i) result[i][j] = colDCT[i];
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalPoints += rows * cols;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

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
 * @brief DCT-II实现
 * @param x 输入序列
 * @return DCT-II系数
 *
 * X[k] = sum_{n=0}^{N-1} x[n] * cos(pi*(2n+1)*k/(2N))
 */
QVector<double> DCTFast4::dctII(const QVector<double>& x)
{
    int N = x.size();
    QVector<double> X(N, 0.0);

    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n) {
            sum += x[n] * qCos(M_PI * (2 * n + 1) * k / (2.0 * N));
        }
        X[k] = sum;
    }

    // 第一个系数缩放（正交归一化）
    X[0] *= 1.0 / qSqrt(static_cast<double>(N));
    double scale = qSqrt(2.0 / N);
    for (int k = 1; k < N; ++k) {
        X[k] *= scale;
    }

    return X;
}

/**
 * @brief DCT-III实现（逆DCT）
 * @param X 输入DCT系数
 * @return 时域序列
 *
 * x[n] = (1/2)X[0] + sum_{k=1}^{N-1} X[k] * cos(pi*k*(2n+1)/(2N))
 */
QVector<double> DCTFast4::dctIII(const QVector<double>& X)
{
    int N = X.size();
    QVector<double> x(N, 0.0);
    double scale = qSqrt(2.0 / N);
    double scale0 = 1.0 / qSqrt(static_cast<double>(N));

    for (int n = 0; n < N; ++n) {
        double sum = scale0 * X[0];
        for (int k = 1; k < N; ++k) {
            sum += scale * X[k] * qCos(M_PI * k * (2 * n + 1) / (2.0 * N));
        }
        x[n] = sum;
    }

    return x;
}
