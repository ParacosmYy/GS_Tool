/**
 * @file SpiralMatrix.h
 * @brief 螺旋矩阵生成器(Spiral Matrix Generator)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPoint>

/**
 * @class SpiralMatrix
 * @brief 螺旋矩阵生成器 — 生成/遍历/搜索螺旋路径
 *
 * 支持多种螺旋模式(顺时针/逆时针/双螺旋)、坐标转换、
 * 矩阵填充、路径搜索。
 * 适用于矩阵可视化、扫描模式生成、空间索引等场景。
 */
class SpiralMatrix : public QObject
{
    Q_OBJECT

public:
    /** @brief 螺旋方向 */
    enum Direction {
        Clockwise,       /**< 顺时针 */
        CounterClockwise /**< 逆时针 */
    };
    Q_ENUM(Direction)

    /** @brief 统计信息 */
    struct Stats {
        int totalGenerated = 0;    /**< 总生成次数 */
        int totalSearched = 0;     /**< 总搜索次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit SpiralMatrix(QObject* parent = nullptr);

    /**
     * @brief 生成螺旋矩阵
     * @param rows 行数
     * @param cols 列数
     * @param dir 螺旋方向
     * @return 矩阵(rows×cols), 值从1开始
     */
    QVector<QVector<int>> generate(int rows, int cols,
                                     Direction dir = Clockwise);

    /**
     * @brief 获取螺旋遍历坐标序列
     * @param rows 行数
     * @param cols 列数
     * @param dir 螺旋方向
     * @return 坐标列表(row, col)
     */
    QVector<QPoint> spiralPath(int rows, int cols,
                                Direction dir = Clockwise);

    /**
     * @brief 在螺旋矩阵中搜索值的位置
     * @param rows 行数
     * @param cols 列数
     * @param value 目标值
     * @return 坐标(row, col), 未找到返回(-1,-1)
     */
    QPoint search(int rows, int cols, int value);

    /**
     * @brief 生成Ulam螺旋素数矩阵
     * @param size 矩阵大小(size×size)
     * @return 矩阵, 素数位置为1, 非素数为0
     */
    QVector<QVector<int>> ulamSpiral(int size);

    /**
     * @brief 从中心向外的螺旋坐标
     * @param index 序号(从0开始)
     * @return 坐标偏移(相对于中心)
     */
    static QPoint spiralCoordinate(int index);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 生成完成信号 */
    void generationCompleted(int rows, int cols);

private:
    bool isPrime(int n) const;

    Stats m_stats;
    double m_timeSum;
};
