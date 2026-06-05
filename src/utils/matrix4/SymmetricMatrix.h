/**
 * @file SymmetricMatrix.h
 * @brief 对称矩阵 — 紧凑存储+Jacobi特征值分解
 *
 * 功能: 使用一维紧凑数组存储n阶对称矩阵(仅存下三角)，
 *       支持元素读写、矩阵-向量乘法、Jacobi旋转法求
 *       全部特征值、行列式计算。适用于协方差矩阵分析、
 *       主成分分析(PCA)、最小二乘法等场景。
 *
 * 协作: DataCorrelator(相关性矩阵) / PCA(主成分分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief 对称矩阵 — 紧凑存储与线性代数运算
 *
 * 仅存储下三角部分，节省约50%内存。
 * 所有(i,j)访问自动映射到线性索引。
 */
class SymmetricMatrix : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalOperations = 0;        ///< 累计运算次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param size 矩阵维度(n x n)
     * @param parent 父对象
     */
    explicit SymmetricMatrix(int size = 0, QObject* parent = nullptr);

    /**
     * @brief 设置矩阵元素值(自动对称)
     * @param i 行索引(0-based)
     * @param j 列索引(0-based)
     * @param value 元素值
     */
    void setValue(int i, int j, double value);

    /**
     * @brief 获取矩阵元素值
     * @param i 行索引
     * @param j 列索引
     * @return 元素值
     */
    double value(int i, int j) const;

    /**
     * @brief 矩阵-向量乘法 y = A * vec
     * @param vec 输入向量(长度必须等于矩阵维度)
     * @return 乘积向量
     */
    QVector<double> multiply(const QVector<double>& vec) const;

    /**
     * @brief 计算全部特征值(Jacobi旋转法)
     * @return 特征值列表(降序排列)
     */
    QVector<double> eigenvalues() const;

    /**
     * @brief 计算行列式(基于特征值的乘积)
     * @return 行列式值
     */
    double determinant() const;

    /** @brief 获取矩阵维度 */
    int size() const { return m_size; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 运算完成信号 @param opType 运算类型 */
    void operationCompleted(const QString& opType);

private:
    /**
     * @brief 将(i,j)映射到紧凑存储索引
     * @param i 行索引
     * @param j 列索引
     * @return 线性索引
     */
    int index(int i, int j) const;

    int    m_size;              ///< 矩阵维度
    QVector<double> m_data;    ///< 紧凑存储数据(下三角)

    mutable QElapsedTimer m_timer;   ///< 计时器
    mutable double  m_timeSum;       ///< 累计耗时
    mutable Stats   m_stats;         ///< 统计信息
};
