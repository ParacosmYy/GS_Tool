/**
 * @file HiddenMarkovModel2.h
 * @brief 隐马尔可夫模型(HMM) — Forward-Backward + Viterbi算法
 *
 * 功能: 实现HMM的三大基本问题:
 *       1. 评估问题(Forward算法计算观测序列概率)
 *       2. 解码问题(Viterbi算法找最优状态序列)
 *       3. 学习问题(Baum-Welch/EM算法参数估计)
 *
 * 协作: StatDistribution(分布) / AnomalyDetector(异常检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

#include <vector>

/**
 * @brief 隐马尔可夫模型 — Forward-Backward/Viterbi/Baum-Welch
 */
class HiddenMarkovModel2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDecodings        = 0;   ///< 累计解码次数
        quint64 totalForwardPasses     = 0;   ///< 累计前向传递次数
        quint64 totalBaumWelchIters    = 0;   ///< 累计Baum-Welch迭代数
        double  avgProcessingTimeMs    = 0.0; ///< 平均处理时间(ms)
        double  lastLogLikelihood      = 0.0; ///< 最近一次对数似然
    };

    /**
     * @brief 构造函数
     * @param numStates 隐状态数
     * @param numObservations 观测符号数
     * @param parent 父对象
     */
    explicit HiddenMarkovModel2(int numStates = 2, int numObservations = 2,
                                QObject* parent = nullptr);

    /**
     * @brief 设置转移概率矩阵
     * @param A 转移矩阵 [N x N], 行优先
     */
    void setTransitions(const QVector<QVector<double>>& A);

    /**
     * @brief 设置发射概率矩阵
     * @param B 发射矩阵 [N x M]
     */
    void setEmissions(const QVector<QVector<double>>& B);

    /**
     * @brief 设置初始概率
     * @param pi 初始概率 [N]
     */
    void setInitial(const QVector<double>& pi);

    /**
     * @brief Forward算法: 计算P(O|lambda)
     * @param observations 观测序列(符号索引)
     * @return 对数似然 log P(O|lambda)
     */
    double forward(const QVector<int>& observations);

    /**
     * @brief Backward算法: 计算后向概率
     * @param observations 观测序列
     * @return 后向概率矩阵 [T x N]
     */
    QVector<QVector<double>> backward(const QVector<int>& observations);

    /**
     * @brief Viterbi算法: 最优状态序列
     * @param observations 观测序列
     * @return (最优状态序列, 最大对数概率)
     */
    QPair<QVector<int>, double> viterbi(const QVector<int>& observations);

    /**
     * @brief Baum-Welch算法: EM参数估计
     * @param observations 观测序列
     * @param maxIterations 最大迭代次数
     * @param tolerance 收敛阈值
     * @return (最终对数似然, 迭代次数)
     */
    QPair<double, int> baumWelch(const QVector<int>& observations,
                                  int maxIterations = 100,
                                  double tolerance = 1e-6);

    /** @brief 状态数 @return 状态数 */
    int numStates() const { return m_numStates; }

    /** @brief 观测符号数 @return 符号数 */
    int numObservations() const { return m_numObs; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief Forward计算完成 @param logLikelihood 对数似然 */
    void forwardCompleted(double logLikelihood);

    /** @brief Viterbi解码完成 @param states 状态序列 */
    void viterbiCompleted(const QVector<int>& states);

    /** @brief Baum-Welch迭代完成 @param iter 迭代数 @param ll 对数似然 */
    void baumWelchIterated(int iter, double ll);

private:
    int m_numStates;  ///< 隐状态数N
    int m_numObs;     ///< 观测符号数M

    QVector<QVector<double>> m_A;   ///< 转移概率 [N][N]
    QVector<QVector<double>> m_B;   ///< 发射概率 [N][M]
    QVector<double>          m_pi;  ///< 初始概率 [N]

    mutable Stats  m_stats;
    mutable double m_timeSumMs = 0.0;
};
