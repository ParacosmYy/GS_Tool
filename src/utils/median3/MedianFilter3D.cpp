/**
 * @file MedianFilter3D.cpp
 * @brief 三维中值滤波器实现
 */

#include "utils/median3/MedianFilter3D.h"

#include <QElapsedTimer>
#include <algorithm>

MedianFilter3D::MedianFilter3D(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<QVector<QVector<double>>> MedianFilter3D::filter(
    const QVector<QVector<QVector<double>>>& volume, int kernelSize)
{
    QElapsedTimer timer;
    timer.start();

    int depth = volume.size();
    if (depth == 0) return {};
    int rows = volume[0].size();
    if (rows == 0) return {};
    int cols = volume[0][0].size();
    if (cols == 0) return {};

    /* 核大小必须为正奇数 */
    int ksz = qMax(1, kernelSize);
    if (ksz % 2 == 0) ksz += 1;
    int half = ksz / 2;

    /* 分配输出体数据 */
    QVector<QVector<QVector<double>>> output(depth);
    for (int z = 0; z < depth; ++z) {
        output[z].resize(rows);
        for (int y = 0; y < rows; ++y) {
            output[z][y].resize(cols, 0.0);
        }
    }

    /* 遍历每个体素 */
    qint64 voxelCount = 0;
    for (int z = 0; z < depth; ++z) {
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                output[z][y][x] = extractMedian(volume, z, y, x,
                                                 ksz, depth, rows, cols);
                voxelCount++;
            }
        }
    }

    /* 更新统计 */
    m_stats.totalFiltered++;
    m_stats.totalVoxels += voxelCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFiltered;

    emit filteringCompleted(static_cast<int>(voxelCount));
    return output;
}

double MedianFilter3D::extractMedian(
    const QVector<QVector<QVector<double>>>& volume,
    int z, int y, int x, int ksz,
    int depth, int rows, int cols) const
{
    int half = ksz / 2;
    QVector<double> window;
    window.reserve(ksz * ksz * ksz);

    for (int dz = -half; dz <= half; ++dz) {
        int zz = z + dz;
        if (zz < 0 || zz >= depth) continue;
        for (int dy = -half; dy <= half; ++dy) {
            int yy = y + dy;
            if (yy < 0 || yy >= rows) continue;
            for (int dx = -half; dx <= half; ++dx) {
                int xx = x + dx;
                if (xx < 0 || xx >= cols) continue;
                window.append(volume[zz][yy][xx]);
            }
        }
    }

    if (window.isEmpty()) return 0.0;

    /* 使用nth_element高效找中值 */
    int n = window.size();
    int mid = n / 2;
    std::nth_element(window.begin(), window.begin() + mid, window.end());

    if (n % 2 == 1) return window[mid];
    /* 偶数取中间两个平均值 */
    double a = window[mid];
    std::nth_element(window.begin(), window.begin() + mid - 1,
                     window.begin() + mid);
    return (a + window[mid - 1]) / 2.0;
}

void MedianFilter3D::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
