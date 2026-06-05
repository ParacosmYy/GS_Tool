/**
 * @file MorphologicalOps.cpp
 * @brief 二值形态学运算实现 — 腐蚀/膨胀/开闭/梯度
 */

#include "utils/morpho2/MorphologicalOps.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
MorphologicalOps::MorphologicalOps(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
    /* 默认3x3十字形结构元素 */
    m_defaultKernel = {{0, 1, 0},
                       {1, 1, 1},
                       {0, 1, 0}};
}

/** @brief 内部腐蚀实现 @param input 输入矩阵 @param kernel 结构元素 @return 腐蚀结果 */
QVector<QVector<int>> MorphologicalOps::erodeImpl(
    const QVector<QVector<int>>& input,
    const QVector<QVector<int>>& kernel) const
{
    if (input.isEmpty() || input[0].isEmpty()) return {};
    if (kernel.isEmpty() || kernel[0].isEmpty()) return input;

    int rows = input.size();
    int cols = input[0].size();
    int kRows = kernel.size();
    int kCols = kernel[0].size();
    int kCenterY = kRows / 2;
    int kCenterX = kCols / 2;

    QVector<QVector<int>> result(rows, QVector<int>(cols, 0));

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            bool allMatch = true;
            for (int ky = 0; ky < kRows && allMatch; ++ky) {
                for (int kx = 0; kx < kCols && allMatch; ++kx) {
                    if (kernel[ky][kx] == 0) continue;
                    int iy = y + ky - kCenterY;
                    int ix = x + kx - kCenterX;
                    /* 超出边界视为0 */
                    if (iy < 0 || iy >= rows || ix < 0 || ix >= cols
                        || input[iy][ix] == 0) {
                        allMatch = false;
                    }
                }
            }
            result[y][x] = allMatch ? 1 : 0;
        }
    }
    return result;
}

/** @brief 内部膨胀实现 @param input 输入矩阵 @param kernel 结构元素 @return 膨胀结果 */
QVector<QVector<int>> MorphologicalOps::dilateImpl(
    const QVector<QVector<int>>& input,
    const QVector<QVector<int>>& kernel) const
{
    if (input.isEmpty() || input[0].isEmpty()) return {};
    if (kernel.isEmpty() || kernel[0].isEmpty()) return input;

    int rows = input.size();
    int cols = input[0].size();
    int kRows = kernel.size();
    int kCols = kernel[0].size();
    int kCenterY = kRows / 2;
    int kCenterX = kCols / 2;

    QVector<QVector<int>> result(rows, QVector<int>(cols, 0));

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            bool anyMatch = false;
            for (int ky = 0; ky < kRows && !anyMatch; ++ky) {
                for (int kx = 0; kx < kCols && !anyMatch; ++kx) {
                    if (kernel[ky][kx] == 0) continue;
                    int iy = y + ky - kCenterY;
                    int ix = x + kx - kCenterX;
                    if (iy >= 0 && iy < rows && ix >= 0 && ix < cols
                        && input[iy][ix] != 0) {
                        anyMatch = true;
                    }
                }
            }
            result[y][x] = anyMatch ? 1 : 0;
        }
    }
    return result;
}

/** @brief 腐蚀运算 @param binary 二值输入 @param kernel 结构元素 @return 腐蚀结果 */
QVector<QVector<int>> MorphologicalOps::erode(
    const QVector<QVector<int>>& binary,
    const QVector<QVector<int>>& kernel)
{
    QElapsedTimer timer;
    timer.start();

    auto result = erodeImpl(binary, kernel);

    m_timeSum += timer.elapsed();
    ++m_stats.totalOperations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    if (!result.isEmpty()) {
        emit operationCompleted(tr("erode"), result.size(), result[0].size());
    }
    return result;
}

/** @brief 膨胀运算 @param binary 二值输入 @param kernel 结构元素 @return 膨胀结果 */
QVector<QVector<int>> MorphologicalOps::dilate(
    const QVector<QVector<int>>& binary,
    const QVector<QVector<int>>& kernel)
{
    QElapsedTimer timer;
    timer.start();

    auto result = dilateImpl(binary, kernel);

    m_timeSum += timer.elapsed();
    ++m_stats.totalOperations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    if (!result.isEmpty()) {
        emit operationCompleted(tr("dilate"), result.size(), result[0].size());
    }
    return result;
}

/** @brief 开运算 @param binary 二值输入 @param kernel 结构元素 @return 开运算结果 */
QVector<QVector<int>> MorphologicalOps::open(
    const QVector<QVector<int>>& binary,
    const QVector<QVector<int>>& kernel)
{
    QElapsedTimer timer;
    timer.start();

    /* 先腐蚀再膨胀 */
    auto eroded = erodeImpl(binary, kernel);
    auto result = dilateImpl(eroded, kernel);

    m_timeSum += timer.elapsed();
    ++m_stats.totalOperations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    if (!result.isEmpty()) {
        emit operationCompleted(tr("open"), result.size(), result[0].size());
    }
    return result;
}

/** @brief 闭运算 @param binary 二值输入 @param kernel 结构元素 @return 闭运算结果 */
QVector<QVector<int>> MorphologicalOps::close(
    const QVector<QVector<int>>& binary,
    const QVector<QVector<int>>& kernel)
{
    QElapsedTimer timer;
    timer.start();

    /* 先膨胀再腐蚀 */
    auto dilated = dilateImpl(binary, kernel);
    auto result = erodeImpl(dilated, kernel);

    m_timeSum += timer.elapsed();
    ++m_stats.totalOperations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    if (!result.isEmpty()) {
        emit operationCompleted(tr("close"), result.size(), result[0].size());
    }
    return result;
}

/** @brief 形态学梯度 @param binary 二值输入 @param kernel 结构元素 @return 梯度结果 */
QVector<QVector<int>> MorphologicalOps::gradient(
    const QVector<QVector<int>>& binary,
    const QVector<QVector<int>>& kernel)
{
    QElapsedTimer timer;
    timer.start();

    auto dilated = dilateImpl(binary, kernel);
    auto eroded = erodeImpl(binary, kernel);

    /* 梯度 = 膨胀 - 腐蚀 */
    int rows = binary.size();
    int cols = binary.isEmpty() ? 0 : binary[0].size();
    QVector<QVector<int>> result(rows, QVector<int>(cols, 0));

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            result[y][x] = dilated[y][x] - eroded[y][x];
        }
    }

    m_timeSum += timer.elapsed();
    ++m_stats.totalOperations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOperations);

    if (!result.isEmpty()) {
        emit operationCompleted(tr("gradient"), result.size(), result[0].size());
    }
    return result;
}

/** @brief 设置默认结构元素 @param k 结构元素矩阵 */
void MorphologicalOps::setKernel(const QVector<QVector<int>>& k)
{
    m_defaultKernel = k;
}

/** @brief 重置统计 */
void MorphologicalOps::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
