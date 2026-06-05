/**
 * @file WatershedSegmentation.h
 * @brief 分水岭分割 — 基于沉浸模型的图像/二维数据分割
 */
#pragma once

#include <QObject>
#include <QVector>
#include <queue>
#include <utility>
#include <vector>

/**
 * @class WatershedSegmentation
 * @brief 基于沉浸（flooding）模型的分水岭算法
 *
 * 将二维高程数据（灰度图/矩阵）分割为不同区域。
 * 使用优先队列按高程从低到高"淹没"，形成分水岭边界。
 */
class WatershedSegmentation : public QObject {
    Q_OBJECT
public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalSegmented = 0;          ///< 总分割次数
        quint64 totalRegions = 0;            ///< 总区域数
        double  avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit WatershedSegmentation(QObject* parent = nullptr);

    /**
     * @brief 对高程数据进行分水岭分割
     * @param elevation 二维高程矩阵 [rows][cols]
     * @return 标记矩阵，每个像素值为区域编号（0表示分水岭边界）
     */
    QVector<QVector<int>> segment(const QVector<QVector<double>>& elevation);

    /** @brief 获取最近一次分割的区域数量 */
    int regionCount() const { return m_regionCount; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 分割完成信号 @param regionCount 区域数量 */
    void segmentationCompleted(int regionCount);

private:
    /** @brief 像素点，含坐标和高程值 */
    struct Pixel {
        int    r, c;
        double val;
    };

    Stats  m_stats;
    double m_timeSum = 0.0;      ///< 累计处理时间
    int    m_regionCount = 0;    ///< 最近区域数

    static constexpr int dx[4] = {-1, 1, 0, 0};
    static constexpr int dy[4] = {0, 0, -1, 1};

    /** @brief 判断坐标是否在矩阵范围内 */
    bool inBounds(int r, int c, int rows, int cols) const;

    /** @brief 收集并按高程排序所有像素 */
    std::vector<Pixel> collectSortedPixels(
        const QVector<QVector<double>>& elevation) const;

    /** @brief 处理同一高程层的邻域标记 */
    void processLevelNeighbors(
        const std::vector<Pixel>& pixels,
        const std::vector<size_t>& levelIndices,
        QVector<QVector<int>>& labels,
        std::vector<std::vector<int>>& dist,
        std::queue<std::pair<int, int>>& queue,
        int rows, int cols, int wshed) const;

    /** @brief BFS扩展已标记区域 */
    void bfsExpand(
        QVector<QVector<int>>& labels,
        std::vector<std::vector<int>>& dist,
        std::queue<std::pair<int, int>>& queue,
        int rows, int cols, int wshed) const;

    /** @brief BFS标记新区域 */
    void bfsLabelNewRegion(
        QVector<QVector<int>>& labels,
        std::queue<std::pair<int, int>>& queue,
        int regionId, int rows, int cols) const;

    /** @brief 对当前层未标记像素创建新区域 */
    void createNewRegions(
        const std::vector<Pixel>& pixels,
        const std::vector<size_t>& levelIndices,
        QVector<QVector<int>>& labels,
        std::vector<std::vector<int>>& dist,
        int& currentLabel, int rows, int cols) const;
};
