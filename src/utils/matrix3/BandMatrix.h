/**
 * @file BandMatrix.h
 * @brief 带状矩阵 — 紧凑存储与高效运算
 *
 * 功能: 带状矩阵的高效紧凑存储格式，支持元素的读写、
 *       矩阵向量乘法以及基于高斯消去的线性方程组求解。
 *       存储复杂度O(N*(kl+ku+1))而非O(N^2)。
 *
 * 协作: TridiagonalSolver(三对角求解) / DataInterpolator(样条)
 * @author Serial Tool Team
 * @date 2026-06-05
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QtGlobal>

/**
 * @class BandMatrix
 * @brief 带状矩阵 — 紧凑存储与运算
 *
 * 使用带状存储格式(Banded Storage)，仅存储下带宽kl、
 * 主对角线和上带宽ku范围内的元素。支持标量读写、
 * 矩阵向量乘法、以及带状高斯消去求解。
 */
class BandMatrix : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalOperations = 0;        ///< 累计运算次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param size 矩阵维度(方阵)
     * @param lowerBandwidth 下带宽kl
     * @param upperBandwidth 上带宽ku
     * @param parent 父对象
     */
    explicit BandMatrix(int size, int lowerBandwidth, int upperBandwidth,
                        QObject *parent = nullptr);

    /**
     * @brief 设置矩阵元素值
     * @param row 行索引(0-based)
     * @param col 列索引(0-based)
     * @param value 元素值
     */
    void setValue(int row, int col, double value);

    /**
     * @brief 获取矩阵元素值
     * @param row 行索引(0-based)
     * @param col 列索引(0-based)
     * @return 元素值，带外元素返回0.0
     */
    double value(int row, int col) const;

    /**
     * @brief 矩阵-向量乘法 y = A*x
     * @param vec 输入向量(长度=size)
     * @return 乘积向量
     */
    QVector<double> multiply(const QVector<double> &vec) const;

    /**
     * @brief 求解线性方程组 A*x = b
     * @param rhs 右端项向量(长度=size)
     * @return 解向量，失败返回空
     */
    QVector<double> solve(const QVector<double> &rhs);

    /** @brief 获取矩阵维度 @return 维度 */
    int size() const { return m_size; }

    /** @brief 获取下带宽 @return 下带宽 */
    int lowerBandwidth() const { return m_kl; }

    /** @brief 获取上带宽 @return 上带宽 */
    int upperBandwidth() const { return m_ku; }

    /** @brief 获取统计信息 @return 常量引用 */
    const Stats &stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 @param size 方程组规模 @param timeMs 耗时 */
    void solveCompleted(int size, double timeMs);

private:
    int m_size;                 ///< 矩阵维度
    int m_kl;                   ///< 下带宽
    int m_ku;                   ///< 上带宽
    int m_bandwidth;            ///< 总带宽 = kl + ku + 1
    /**
     * 紧凑存储: m_data[row * m_bandwidth + (col - row + m_ku)]
     * 行优先存储每行的带宽内元素
     */
    QVector<double> m_data;     ///< 紧凑存储数据

    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 处理时间累加器
};
