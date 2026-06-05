/**
 * @file HiddenMarkovModel.h
 * @brief 隐马尔可夫模型 — 前向/后向/Viterbi/Baum-Welch
 *
 * 功能: 离散观测HMM，支持前向概率/后向概率/Viterbi解码/
 *       Baum-Welch参数估计，统计训练/解码次数/耗时。
 */
#ifndef HIDDENMARKOVMODEL_H
#define HIDDENMARKOVMODEL_H

#include <QObject>
#include <QVector>

class HiddenMarkovModel : public QObject {
    Q_OBJECT
public:
    /** 模型统计 */
    struct Stats {
        quint64 totalDecodings = 0;
        quint64 totalTrainings = 0;
        quint64 totalObservationsProcessed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    /** @brief 构造 @param states 隐状态数 @param symbols 观测符号数 @param parent 父对象 */
    explicit HiddenMarkovModel(int states = 2, int symbols = 2,
                               QObject* parent = nullptr);

    /** @brief 设置转移概率矩阵 @param A 矩阵[states][states] */
    void setTransitionMatrix(const QVector<QVector<double>>& A);

    /** @brief 设置发射概率矩阵 @param B 矩阵[states][symbols] */
    void setEmissionMatrix(const QVector<QVector<double>>& B);

    /** @brief 设置初始概率 @param pi 概率向量[states] */
    void setInitialProbabilities(const QVector<double>& pi);

    /** @brief 前向算法 @param observations 观测序列 @return P(O|λ) */
    double forward(const QVector<int>& observations) const;

    /** @brief 后向算法 @param observations 观测序列 @return P(O|λ) */
    double backward(const QVector<int>& observations) const;

    /** @brief Viterbi解码 @param observations 观测序列 @return 最优状态序列 */
    QVector<int> viterbi(const QVector<int>& observations) const;

    /** @brief Baum-Welch训练 @param observations 观测序列 @param maxIterations 最大迭代 @param tolerance 收敛阈值 @return 最终log似然 */
    double train(const QVector<int>& observations,
                 int maxIterations = 100, double tolerance = 1e-6);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void trainingCompleted(int iterations, double logLikelihood);
    void decodingCompleted(int sequenceLength);

private:
    int m_states;
    int m_symbols;
    QVector<QVector<double>> m_A;   ///< 转移概率
    QVector<QVector<double>> m_B;   ///< 发射概率
    QVector<double> m_pi;           ///< 初始概率

    Stats m_stats;
    double m_timeSum;
};

#endif // HIDDENMARKOVMODEL_H
