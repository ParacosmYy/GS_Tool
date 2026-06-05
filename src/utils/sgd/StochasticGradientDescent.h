/**
 * @file StochasticGradientDescent.h
 * @brief 随机梯度下降优化器 — SGD/Momentum/Nesterov/Adam
 *
 * 支持四种优化器变体用于机器学习参数优化:
 *   - Vanilla SGD: 标准随机梯度下降
 *   - Momentum SGD: 动量加速收敛
 *   - Nesterov SGD: Nesterov加速梯度
 *   - Adam: 自适应矩估计优化器
 *
 * 协作: DataClassifier(模型训练) / TrendPredictor(拟合优化)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

/**
 * @class StochasticGradientDescent
 * @brief 随机梯度下降优化器 — 多种变体支持
 *
 * 提供SGD系列优化算法，适用于线性回归、逻辑回归、
 * 神经网络等模型的参数训练。
 */
class StochasticGradientDescent : public QObject
{
    Q_OBJECT

public:
    /** @brief 优化器类型 */
    enum class OptimizerType {
        VanillaSGD,     ///< 标准SGD
        Momentum,       ///< 动量SGD
        Nesterov,       ///< Nesterov加速梯度
        Adam            ///< 自适应矩估计
    };
    Q_ENUM(OptimizerType)

    /** @brief 优化器配置参数 */
    struct Config {
        double learningRate = 0.01;       ///< 学习率
        double momentum = 0.9;            ///< 动量系数(Momentum/Nesterov)
        double beta1 = 0.9;               ///< Adam一阶矩衰减率
        double beta2 = 0.999;             ///< Adam二阶矩衰减率
        double epsilon = 1e-8;            ///< 数值稳定项
        double weightDecay = 0.0;         ///< L2正则化系数
        int maxIterations = 1000;         ///< 最大迭代次数
        double tolerance = 1e-6;          ///< 收敛阈值
    };

    /** @brief 优化结果 */
    struct OptimizeResult {
        QVector<double> parameters;       ///< 最优参数
        double finalLoss = 0.0;           ///< 最终损失值
        int iterations = 0;               ///< 实际迭代次数
        bool converged = false;           ///< 是否收敛
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalUpdates = 0;         ///< 累计参数更新次数
        quint64 totalOptimizations = 0;   ///< 累计完整优化次数
        quint64 totalConverged = 0;       ///< 累计收敛次数
        double  avgProcessingTimeMs = 0.0;///< 平均单次优化耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit StochasticGradientDescent(QObject* parent = nullptr);

    /**
     * @brief 设置优化器类型
     * @param type 优化器类型
     */
    void setOptimizerType(OptimizerType type);

    /**
     * @brief 设置配置参数
     * @param config 配置结构体
     */
    void setConfig(const Config& config);

    /**
     * @brief 执行一步梯度更新
     * @param parameters 当前参数
     * @param gradients 当前梯度
     * @return 更新后的参数
     */
    QVector<double> step(const QVector<double>& parameters,
                         const QVector<double>& gradients);

    /**
     * @brief 完整优化过程(使用损失函数梯度回调)
     * @param initialParams 初始参数
     * @param lossGradient 接收参数返回梯度的回调
     * @return 优化结果
     */
    OptimizeResult optimize(
        const QVector<double>& initialParams,
        std::function<QVector<double>(const QVector<double>&)> lossGradient);

    /**
     * @brief 重置优化器内部状态(动量/矩等)
     */
    void resetState();

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 参数更新 @param iteration 迭代次数 @param loss 当前损失 */
    void parameterUpdated(int iteration, double loss);

    /** @brief 优化完成 @param result 优化结果 */
    void optimizationFinished(const OptimizeResult& result);

private:
    /** @brief Vanilla SGD更新 */
    QVector<double> updateVanilla(const QVector<double>& params,
                                  const QVector<double>& grads);

    /** @brief Momentum SGD更新 */
    QVector<double> updateMomentum(const QVector<double>& params,
                                   const QVector<double>& grads);

    /** @brief Nesterov加速梯度更新 */
    QVector<double> updateNesterov(const QVector<double>& params,
                                   const QVector<double>& grads);

    /** @brief Adam优化器更新 */
    QVector<double> updateAdam(const QVector<double>& params,
                               const QVector<double>& grads);

    OptimizerType m_type = OptimizerType::Adam; ///< 优化器类型
    Config m_config;                             ///< 配置参数

    QVector<double> m_velocity;    ///< 动量速度向量
    QVector<double> m_momentFirst; ///< Adam一阶矩
    QVector<double> m_momentSecond;///< Adam二阶矩
    int m_timeStep = 0;            ///< Adam时间步

    mutable Stats m_stats;         ///< 操作统计
    mutable double m_timeSum = 0.0;///< 累计耗时
};
