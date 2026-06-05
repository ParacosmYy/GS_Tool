/**
 * @file NelderMead.h
 * @brief Nelder-Mead单纯形优化 — 无导数非线性优化
 *
 * 功能: 使用Nelder-Mead下山单纯形法求解无约束非线性优化问题。
 *       不需要目标函数的梯度信息，适用于不可微或噪声目标函数。
 *       支持反射/扩展/收缩/缩小四种操作。
 *
 * 协作: BfgsOptimizer(梯度优化) / SimulatedAnnealing(全局搜索)
 */
#ifndef NELDERMEAD_H
#define NELDERMEAD_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief Nelder-Mead单纯形优化器
 */
class NelderMead : public QObject {
    Q_OBJECT

public:
    /** @brief 目标函数类型 */
    using ObjFunc = std::function<double(const QVector<double>&)>;

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalMinimizations = 0;   ///< 累计优化次数
        quint64 totalEvaluations = 0;     ///< 累计函数求值次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit NelderMead(QObject* parent = nullptr);

    /** @brief 执行Nelder-Mead优化
     *  @param f 目标函数
     *  @param x0 初始点
     *  @param tol 收敛容限
     *  @param maxIter 最大迭代次数
     *  @return 最优解 */
    QVector<double> minimize(const ObjFunc& f,
                             const QVector<double>& x0,
                             double tol = 1e-8,
                             int maxIter = 1000);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 优化完成 @param minValue 最优值 @param iterations 迭代次数 */
    void minimizationCompleted(double minValue, int iterations);

private:
    /** @brief 单纯形顶点(值+坐标) */
    struct Vertex {
        double value;                  ///< 函数值
        QVector<double> coords;        ///< 坐标
    };

    /** @brief 初始化单纯形 @param x0 初始点 @param f 目标函数 */
    QVector<Vertex> initSimplex(const QVector<double>& x0,
                                const ObjFunc& f);

    /** @brief 计算重心(排除最差点) */
    QVector<double> centroid(const QVector<Vertex>& simplex, int excludeIdx);

    /** @brief 反射操作 */
    Vertex reflect(const QVector<double>& cent,
                   const Vertex& worst, double alpha);

    /** @brief 扩展操作 */
    Vertex expand(const QVector<double>& cent,
                  const Vertex& worst, double gamma, const ObjFunc& f);

    /** @brief 外部收缩 */
    Vertex contractOutside(const QVector<double>& cent,
                           const Vertex& worst, double beta, const ObjFunc& f);

    /** @brief 内部收缩 */
    Vertex contractInside(const QVector<double>& cent,
                          const Vertex& worst, double beta, const ObjFunc& f);

    /** @brief 缩小操作 */
    void shrink(QVector<Vertex>& simplex, double sigma, const ObjFunc& f);

    /** @brief 简单线搜索计算初始步长 */
    static double computeStep(int dim, int idx);

    double m_timeSum;       ///< 处理时间累加器
    quint64 m_evalCount;    ///< 本次求值计数
    Stats   m_stats;        ///< 统计信息
};

#endif // NELDERMEAD_H
