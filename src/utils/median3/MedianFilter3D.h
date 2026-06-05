/**
 * @file MedianFilter3D.h
 * @brief 三维中值滤波器 — 体数据降噪
 *
 * 功能: 对三维体数据执行中值滤波，支持可变核大小，
 *       统计滤波次数/体素数/耗时。
 */
#pragma once

#include <QObject>
#include <QVector>

class MedianFilter3D : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalFiltered = 0;      ///< 总滤波次数
        quint64 totalVoxels = 0;        ///< 总处理体素数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit MedianFilter3D(QObject* parent = nullptr);

    /**
     * @brief 对三维体数据进行中值滤波
     * @param volume 输入体数据 [depth][rows][cols]
     * @param kernelSize 核大小(必须为奇数,如3/5/7)
     * @return 滤波后的体数据
     */
    QVector<QVector<QVector<double>>> filter(
        const QVector<QVector<QVector<double>>>& volume, int kernelSize);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 滤波完成信号 @param voxels 处理的体素总数 */
    void filteringCompleted(int voxels);

private:
    /** @brief 从体数据中提取核窗口的中值 */
    double extractMedian(
        const QVector<QVector<QVector<double>>>& volume,
        int z, int y, int x, int ksz,
        int depth, int rows, int cols) const;

    Stats  m_stats;
    double m_timeSum;
};
