/**
 * @file NumericalGradient.h
 * @brief 数值梯度计算器 — 有限差分法
 *
 * 功能: 前向/中心/后向差分，标量场梯度，
 *       向量场雅可比矩阵，自动步长选择，
 *       统计计算次数/维度/耗时。
 */
#ifndef NUMERICALGRADIENT_H
#define NUMERICALGRADIENT_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief 数值梯度计算器
 */
class NumericalGradient : public QObject {
    Q_OBJECT

public:
    /** @brief 差分方法 */
    enum class Method {
        Forward,    ///< 前向差分 O(h)
        Backward,   ///< 后向差分 O(h)
        Central     ///< 中心差分 O(h^2)
    };
    Q_ENUM(Method)

    /** @brief 统计 */
    struct Stats {
        quint64 totalEvaluations = 0;   ///< 累计函数求值次数
        quint64 totalGradients = 0;     ///< 累计梯度计算次数
        quint64 totalJacobians = 0;     ///< 累计雅可比计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    /** @brief 标量函数类型: 输入n维向量, 输出标量 */
    using ScalarFunc = std::function<double(const QVector<double>&)>;

    /** @brief 向量函数类型: 输入n维向量, 输出m维向量 */
    using VectorFunc = std::function<QVector<double>(const QVector<double>&)>;

    explicit NumericalGradient(QObject* parent = nullptr);

    /** @brief 设置步长 @param h 差分步长(默认1e-6) */
    void setStepSize(double h);

    /** @brief 设置差分方法 @param method 方法 */
    void setMethod(Method method);

    /** @brief 计算标量场梯度 @param func 标量函数 @param x 评估点 @return 梯度向量 */
    QVector<double> gradient(const ScalarFunc& func, const QVector<double>& x);

    /** @brief 计算单变量导数 @param func 标量函数 @param x 评估点 @param dim 维度索引 @return 偏导数 */
    double partialDerivative(const ScalarFunc& func, const QVector<double>& x, int dim);

    /** @brief 计算向量场雅可比矩阵 @param func 向量函数 @param x 评估点 @return 雅可比矩阵[输出维度][输入维度] */
    QVector<QVector<double>> jacobian(const VectorFunc& func, const QVector<double>& x);

    /** @brief 计算Hessian矩阵 @param func 标量函数 @param x 评估点 @return Hessian矩阵 */
    QVector<QVector<double>> hessian(const ScalarFunc& func, const QVector<double>& x);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 梯度计算完成 @param dim 维度数 */
    void gradientComputed(int dim);
    /** @brief 雅可比计算完成 @param rows 行数 @param cols 列数 */
    void jacobianComputed(int rows, int cols);

private:
    double m_stepSize;              ///< 差分步长
    Method m_method;                ///< 差分方法
    Stats m_stats;
    double m_timeSum;
};

#endif // NUMERICALGRADIENT_H
