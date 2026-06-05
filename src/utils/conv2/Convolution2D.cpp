/**
 * @file Convolution2D.cpp
 * @brief 二维卷积实现 — 多种边界模式/可分离卷积
 */

#include "utils/conv2/Convolution2D.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
Convolution2D::Convolution2D(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_borderMode("reflect")
{
}

/** @brief 边界扩展取值 @param input 输入矩阵 @param row 行索引 @param col 列索引 @param mode 边界模式 @return 扩展后的值 */
double Convolution2D::borderValue(const QVector<QVector<double>>& input,
                                  int row, int col,
                                  const QString& mode) const
{
    int rows = input.size();
    int cols = input[0].size();

    /* 在范围内直接返回 */
    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        return input[row][col];
    }

    if (mode == "zero") {
        /* 零填充 */
        return 0.0;
    } else if (mode == "replicate") {
        /* 复制边界值 */
        int r = qBound(0, row, rows - 1);
        int c = qBound(0, col, cols - 1);
        return input[r][c];
    } else if (mode == "wrap") {
        /* 循环环绕 */
        int r = ((row % rows) + rows) % rows;
        int c = ((col % cols) + cols) % cols;
        return input[r][c];
    } else {
        /* reflect: 镜像反射(默认) */
        int r = row, c = col;
        if (r < 0) r = -r - 1;
        if (r >= rows) r = 2 * rows - r - 1;
        if (c < 0) c = -c - 1;
        if (c >= cols) c = 2 * cols - c - 1;
        r = qBound(0, r, rows - 1);
        c = qBound(0, c, cols - 1);
        return input[r][c];
    }
}

/** @brief 二维卷积 @param input 输入矩阵 @param kernel 卷积核 @param borderMode 边界模式 @return 卷积结果 */
QVector<QVector<double>> Convolution2D::convolve(
    const QVector<QVector<double>>& input,
    const QVector<QVector<double>>& kernel,
    const QString& borderMode)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty() || input[0].isEmpty() || kernel.isEmpty()
        || kernel[0].isEmpty()) {
        return {};
    }

    int rows = input.size();
    int cols = input[0].size();
    int kRows = kernel.size();
    int kCols = kernel[0].size();
    int kCenterY = kRows / 2;
    int kCenterX = kCols / 2;

    QVector<QVector<double>> result(rows, QVector<double>(cols, 0.0));

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            double sum = 0.0;
            for (int ky = 0; ky < kRows; ++ky) {
                for (int kx = 0; kx < kCols; ++kx) {
                    int iy = y + ky - kCenterY;
                    int ix = x + kx - kCenterX;
                    sum += kernel[ky][kx]
                           * borderValue(input, iy, ix, borderMode);
                }
            }
            result[y][x] = sum;
        }
    }

    m_timeSum += timer.elapsed();
    ++m_stats.totalConvolutions;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalConvolutions);

    emit convolutionCompleted(rows, cols);
    return result;
}

/** @brief 设置默认边界模式 @param mode 边界模式 */
void Convolution2D::setBorderMode(const QString& mode)
{
    if (mode == "reflect" || mode == "zero" || mode == "replicate"
        || mode == "wrap") {
        m_borderMode = mode;
    }
}

/** @brief 可分离卷积 @param input 输入矩阵 @param kernelX X方向1D核 @param kernelY Y方向1D核 @return 卷积结果 */
QVector<QVector<double>> Convolution2D::separableConvolve(
    const QVector<QVector<double>>& input,
    const QVector<double>& kernelX,
    const QVector<double>& kernelY)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty() || input[0].isEmpty()
        || kernelX.isEmpty() || kernelY.isEmpty()) {
        return {};
    }

    int rows = input.size();
    int cols = input[0].size();
    int kxLen = kernelX.size();
    int kyLen = kernelY.size();
    int kxC = kxLen / 2;
    int kyC = kyLen / 2;
    const QString& mode = m_borderMode;

    /* 第一步: 沿X方向卷积 */
    QVector<QVector<double>> temp(rows, QVector<double>(cols, 0.0));
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            double sum = 0.0;
            for (int k = 0; k < kxLen; ++k) {
                int ix = x + k - kxC;
                sum += kernelX[k] * borderValue(input, y, ix, mode);
            }
            temp[y][x] = sum;
        }
    }

    /* 第二步: 沿Y方向卷积 */
    QVector<QVector<double>> result(rows, QVector<double>(cols, 0.0));
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            double sum = 0.0;
            for (int k = 0; k < kyLen; ++k) {
                int iy = y + k - kyC;
                sum += kernelY[k] * borderValue(temp, iy, x, mode);
            }
            result[y][x] = sum;
        }
    }

    m_timeSum += timer.elapsed();
    ++m_stats.totalConvolutions;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalConvolutions);

    emit convolutionCompleted(rows, cols);
    return result;
}

/** @brief 重置统计 */
void Convolution2D::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
