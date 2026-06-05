/**
 * @file HiddenMarkovModel2.cpp
 * @brief 隐马尔可夫模型实现 — Forward-Backward/Viterbi/Baum-Welch
 */

#include "utils/hmm2/HiddenMarkovModel2.h"

#include <QElapsedTimer>

#include <cmath>
#include <algorithm>
#include <limits>

/** @brief 构造函数 @param numStates 状态数 @param numObservations 观测数 @param parent 父对象 */
HiddenMarkovModel2::HiddenMarkovModel2(int numStates, int numObservations,
                                       QObject* parent)
    : QObject(parent)
    , m_numStates(std::max(2, numStates))
    , m_numObs(std::max(2, numObservations))
{
    /* 默认均匀分布初始化 */
    double initP = 1.0 / m_numStates;
    m_pi.fill(initP, m_numStates);

    m_A.resize(m_numStates);
    double transP = 1.0 / m_numStates;
    for (auto& row : m_A) {
        row.fill(transP, m_numStates);
    }

    m_B.resize(m_numStates);
    double emitP = 1.0 / m_numObs;
    for (auto& row : m_B) {
        row.fill(emitP, m_numObs);
    }
}

/** @brief 设置转移概率 @param A 转移矩阵 */
void HiddenMarkovModel2::setTransitions(const QVector<QVector<double>>& A)
{
    if (A.size() != m_numStates) return;
    for (int i = 0; i < m_numStates; ++i) {
        if (A[i].size() != m_numStates) return;
    }
    m_A = A;
}

/** @brief 设置发射概率 @param B 发射矩阵 */
void HiddenMarkovModel2::setEmissions(const QVector<QVector<double>>& B)
{
    if (B.size() != m_numStates) return;
    for (int i = 0; i < m_numStates; ++i) {
        if (B[i].size() != m_numObs) return;
    }
    m_B = B;
}

/** @brief 设置初始概率 @param pi 初始概率 */
void HiddenMarkovModel2::setInitial(const QVector<double>& pi)
{
    if (pi.size() != m_numStates) return;
    m_pi = pi;
}

/** @brief Forward算法 @param observations 观测序列 @return 对数似然 */
double HiddenMarkovModel2::forward(const QVector<int>& observations)
{
    QElapsedTimer timer;
    timer.start();

    int T = observations.size();
    if (T == 0) return -std::numeric_limits<double>::infinity();

    /* alpha[t][i] = P(o1..ot, qt=Si | lambda) */
    QVector<QVector<double>> alpha(T, QVector<double>(m_numStates, 0.0));

    /* 初始化: alpha[0][i] = pi[i] * B[i][o0] */
    int o0 = observations[0];
    if (o0 < 0 || o0 >= m_numObs) return -std::numeric_limits<double>::infinity();

    for (int i = 0; i < m_numStates; ++i) {
        alpha[0][i] = m_pi[i] * m_B[i][o0];
    }

    /* 递推: alpha[t][j] = sum_i(alpha[t-1][i] * A[i][j]) * B[j][ot] */
    for (int t = 1; t < T; ++t) {
        int ot = observations[t];
        if (ot < 0 || ot >= m_numObs) return -std::numeric_limits<double>::infinity();

        for (int j = 0; j < m_numStates; ++j) {
            double sum = 0.0;
            for (int i = 0; i < m_numStates; ++i) {
                sum += alpha[t - 1][i] * m_A[i][j];
            }
            alpha[t][j] = sum * m_B[j][ot];
        }
    }

    /* 终止: P(O|lambda) = sum_i alpha[T-1][i] */
    double logLikelihood = 0.0;
    for (int i = 0; i < m_numStates; ++i) {
        logLikelihood += alpha[T - 1][i];
    }
    logLikelihood = std::log(logLikelihood);

    ++m_stats.totalForwardPasses;
    m_stats.lastLogLikelihood = logLikelihood;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalForwardPasses + m_stats.totalDecodings);

    emit forwardCompleted(logLikelihood);
    return logLikelihood;
}

/** @brief Backward算法 @param observations 观测序列 @return 后向概率矩阵 */
QVector<QVector<double>> HiddenMarkovModel2::backward(const QVector<int>& observations)
{
    int T = observations.size();
    QVector<QVector<double>> beta(T, QVector<double>(m_numStates, 0.0));
    if (T == 0) return beta;

    /* 初始化: beta[T-1][i] = 1 */
    for (int i = 0; i < m_numStates; ++i) {
        beta[T - 1][i] = 1.0;
    }

    /* 递推: beta[t][i] = sum_j(A[i][j] * B[j][o_{t+1}] * beta[t+1][j]) */
    for (int t = T - 2; t >= 0; --t) {
        int ot1 = observations[t + 1];
        if (ot1 < 0 || ot1 >= m_numObs) return beta;

        for (int i = 0; i < m_numStates; ++i) {
            double sum = 0.0;
            for (int j = 0; j < m_numStates; ++j) {
                sum += m_A[i][j] * m_B[j][ot1] * beta[t + 1][j];
            }
            beta[t][i] = sum;
        }
    }

    return beta;
}

/** @brief Viterbi算法 @param observations 观测序列 @return (状态序列, 对数概率) */
QPair<QVector<int>, double> HiddenMarkovModel2::viterbi(const QVector<int>& observations)
{
    QElapsedTimer timer;
    timer.start();

    int T = observations.size();
    QVector<int> path;
    double maxLogProb = -std::numeric_limits<double>::infinity();

    if (T == 0) return {path, maxLogProb};

    /* delta[t][i] = max over paths ending at state i at time t */
    QVector<QVector<double>> delta(T, QVector<double>(m_numStates, 0.0));
    QVector<QVector<int>> psi(T, QVector<int>(m_numStates, -1));

    /* 初始化(对数空间) */
    int o0 = observations[0];
    if (o0 < 0 || o0 >= m_numObs) return {path, maxLogProb};

    for (int i = 0; i < m_numStates; ++i) {
        delta[0][i] = std::log(m_pi[i]) + std::log(m_B[i][o0]);
    }

    /* 递推 */
    for (int t = 1; t < T; ++t) {
        int ot = observations[t];
        if (ot < 0 || ot >= m_numObs) return {path, maxLogProb};

        for (int j = 0; j < m_numStates; ++j) {
            double bestVal = -std::numeric_limits<double>::infinity();
            int bestIdx = 0;
            for (int i = 0; i < m_numStates; ++i) {
                double val = delta[t - 1][i] + std::log(m_A[i][j]);
                if (val > bestVal) {
                    bestVal = val;
                    bestIdx = i;
                }
            }
            delta[t][j] = bestVal + std::log(m_B[j][ot]);
            psi[t][j] = bestIdx;
        }
    }

    /* 终止: 找最大终态 */
    maxLogProb = -std::numeric_limits<double>::infinity();
    int lastState = 0;
    for (int i = 0; i < m_numStates; ++i) {
        if (delta[T - 1][i] > maxLogProb) {
            maxLogProb = delta[T - 1][i];
            lastState = i;
        }
    }

    /* 回溯路径 */
    path.resize(T);
    path[T - 1] = lastState;
    for (int t = T - 2; t >= 0; --t) {
        path[t] = psi[t + 1][path[t + 1]];
    }

    ++m_stats.totalDecodings;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalForwardPasses + m_stats.totalDecodings);

    emit viterbiCompleted(path);
    return {path, maxLogProb};
}

/** @brief Baum-Welch算法 @param observations 观测序列 @param maxIter 最大迭代 @param tol 收敛阈值 @return (对数似然, 迭代数) */
QPair<double, int> HiddenMarkovModel2::baumWelch(const QVector<int>& observations,
                                                  int maxIterations, double tolerance)
{
    QElapsedTimer timer;
    timer.start();

    int T = observations.size();
    if (T == 0) return {-std::numeric_limits<double>::infinity(), 0};

    double prevLL = -std::numeric_limits<double>::infinity();
    int iter = 0;

    for (iter = 0; iter < maxIterations; ++iter) {
        /* E-step: 计算alpha, beta, gamma, xi */
        QVector<QVector<double>> alpha(T, QVector<double>(m_numStates, 0.0));
        QVector<QVector<double>> beta(T, QVector<double>(m_numStates, 0.0));

        /* Forward with scaling */
        QVector<double> scale(T);
        int o0 = observations[0];
        for (int i = 0; i < m_numStates; ++i) {
            alpha[0][i] = m_pi[i] * m_B[i][o0];
        }
        scale[0] = 0.0;
        for (int i = 0; i < m_numStates; ++i) scale[0] += alpha[0][i];
        if (scale[0] > 0) {
            for (int i = 0; i < m_numStates; ++i) alpha[0][i] /= scale[0];
        }

        for (int t = 1; t < T; ++t) {
            int ot = observations[t];
            for (int j = 0; j < m_numStates; ++j) {
                double sum = 0.0;
                for (int i = 0; i < m_numStates; ++i) {
                    sum += alpha[t - 1][i] * m_A[i][j];
                }
                alpha[t][j] = sum * m_B[j][ot];
            }
            scale[t] = 0.0;
            for (int j = 0; j < m_numStates; ++j) scale[t] += alpha[t][j];
            if (scale[t] > 0) {
                for (int j = 0; j < m_numStates; ++j) alpha[t][j] /= scale[t];
            }
        }

        /* Backward with same scaling */
        for (int i = 0; i < m_numStates; ++i) beta[T - 1][i] = 1.0;
        for (int t = T - 2; t >= 0; --t) {
            int ot1 = observations[t + 1];
            for (int i = 0; i < m_numStates; ++i) {
                double sum = 0.0;
                for (int j = 0; j < m_numStates; ++j) {
                    sum += m_A[i][j] * m_B[j][ot1] * beta[t + 1][j];
                }
                beta[t][i] = sum;
            }
            if (scale[t + 1] > 0) {
                for (int i = 0; i < m_numStates; ++i) beta[t][i] /= scale[t + 1];
            }
        }

        /* 计算对数似然 */
        double logLL = 0.0;
        for (int t = 0; t < T; ++t) {
            if (scale[t] > 0) logLL += std::log(scale[t]);
        }

        /* 检查收敛 */
        if (iter > 0 && std::abs(logLL - prevLL) < tolerance) break;
        prevLL = logLL;

        /* M-step: 更新参数 */
        /* gamma[t][i] = P(qt=Si | O, lambda) */
        /* xi[t][i][j] = P(qt=Si, qt+1=Sj | O, lambda) */

        /* 更新pi */
        double gammaSum0 = 0.0;
        for (int i = 0; i < m_numStates; ++i) {
            gammaSum0 += alpha[0][i] * beta[0][i];
        }
        for (int i = 0; i < m_numStates; ++i) {
            if (gammaSum0 > 0) m_pi[i] = (alpha[0][i] * beta[0][i]) / gammaSum0;
        }

        /* 更新A */
        for (int i = 0; i < m_numStates; ++i) {
            double gammaSumI = 0.0;
            for (int t = 0; t < T - 1; ++t) {
                gammaSumI += alpha[t][i] * beta[t][i];
            }
            for (int j = 0; j < m_numStates; ++j) {
                double xiSum = 0.0;
                for (int t = 0; t < T - 1; ++t) {
                    xiSum += alpha[t][i] * m_A[i][j]
                           * m_B[j][observations[t + 1]] * beta[t + 1][j];
                }
                if (gammaSumI > 0) m_A[i][j] = xiSum / gammaSumI;
            }
        }

        /* 更新B */
        for (int i = 0; i < m_numStates; ++i) {
            double gammaSumAll = 0.0;
            for (int t = 0; t < T; ++t) {
                gammaSumAll += alpha[t][i] * beta[t][i];
            }
            for (int k = 0; k < m_numObs; ++k) {
                double gammaK = 0.0;
                for (int t = 0; t < T; ++t) {
                    if (observations[t] == k) {
                        gammaK += alpha[t][i] * beta[t][i];
                    }
                }
                if (gammaSumAll > 0) m_B[i][k] = gammaK / gammaSumAll;
            }
        }

        emit baumWelchIterated(iter, logLL);
    }

    ++m_stats.totalBaumWelchIters += static_cast<quint64>(iter);
    m_stats.lastLogLikelihood = prevLL;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalForwardPasses + m_stats.totalDecodings + 1);

    return {prevLL, iter};
}

/** @brief 重置统计 */
void HiddenMarkovModel2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
