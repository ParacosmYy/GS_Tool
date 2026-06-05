#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Hessenberg矩阵分解
 *
 * 将一般矩阵通过正交相似变换化为上Hessenberg形式,
 * 作为QR特征值算法的预处理步骤,显著加速特征值收敛。
 */
class Hessenberg4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDecomposed = 0; double avgProcessingTimeMs = 0.0; };

    explicit Hessenberg4(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int dim);

    /** @brief 添加矩阵元素 */
    void addEntry(int row, int col, double value);

    /** @brief 执行Hessenberg分解 */
    void decompose();

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 分解完成信号,返回矩阵维度 */
    void decomposed(int dimension);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
};
