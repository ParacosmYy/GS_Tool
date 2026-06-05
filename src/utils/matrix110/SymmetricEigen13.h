#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 对称矩阵特征值分解实现 (13参数配置)
 *
 * 提供对称矩阵的特征值/特征向量计算，基于Jacobi迭代和三对角化方法。
 */
class SymmetricEigen13 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalDecompositions = 0;    ///< 总分解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int totalJacobiRotations = 0;   ///< 总Jacobi旋转次数
    };

    explicit SymmetricEigen13(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 对称矩阵特征值分解
     * @param matrix 对称矩阵（仅使用下三角部分）
     * @param computeVectors 是否计算特征向量
     * @return 特征值向量（升序排列）
     */
    QVector<double> decompose(const QVector<QVector<double>>& matrix, bool computeVectors = true);

    /**
     * @brief 获取特征向量矩阵
     * @return 各列对应特征值的特征向量
     */
    QVector<QVector<double>> eigenvectors() const { return m_eigenvectors; }

    /**
     * @brief 获取前k个主成分
     * @param k 主成分数量
     * @return 降维投影矩阵
     */
    QVector<QVector<double>> principalComponents(int k) const;

    /**
     * @brief 计算条件数（最大/最小特征值比）
     * @return 条件数
     */
    double conditionNumber() const;

    /**
     * @brief 设置收敛精度
     * @param tolerance 收敛容差
     */
    void setTolerance(double tolerance) { m_tolerance = tolerance; }

signals:
    /// 分解完成信号
    void decompositionCompleted(int matrixSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_tolerance = 1e-10;
    QVector<double> m_eigenvalues;
    QVector<QVector<double>> m_eigenvectors;
};
