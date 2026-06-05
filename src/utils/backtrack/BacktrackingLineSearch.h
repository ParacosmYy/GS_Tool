/**
 * @file BacktrackingLineSearch.h
 * @brief 回溯线搜索 — Armijo 条件
 *
 * 功能: 给定搜索方向后，沿该方向逐步回缩步长 α，
 *       直至满足 Armijo 充分下降条件，保证梯度类优化器稳定收敛。
 *
 * 协作: GoldenSectionSearch(精确线搜索) / GradientDescent(调用者)
 */
#ifndef BACKTRACKINGLINESEARCH_H
#define BACKTRACKINGLINESEARCH_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief 回溯线搜索器(Armijo 条件)
 */
class BacktrackingLineSearch : public QObject {
    Q_OBJECT

public:
    /** @brief 多元目标函数类型 */
    using ObjFunc = std::function<double(QVector<double>)>;
    /** @brief 梯度函数类型 */
    using GradFunc = std::function<QVector<double>(QVector<double>)>;

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSearches = 0;        ///< 累计搜索次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit BacktrackingLineSearch(QObject* parent = nullptr);

    /**
     * @brief 沿给定方向执行回溯线搜索
     * @param f         目标函数
     * @param grad      梯度函数
     * @param x         当前点
     * @param direction 搜索方向(通常为负梯度)
     * @param alpha0    初始步长
     * @param rho       步长衰减因子(0,1)
     * @param c         Armijo 常数(0,1)
     * @return 满足 Armijo 条件的步长 α
     */
    double search(ObjFunc f, GradFunc grad,
                  QVector<double> x, QVector<double> direction,
                  double alpha0 = 1.0, double rho = 0.5,
                  double c = 1e-4);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 搜索完成 @param alpha 最终步长 @param evaluations 函数求值次数 */
    void searchCompleted(double alpha, int evaluations);

private:
    Stats  m_stats;          ///< 统计信息
    double m_timeSum = 0.0;  ///< 累计耗时
};

#endif // BACKTRACKINGLINESEARCH_H
