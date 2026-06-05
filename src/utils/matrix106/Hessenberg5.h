#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Hessenberg矩阵分解
 *
 * 将一般矩阵通过正交相似变换约化为上Hessenberg形式，
 * 是QR算法求特征值的重要预处理步骤。
 */
class Hessenberg5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDecomposed = 0; double avgProcessingTimeMs = 0.0; };

    explicit Hessenberg5(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int dim);

    /** @brief 添加矩阵元素 */
    void addEntry(int row, int col, double value);

    /** @brief 执行Hessenberg分解 */
    void decompose();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成信号 */
    void decomposed(int dimension);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
};
