/**
 * @file HilbertCurve.h
 * @brief Hilbert空间填充曲线
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class HilbertCurve
 * @brief Hilbert曲线 — 将多维数据映射到一维保持空间局部性
 *
 * 支持任意阶数(2^n × 2^n网格)的Hilbert曲线编码/解码。
 * 用于空间索引、数据库聚类、图像遍历等场景。
 */
class HilbertCurve : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalEncoded = 0;       /**< 总编码次数 */
        int totalDecoded = 0;       /**< 总解码次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param order 曲线阶数(默认4，即16×16网格)
     * @param parent 父对象
     */
    explicit HilbertCurve(int order = 4, QObject* parent = nullptr);

    /**
     * @brief 2D坐标 → Hilbert索引
     * @param x X坐标
     * @param y Y坐标
     * @return Hilbert曲线索引
     */
    int encode(int x, int y) const;

    /**
     * @brief Hilbert索引 → 2D坐标
     * @param index Hilbert索引
     * @return (x, y)坐标
     */
    QPair<int, int> decode(int index) const;

    /**
     * @brief 批量编码
     * @param points 坐标列表
     * @return 索引列表
     */
    QVector<int> encodeBatch(const QVector<QPair<int, int>>& points) const;

    /**
     * @brief 批量解码
     * @param indices 索引列表
     * @return 坐标列表
     */
    QVector<QPair<int, int>> decodeBatch(const QVector<int>& indices) const;

    /**
     * @brief 计算两个索引在曲线上的距离
     * @param idx1 第一个索引
     * @param idx2 第二个索引
     * @return 曲线距离
     */
    int curveDistance(int idx1, int idx2) const;

    /**
     * @brief 获取曲线上指定范围内的坐标序列
     * @param fromIdx 起始索引
     * @param toIdx 结束索引
     * @return 坐标序列
     */
    QVector<QPair<int, int>> curveRange(int fromIdx, int toIdx) const;

    /** @brief 获取网格大小 */
    int gridSize() const;

    /** @brief 获取阶数 */
    int order() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 */
    void encodeCompleted(int x, int y, int index);

private:
    void rotate(int n, int* x, int* y, int rx, int ry) const;

    int m_order;                /**< 曲线阶数 */
    mutable Stats m_stats;      /**< 统计信息 */
    mutable double m_timeSum;   /**< 累计时间 */
};
