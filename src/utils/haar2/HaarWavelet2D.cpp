/**
 * @file HaarWavelet2DEnhanced.cpp
 * @brief 二维Haar小波变换实现
 */

#include "utils/haar2/HaarWavelet2DEnhanced.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
HaarWavelet2DEnhanced::HaarWavelet2DEnhanced(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 正向Haar小波变换
 * @param image 二维输入数据(行/列须为2的幂)
 * @return 变换系数矩阵
 */
QVector<QVector<double>> HaarWavelet2DEnhanced::forward(
    const QVector<QVector<double>>& image)
{
    if (image.isEmpty() || image[0].isEmpty()) return {};

    m_timer.start();

    QVector<QVector<double>> data = padToPowerOfTwo(image);
    int rows = data.size();
    int cols = data[0].size();

    /* 多级分解 */
    int levels = m_levels;
    int maxLevels = static_cast<int>(qLn(std::max(rows, cols)) / qLn(2.0));
    if (levels > maxLevels) levels = maxLevels;

    int currentRows = rows;
    int currentCols = cols;

    for (int lev = 0; lev < levels; ++lev) {
        /* 先对当前区域的每行做Haar变换 */
        transformRows(data);

        /* 再对当前区域的每列做Haar变换 */
        transformCols(data);

        currentRows /= 2;
        currentCols /= 2;
    }

    /* 更新统计 */
    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalTransforms;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(data.size());
    return data;
}

/**
 * @brief 逆向Haar小波变换
 * @param coeffs 变换系数矩阵
 * @return 重构后的二维数据
 */
QVector<QVector<double>> HaarWavelet2DEnhanced::inverse(
    const QVector<QVector<double>>& coeffs)
{
    if (coeffs.isEmpty() || coeffs[0].isEmpty()) return {};

    m_timer.start();

    QVector<QVector<double>> data = coeffs;
    int rows = data.size();
    int cols = data[0].size();

    int levels = m_levels;
    int maxLevels = static_cast<int>(qLn(std::max(rows, cols)) / qLn(2.0));
    if (levels > maxLevels) levels = maxLevels;

    /* 逆序逐级重构 */
    int currentRows = rows;
    int currentCols = cols;
    for (int i = 1; i < levels; ++i) {
        currentRows /= 2;
        currentCols /= 2;
    }

    for (int lev = levels - 1; lev >= 0; --lev) {
        inverseCols(data);
        inverseRows(data);
        currentRows *= 2;
        currentCols *= 2;
    }

    /* 更新统计 */
    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalTransforms;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(data.size());
    return data;
}

/** @brief 设置分解级数 @param levels 级数 */
void HaarWavelet2DEnhanced::setLevels(int levels)
{
    m_levels = qMax(1, levels);
}

/** @brief 重置统计 */
void HaarWavelet2DEnhanced::resetStatistics()
{
    m_stats = Stats{};
    m_totalTimeMs = 0.0;
}

/**
 * @brief 单级行变换 — 对每行做Haar小波变换
 * 将每行的前半替换为平均值(L)，后半替换为差值(H)
 */
void HaarWavelet2DEnhanced::transformRows(QVector<QVector<double>>& data) const
{
    int rows = data.size();
    int halfCols = data[0].size() / 2;

    for (int r = 0; r < rows; ++r) {
        QVector<double> row = data[r];
        QVector<double> newRow(row.size());

        for (int c = 0; c < halfCols; ++c) {
            double a = row[2 * c];
            double b = row[2 * c + 1];
            /* 平均值(低频) */
            newRow[c] = (a + b) / qSqrt(2.0);
            /* 差值(高频) */
            newRow[halfCols + c] = (a - b) / qSqrt(2.0);
        }
        data[r] = newRow;
    }
}

/**
 * @brief 单级列变换 — 对每列做Haar小波变换
 */
void HaarWavelet2DEnhanced::transformCols(QVector<QVector<double>>& data) const
{
    int cols = data[0].size();
    int halfRows = data.size() / 2;

    for (int c = 0; c < cols; ++c) {
        QVector<double> col(data.size());
        for (int r = 0; r < data.size(); ++r) {
            col[r] = data[r][c];
        }

        for (int r = 0; r < halfRows; ++r) {
            double a = col[2 * r];
            double b = col[2 * r + 1];
            data[r][c] = (a + b) / qSqrt(2.0);
            data[halfRows + r][c] = (a - b) / qSqrt(2.0);
        }
    }
}

/**
 * @brief 单级行逆变换
 */
void HaarWavelet2DEnhanced::inverseRows(QVector<QVector<double>>& data) const
{
    int rows = data.size();
    int halfCols = data[0].size() / 2;

    for (int r = 0; r < rows; ++r) {
        QVector<double> row = data[r];
        QVector<double> newRow(row.size());

        for (int c = 0; c < halfCols; ++c) {
            double avg = row[c];
            double diff = row[halfCols + c];
            newRow[2 * c] = (avg + diff) / qSqrt(2.0);
            newRow[2 * c + 1] = (avg - diff) / qSqrt(2.0);
        }
        data[r] = newRow;
    }
}

/**
 * @brief 单级列逆变换
 */
void HaarWavelet2DEnhanced::inverseCols(QVector<QVector<double>>& data) const
{
    int cols = data[0].size();
    int halfRows = data.size() / 2;

    for (int c = 0; c < cols; ++c) {
        QVector<double> col(data.size());
        for (int r = 0; r < data.size(); ++r) {
            col[r] = data[r][c];
        }

        for (int r = 0; r < halfRows; ++r) {
            double avg = col[r];
            double diff = col[halfRows + r];
            data[2 * r][c] = (avg + diff) / qSqrt(2.0);
            data[2 * r + 1][c] = (avg - diff) / qSqrt(2.0);
        }
    }
}

/** @brief 检查是否为2的幂 */
bool HaarWavelet2DEnhanced::isPowerOfTwo(int n)
{
    return n > 0 && (n & (n - 1)) == 0;
}

/** @brief 向上取整到2的幂 */
int HaarWavelet2DEnhanced::nextPowerOfTwo(int n)
{
    if (n <= 0) return 1;
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/**
 * @brief 填充到2的幂尺寸
 * 右侧补零列，底部补零行
 */
QVector<QVector<double>> HaarWavelet2DEnhanced::padToPowerOfTwo(
    const QVector<QVector<double>>& data)
{
    int rows = data.size();
    int cols = data[0].size();
    int newRows = nextPowerOfTwo(rows);
    int newCols = nextPowerOfTwo(cols);

    if (newRows == rows && newCols == cols) return data;

    QVector<QVector<double>> padded(newRows, QVector<double>(newCols, 0.0));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            padded[r][c] = data[r][c];
        }
    }
    return padded;
}
