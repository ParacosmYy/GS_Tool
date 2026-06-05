/**
 * @file NewtonMethod.h
 * @brief 牛顿法优化器 — 带Hessian修正的非凸函数优化
 *
 * 功能: 实现带阻尼和Hessian修正的牛顿法，用于非线性函数的
 *       局部最优求解。支持线搜索、Hessian正定性修正、
 *       收敛性检测。适用于传感器标定曲线拟合、参数辨识。
 *
 * 协作: TrendPredictor(趋势拟合) / DataInterpolator(插值优化)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <functional>

/**
 * @brief 牛顿法优化器 — Hessian修正的非凸优化
 */
class NewtonMethod : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalOptimizations = 0;             ///< 累计优化次数
        int totalIterations = 0;                ///< 累计迭代次数
        double avgProcessingTimeMs = 0.0;       ///< 平均处理时间(ms)
    };

    /** @brief 优化结果 */
    struct Result {
        QVector<double> x;          ///< 最优解
        double functionValue = 0.0; ///< 目标函数值
        int iterations = 0;         ///< 实际迭代次数
        bool converged = false;     ///< 是否收敛
        QString message;            ///< 状态消息
    };

    /** @brief 目标函数类型: 输入向量 -> 函数值 */
    using ObjectiveFunc = std::function<double(const QVector<double>&)>;

    /** @brief 梯度函数类型: 输入向量 -> 梯度向量 */
    using GradientFunc = std::function<QVector<double>(const QVector<double>&)>;

    /** @brief Hessian函数类型: 输入向量 -> Hessian矩阵 */
    using HessianFunc = std::function<QVector<QVector<double>>(const QVector<double>&)>;

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit NewtonMethod(QObject* parent = nullptr);

    /**
     * @brief 设置目标函数
     * @param func 目标函数
     */
    void setObjective(ObjectiveFunc func);

    /**
     * @brief 设置梯度函数(可选, 不设则数值差分)
     * @param func 梯度函数
     */
    void setGradient(GradientFunc func);

    /**
     * @brief 设置Hessian函数(可选, 不设则数值差分)
     * @param func Hessian函数
     */
    void setHessian(HessianFunc func);

    /**
     * @brief 执行牛顿法优化
     * @param x0 初始点
     * @param maxIter 最大迭代次数
     * @param tolerance 收敛容差
     * @return 优化结果
     */
    Result optimize(const QVector<double>& x0,
                    int maxIter = 100,
                    double tolerance = 1e-8);

    /**
     * @brief 数值梯度(中心差分)
     * @param x 当前点
     * @param eps 差分步长
     * @return 梯度向量
     */
    QVector<double> numericalGradient(const QVector<double>& x,
                                      double eps = 1e-6) const;

    /**
     * @brief 数值Hessian(中心差分)
     * @param x 当前点
     * @param eps 差分步长
     * @return Hessian矩阵
     */
    QVector<QVector<double>> numericalHessian(
        const QVector<double>& x, double eps = 1e-5) const;

    /**
     * @brief 获取统计信息
     * @return 统计引用
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief 迭代进度信号
     * @param iteration 当前迭代次数
     * @param fValue 当前函数值
     */
    void iterationProgress(int iteration, double fValue);

private:
    /**
     * @brief 修正Hessian为正定(特征值截断)
     * @param H Hessian矩阵
     * @return 修正后的矩阵
     */
    QVector<QVector<double>> regularizeHessian(
        const QVector<QVector<double>>& H) const;

    /**
     * @brief 求解线性系统 H * d = -g (Cholesky)
     * @param H Hessian矩阵
     * @param g 负梯度
     * @return 搜索方向
     */
    QVector<double> solveLinearSystem(
        const QVector<QVector<double>>& H,
        const QVector<double>& g) const;

    /**
     * @brief 回溯线搜索
     * @param x 当前点
     * @param d 搜索方向
     * @param alpha0 初始步长
     * @return 步长
     */
    double lineSearch(const QVector<double>& x,
                      const QVector<double>& d,
                      double alpha0 = 1.0) const;

    ObjectiveFunc m_objective;          ///< 目标函数
    GradientFunc m_gradient;            ///< 梯度函数(可选)
    HessianFunc m_hessian;              ///< Hessian函数(可选)
    bool m_hasAnalyticGradient;         ///< 是否有解析梯度
    bool m_hasAnalyticHessian;          ///< 是否有解析Hessian

    Stats m_stats;
    double m_timeSum = 0.0;             ///< 处理时间累加器
};
