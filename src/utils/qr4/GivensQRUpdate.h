/**
 * @file GivensQRUpdate.h
 * @brief 增量QR分解更新 — Givens旋转插入/删除行
 *
 * 在已有QR分解基础上, 高效插入或删除行而不需要重新分解。
 * 适用于递归最小二乘(RLS)、滑动窗口回归等增量场景。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class GivensQRUpdate
 * @brief 增量QR更新 — 行插入/删除通过Givens旋转
 *
 * 维护上三角矩阵R, 插入行时通过Givens旋转恢复上三角结构,
 * 删除行时从R中恢复对应信息。
 */
class GivensQRUpdate : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInsertions = 0;    ///< 总行插入次数
        quint64 totalDeletions = 0;     ///< 总行删除次数
        double  avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit GivensQRUpdate(QObject* parent = nullptr);

    /**
     * @brief 初始化QR分解(首次完整分解)
     * @param matrix 输入矩阵(行优先, rows x cols)
     * @param rows 行数
     * @param cols 列数
     * @return 上三角矩阵R (cols x cols)
     */
    QVector<double> initialize(const QVector<double>& matrix,
                               int rows, int cols);

    /**
     * @brief 插入一行到QR分解
     * @param R 当前上三角矩阵 (n x n)
     * @param newRow 要插入的行向量(长度n)
     * @return 更新后的R矩阵 (n x n)
     */
    QVector<double> insertRow(const QVector<double>& R,
                              const QVector<double>& newRow);

    /**
     * @brief 删除一行(恢复上三角)
     * @param R 当前上三角矩阵 (n x n)
     * @param rowIdx 要删除的行索引
     * @param removedRow 原始被删除行的数据
     * @return 更新后的R矩阵 (n x n)
     */
    QVector<double> deleteRow(const QVector<double>& R,
                              int rowIdx,
                              const QVector<double>& removedRow);

    /**
     * @brief 求解最小二乘 Rx = b
     * @param R 上三角矩阵
     * @param b 右侧向量
     * @param n 维度
     * @return 解向量x
     */
    QVector<double> solveUpperTriangular(const QVector<double>& R,
                                         const QVector<double>& b,
                                         int n) const;

    /**
     * @brief 计算Givens旋转参数
     * @param a 第一个元素
     * @param b 第二个元素
     * @return (cos, sin, 旋转后的r)
     */
    static QPair<QPair<double, double>, double> givensRotation(double a, double b);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 行插入完成 @param rowsUpdated 更新行数 */
    void rowInserted(int rowsUpdated);
    /** @brief 行删除完成 @param rowIdx 删除行索引 */
    void rowDeleted(int rowIdx);

private:
    /** @brief 对R的指定列应用Givens旋转 */
    void applyGivens(QVector<double>& R, int n,
                     double c, double s, int row1, int row2) const;

    mutable Stats m_stats;     ///< 操作统计
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
