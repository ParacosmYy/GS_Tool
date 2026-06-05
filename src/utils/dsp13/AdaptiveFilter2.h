/**
 * @file AdaptiveFilter2.h
 * @brief 自适应滤波器 — LMS/NLMS/RLS算法 + 收敛性追踪
 *
 * 功能: 实现LMS(最小均方)、NLMS(归一化LMS)自适应滤波，
 *       实时跟踪收敛过程(学习曲线、权重轨迹、失配量)。
 *       支持系统辨识、噪声消除、逆建模等应用场景。
 *
 * 协作: DigitalFilter(固定滤波器) / HilbertTransform(解析信号)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 自适应滤波器引擎 — LMS/NLMS算法 + 收敛性追踪
 */
class AdaptiveFilter2 : public QObject {
    Q_OBJECT

public:
    /** @brief 自适应算法类型 */
    enum class Algorithm {
        LMS,            ///< 最小均方算法
        NLMS,           ///< 归一化LMS算法
        SignLMS         ///< 符号LMS(低复杂度)
    };
    Q_ENUM(Algorithm)

    /** @brief 收敛信息 */
    struct ConvergenceInfo {
        double steadyStateError = 0.0;      ///< 稳态误差
        double convergenceRate = 0.0;       ///< 收敛速率
        int convergenceIter = 0;            ///< 收敛所需迭代数
        double misadjustment = 0.0;         ///< 失调量
        bool isConverged = false;           ///< 是否已收敛
    };

    /** @brief 滤波步骤结果 */
    struct StepResult {
        double output = 0.0;                ///< 滤波输出
        double error = 0.0;                 ///< 误差信号
        QVector<double> weights;            ///< 当前权重
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        int totalSteps = 0;                         ///< 累计滤波步数
        int totalConvergences = 0;                   ///< 累计收敛次数
        double peakError = 0.0;                      ///< 峰值误差
        double avgProcessingTimeMs = 0.0;            ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit AdaptiveFilter2(QObject* parent = nullptr);

    /** @brief 设置算法类型 @param algo 算法 */
    void setAlgorithm(Algorithm algo);

    /** @brief 设置滤波器阶数 @param order 阶数 */
    void setFilterOrder(int order);

    /** @brief 设置步长参数 @param mu 步长 */
    void setStepSize(double mu);

    /** @brief 设置NLMS正则化因子 @param eps 正则化因子 */
    void setRegularization(double eps);

    /** @brief 设置收敛判定阈值 @param threshold 误差阈值 */
    void setConvergenceThreshold(double threshold);

    /** @brief 单步自适应滤波 @param input 输入信号 @param desired 期望信号 @return 步骤结果 */
    StepResult processStep(double input, double desired);

    /** @brief 批量自适应滤波 @param inputs 输入序列 @param desireds 期望序列 @return 步骤结果列表 */
    QList<StepResult> processBatch(
        const QVector<double>& inputs,
        const QVector<double>& desireds);

    /** @brief 仅滤波(不更新权重) @param input 输入信号 @return 滤波输出 */
    double filterOnly(double input) const;

    /** @brief 获取收敛信息 @return 收敛信息 */
    ConvergenceInfo convergenceInfo() const;

    /** @brief 获取当前权重 @return 权重向量 */
    QVector<double> weights() const;

    /** @brief 重置滤波器(权重归零) */
    void reset();

    /** @brief 获取当前统计 @return 统计常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 收敛完成 @param iterations 收敛迭代数 @param error 稳态误差 */
    void converged(int iterations, double error);

    /** @brief 单步完成 @param error 误差 */
    void stepProcessed(double error);

private:
    /** @brief 更新权重LMS @param error 误差 */
    void updateLMS(double error);

    /** @brief 更新权重NLMS @param error 误差 */
    void updateNLMS(double error);

    /** @brief 更新权重SignLMS @param error 误差 */
    void updateSignLMS(double error);

    /** @brief 检查收敛 */
    void checkConvergence();

    Algorithm m_algorithm;             ///< 算法类型
    int m_order;                       ///< 滤波器阶数
    double m_mu;                       ///< 步长
    double m_epsilon;                  ///< 正则化因子
    double m_convThreshold;            ///< 收敛阈值

    QVector<double> m_weights;         ///< 滤波器权重
    QVector<double> m_inputBuffer;     ///< 输入延迟线
    int m_bufferIndex;                 ///< 环形缓冲区索引

    ConvergenceInfo m_convergence;     ///< 收敛信息
    double m_errorSum;                 ///< 误差累计(用于判定)
    int m_errorCount;                  ///< 误差计数
    double m_minSteadyError;           ///< 记录最小稳态误差

    Stats m_stats;                     ///< 运行时统计
    double m_timeSum = 0.0;            ///< 累计耗时(ms)
};
