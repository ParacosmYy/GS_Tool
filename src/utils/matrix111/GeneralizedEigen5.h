#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 广义特征值问题求解器实现 (版本5)
 *
 * 求解广义特征值问题 Ax = λBx，其中A为对称矩阵，B为对称正定矩阵，
 * 通过Cholesky分解将广义问题化为标准特征值问题求解。
 */
class GeneralizedEigen5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDecomposed = 0; double avgProcessingTimeMs = 0.0; };

    explicit GeneralizedEigen5(QObject* parent = nullptr);

    /** @brief 设置矩阵A(对称)和B(对称正定) */
    void setMatrices(const QVector<QVector<double>>& A, const QVector<QVector<double>>& B);

    /** @brief 执行广义特征值分解，返回特征值向量(升序) */
    QVector<double> decompose();

    /** @brief 获取特征向量矩阵，每列对应一个特征值 */
    QVector<QVector<double>> eigenvectors() const { return m_eigenvectors; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成信号，返回矩阵维度 */
    void decompositionCompleted(int dimension);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QVector<double>> m_A;
    QVector<QVector<double>> m_B;
    QVector<QVector<double>> m_eigenvectors;
};
