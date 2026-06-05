#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Krylov子空间迭代求解器实现
 *
 * 基于Krylov子空间投影方法求解大规模稀疏线性方程组，
 * 支持GMRES/BiCGSTAB等变体，适用于计算流体力学和有限元分析中
 * 大型稀疏系统的迭代求解。
 */
class KrylovSolver5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit KrylovSolver5(QObject* parent = nullptr);

    /** @brief 设置求解方法："gmres"或"bicgstab" */
    void setMethod(const QString& method);

    /** @brief 设置最大迭代次数和残差收敛阈值 */
    void setTolerance(int maxIterations, double residualThreshold);

    /** @brief 设置预条件器类型："none"、"jacobi"或"ilu0" */
    void setPreconditioner(const QString& type);

    /** @brief 求解稀疏线性方程组Ax=b，返回解向量和迭代次数 */
    QPair<QVector<double>, int> solve(int dimension,
                                       const QVector<QPair<QPair<int,int>,double>>& entries,
                                       const QVector<double>& b);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号，返回迭代次数和最终残差 */
    void solveCompleted(int iterations, double finalResidual);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_method = QStringLiteral("gmres");
    int m_maxIter = 1000;
    double m_tolerance = 1e-8;
    QString m_preconditioner = QStringLiteral("none");
};
