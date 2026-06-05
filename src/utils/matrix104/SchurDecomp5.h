#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Schur分解求解器
 *
 * 将方阵分解为 Q * T * Q^H 形式，其中T为上三角矩阵，
 * 用于计算特征值和矩阵函数。
 */
class SchurDecomp5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalDecompositions = 0; ///< 已完成分解次数
        int matrixDimension = 0;     ///< 矩阵维度
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SchurDecomp5(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int n);
    /** @brief 添加矩阵元素 */
    void addEntry(int row, int col, double value);
    /** @brief 执行Schur分解 */
    void decompose();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成，返回矩阵维度 */
    void decomposed(int dimension);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
    QVector<QPair<int, int>> m_entries;
};
