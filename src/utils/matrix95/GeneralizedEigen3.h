#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 广义特征值求解工具类
 *
 * 提供广义特征值问题 Ax = λBx 的求解功能，
 * 支持设置A、B两个矩阵的非零元素。
 */
class GeneralizedEigen3 : public QObject {
    Q_OBJECT
public:
    /// 求解统计信息
    struct Stats {
        int totalSolves = 0;        ///< 总求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit GeneralizedEigen3(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int dim);

    /** @brief 添加矩阵A的非零元素 */
    void addA(int row, int col, double value);

    /** @brief 添加矩阵B的非零元素 */
    void addB(int row, int col, double value);

    /** @brief 求解广义特征值问题 */
    void solve();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号，返回维度 */
    void solveCompleted(int dimension);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
    QVector<QPair<QPair<int, int>, double>> m_entriesA;
    QVector<QPair<QPair<int, int>, double>> m_entriesB;
};
