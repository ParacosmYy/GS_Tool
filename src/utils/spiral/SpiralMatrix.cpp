/**
 * @file SpiralMatrix.cpp
 * @brief 螺旋矩阵生成器实现
 */

#include "SpiralMatrix.h"
#include <QElapsedTimer>

SpiralMatrix::SpiralMatrix(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<QVector<int>> SpiralMatrix::generate(int rows, int cols, Direction dir)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> matrix(rows, QVector<int>(cols, 0));
    if (rows <= 0 || cols <= 0) return matrix;

    int top = 0, bottom = rows - 1, left = 0, right = cols - 1;
    int val = 1;

    while (top <= bottom && left <= right) {
        if (dir == Clockwise) {
            for (int c = left; c <= right; ++c) matrix[top][c] = val++;
            top++;
            for (int r = top; r <= bottom; ++r) matrix[r][right] = val++;
            right--;
            if (top <= bottom) {
                for (int c = right; c >= left; --c) matrix[bottom][c] = val++;
                bottom--;
            }
            if (left <= right) {
                for (int r = bottom; r >= top; --r) matrix[r][left] = val++;
                left++;
            }
        } else {
            for (int r = top; r <= bottom; ++r) matrix[r][left] = val++;
            left++;
            for (int c = left; c <= right; ++c) matrix[bottom][c] = val++;
            bottom--;
            if (left <= right) {
                for (int r = bottom; r >= top; --r) matrix[r][right] = val++;
                right--;
            }
            if (top <= bottom) {
                for (int c = right; c >= left; --c) matrix[top][c] = val++;
                top++;
            }
        }
    }

    m_stats.totalGenerated++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerated;

    emit generationCompleted(rows, cols);
    return matrix;
}

QVector<QPoint> SpiralMatrix::spiralPath(int rows, int cols, Direction dir)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPoint> path;
    if (rows <= 0 || cols <= 0) return path;

    int top = 0, bottom = rows - 1, left = 0, right = cols - 1;

    while (top <= bottom && left <= right) {
        if (dir == Clockwise) {
            for (int c = left; c <= right; ++c) path.append(QPoint(top, c));
            top++;
            for (int r = top; r <= bottom; ++r) path.append(QPoint(r, right));
            right--;
            if (top <= bottom) {
                for (int c = right; c >= left; --c) path.append(QPoint(bottom, c));
                bottom--;
            }
            if (left <= right) {
                for (int r = bottom; r >= top; --r) path.append(QPoint(r, left));
                left++;
            }
        } else {
            for (int r = top; r <= bottom; ++r) path.append(QPoint(r, left));
            left++;
            for (int c = left; c <= right; ++c) path.append(QPoint(bottom, c));
            bottom--;
            if (left <= right) {
                for (int r = bottom; r >= top; --r) path.append(QPoint(r, right));
                right--;
            }
            if (top <= bottom) {
                for (int c = right; c >= left; --c) path.append(QPoint(top, c));
                top++;
            }
        }
    }

    m_stats.totalGenerated++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerated;

    return path;
}

QPoint SpiralMatrix::search(int rows, int cols, int value)
{
    QElapsedTimer timer;
    timer.start();

    QPoint result(-1, -1);
    int maxVal = rows * cols;
    if (value < 1 || value > maxVal) {
        m_stats.totalSearched++;
        m_timeSum += timer.elapsed();
        int total = m_stats.totalGenerated + m_stats.totalSearched;
        if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;
        return result;
    }

    int top = 0, bottom = rows - 1, left = 0, right = cols - 1;
    int val = 1;

    while (top <= bottom && left <= right) {
        for (int c = left; c <= right; ++c) {
            if (val == value) { result = QPoint(top, c); break; }
            val++;
        }
        if (result.x() >= 0) break;
        top++;
        for (int r = top; r <= bottom; ++r) {
            if (val == value) { result = QPoint(r, right); break; }
            val++;
        }
        if (result.x() >= 0) break;
        right--;
        if (top <= bottom) {
            for (int c = right; c >= left; --c) {
                if (val == value) { result = QPoint(bottom, c); break; }
                val++;
            }
            if (result.x() >= 0) break;
            bottom--;
        }
        if (left <= right) {
            for (int r = bottom; r >= top; --r) {
                if (val == value) { result = QPoint(r, left); break; }
                val++;
            }
            if (result.x() >= 0) break;
            left++;
        }
    }

    m_stats.totalSearched++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalGenerated + m_stats.totalSearched;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

QVector<QVector<int>> SpiralMatrix::ulamSpiral(int size)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> result(size, QVector<int>(size, 0));
    if (size <= 0) return result;

    int cx = size / 2, cy = size / 2;
    int dx = 1, dy = 0;
    int steps = 1, stepCount = 0, turnCount = 0;

    for (int n = 1; n <= size * size; ++n) {
        if (cx >= 0 && cx < size && cy >= 0 && cy < size)
            result[cy][cx] = isPrime(n) ? 1 : 0;

        cx += dx; cy += dy;
        stepCount++;

        if (stepCount >= steps) {
            stepCount = 0;
            int tmp = dx; dx = -dy; dy = tmp;
            turnCount++;
            if (turnCount >= 2) { turnCount = 0; steps++; }
        }
    }

    m_stats.totalGenerated++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerated;

    return result;
}

QPoint SpiralMatrix::spiralCoordinate(int index)
{
    if (index < 0) return QPoint(0, 0);

    /* 确定所在的环 */
    int ring = 0;
    int sideLen = 1;
    int totalInRings = 1;

    while (index >= totalInRings + 4 * sideLen) {
        ring++;
        sideLen += 2;
        totalInRings += 4 * (sideLen - 2);
    }

    if (index == 0) return QPoint(0, 0);

    int offset = index - totalInRings;
    int perSide = sideLen - 1;
    int side = offset / perSide;
    int pos = offset % perSide;

    int x, y;
    switch (side) {
    case 0: x = ring - pos; y = -ring; break;
    case 1: x = -ring; y = -ring + 1 + pos; break;
    case 2: x = -ring + 1 + pos; y = ring; break;
    default: x = ring; y = ring - 1 - pos; break;
    }
    return QPoint(x, y);
}

bool SpiralMatrix::isPrime(int n) const
{
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

SpiralMatrix::Stats SpiralMatrix::stats() const { return m_stats; }

void SpiralMatrix::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
