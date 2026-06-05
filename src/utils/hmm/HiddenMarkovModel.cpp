/**
 * @file HiddenMarkovModel.cpp
 * @brief 隐马尔可夫模型实现 — 前向/后向/Viterbi/Baum-Welch
 */

#include "utils/hmm/HiddenMarkovModel.h"

#include <QtMath>
#include <QElapsedTimer>
#include <algorithm>

HiddenMarkovModel::HiddenMarkovModel(int states, int symbols, QObject* parent)
    : QObject(parent), m_states(qMax(2, states)), m_symbols(qMax(1, symbols)),
      m_timeSum(0.0)
{
    /* 均匀初始化 */
    double initState = 1.0 / m_states;
    m_A.resize(m_states);
    for (auto& row : m_A) {
        row.resize(m_states, initState);
    }
    m_B.resize(m_states);
    double initEmit = 1.0 / m_symbols;
    for (auto& row : m_B) {
        row.resize(m_symbols, initEmit);
    }
    m_pi.resize(m_states, initState);
}

void HiddenMarkovModel::setTransitionMatrix(const QVector<QVector<double>>& A)
{
    if (A.size() != m_states) return;
    m_A = A;
}

void HiddenMarkovModel::setEmissionMatrix(const QVector<QVector<double>>& B)
{
    if (B.size() != m_states) return;
    m_B = B;
}

void HiddenMarkovModel::setInitialProbabilities(const QVector<double>& pi)
{
    if (pi.size() != m_states) return;
    m_pi = pi;
}

double HiddenMarkovModel::forward(const QVector<int>& observations) const
{
    int T = observations.size();
    if (T == 0) return 0.0;

    /* alpha[t][i] */
    QVector<QVector<double>> alpha(T, QVector<double>(m_states, 0.0));

    /* 初始化 */
    for (int i = 0; i < m_states; ++i) {
        int obs = qBound(0, observations[0], m_symbols - 1);
        alpha[0][i] = m_pi[i] * m_B[i][obs];
    }

    /* 递推 */
    for (int t = 1; t < T; ++t) {
        int obs = qBound(0, observations[t], m_symbols - 1);
        for (int j = 0; j < m_states; ++j) {
            double sum = 0.0;
            for (int i = 0; i < m_states; ++i) {
                sum += alpha[t - 1][i] * m_A[i][j];
            }
            alpha[t][j] = sum * m_B[j][obs];
        }
    }

    /* 终止 */
    double prob = 0.0;
    for (int i = 0; i < m_states; ++i) prob += alpha[T - 1][i];
    return prob;
}

double HiddenMarkovModel::backward(const QVector<int>& observations) const
{
    int T = observations.size();
    if (T == 0) return 0.0;

    QVector<QVector<double>> beta(T, QVector<double>(m_states, 0.0));

    /* 初始化 */
    for (int i = 0; i < m_states; ++i) beta[T - 1][i] = 1.0;

    /* 递推 */
    for (int t = T - 2; t >= 0; --t) {
        int obs = qBound(0, observations[t + 1], m_symbols - 1);
        for (int i = 0; i < m_states; ++i) {
            double sum = 0.0;
            for (int j = 0; j < m_states; ++j) {
                sum += m_A[i][j] * m_B[j][obs] * beta[t + 1][j];
            }
            beta[t][i] = sum;
        }
    }

    double prob = 0.0;
    int obs0 = qBound(0, observations[0], m_symbols - 1);
    for (int i = 0; i < m_states; ++i) prob += m_pi[i] * m_B[i][obs0] * beta[0][i];
    return prob;
}

QVector<int> HiddenMarkovModel::viterbi(const QVector<int>& observations) const
{
    int T = observations.size();
    QVector<int> path(T, 0);
    if (T == 0) return path;

    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> delta(T, QVector<double>(m_states, 0.0));
    QVector<QVector<int>> psi(T, QVector<int>(m_states, 0));

    int obs0 = qBound(0, observations[0], m_symbols - 1);
    for (int i = 0; i < m_states; ++i) {
        delta[0][i] = m_pi[i] * m_B[i][obs0];
    }

    for (int t = 1; t < T; ++t) {
        int obs = qBound(0, observations[t], m_symbols - 1);
        for (int j = 0; j < m_states; ++j) {
            double bestVal = -1.0;
            int bestIdx = 0;
            for (int i = 0; i < m_states; ++i) {
                double val = delta[t - 1][i] * m_A[i][j];
                if (val > bestVal) { bestVal = val; bestIdx = i; }
            }
            delta[t][j] = bestVal * m_B[j][obs];
            psi[t][j] = bestIdx;
        }
    }

    /* 回溯 */
    double bestLast = -1.0;
    for (int i = 0; i < m_states; ++i) {
        if (delta[T - 1][i] > bestLast) {
            bestLast = delta[T - 1][i];
            path[T - 1] = i;
        }
    }
    for (int t = T - 2; t >= 0; --t) {
        path[t] = psi[t + 1][path[t + 1]];
    }

    m_stats.totalDecodings++;
    m_stats.totalObservationsProcessed += T;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalDecodings + m_stats.totalTrainings, 1ULL);

    emit decodingCompleted(T);
    return path;
}

double HiddenMarkovModel::train(const QVector<int>& observations,
                                 int maxIterations, double tolerance)
{
    int T = observations.size();
    if (T < 2) return 0.0;

    QElapsedTimer timer;
    timer.start();

    double prevLogLik = -1e30;

    for (int iter = 0; iter < maxIterations; ++iter) {
        /* E-step: 计算alpha, beta, gamma, xi */
        QVector<QVector<double>> alpha(T, QVector<double>(m_states, 0.0));
        QVector<QVector<double>> beta(T, QVector<double>(m_states, 0.0));

        /* Forward */
        int obs0 = qBound(0, observations[0], m_symbols - 1);
        for (int i = 0; i < m_states; ++i)
            alpha[0][i] = m_pi[i] * m_B[i][obs0];

        for (int t = 1; t < T; ++t) {
            int obs = qBound(0, observations[t], m_symbols - 1);
            for (int j = 0; j < m_states; ++j) {
                double sum = 0.0;
                for (int i = 0; i < m_states; ++i) sum += alpha[t - 1][i] * m_A[i][j];
                alpha[t][j] = sum * m_B[j][obs];
            }
        }

        double scaleSum = 0.0;
        for (int i = 0; i < m_states; ++i) scaleSum += alpha[T - 1][i];

        /* Backward */
        for (int i = 0; i < m_states; ++i) beta[T - 1][i] = 1.0;
        for (int t = T - 2; t >= 0; --t) {
            int obs = qBound(0, observations[t + 1], m_symbols - 1);
            for (int i = 0; i < m_states; ++i) {
                double sum = 0.0;
                for (int j = 0; j < m_states; ++j)
                    sum += m_A[i][j] * m_B[j][obs] * beta[t + 1][j];
                beta[t][i] = sum;
            }
        }

        double logLik = (scaleSum > 0) ? qLn(scaleSum) : -1e30;
        if (qAbs(logLik - prevLogLik) < tolerance) break;
        prevLogLik = logLik;

        if (scaleSum < 1e-30) break;

        /* Gamma */
        QVector<QVector<double>> gamma(T, QVector<double>(m_states, 0.0));
        for (int t = 0; t < T; ++t) {
            double rowSum = 0.0;
            for (int i = 0; i < m_states; ++i) {
                gamma[t][i] = alpha[t][i] * beta[t][i];
                rowSum += gamma[t][i];
            }
            if (rowSum > 0) for (int i = 0; i < m_states; ++i) gamma[t][i] /= rowSum;
        }

        /* Xi */
        QVector<QVector<QVector<double>>> xi(
            T - 1, QVector<QVector<double>>(m_states, QVector<double>(m_states, 0.0)));
        for (int t = 0; t < T - 1; ++t) {
            int obs = qBound(0, observations[t + 1], m_symbols - 1);
            double xiSum = 0.0;
            for (int i = 0; i < m_states; ++i) {
                for (int j = 0; j < m_states; ++j) {
                    xi[t][i][j] = alpha[t][i] * m_A[i][j] * m_B[j][obs] * beta[t + 1][j];
                    xiSum += xi[t][i][j];
                }
            }
            if (xiSum > 0) {
                for (int i = 0; i < m_states; ++i)
                    for (int j = 0; j < m_states; ++j)
                        xi[t][i][j] /= xiSum;
            }
        }

        /* M-step: 更新参数 */
        for (int i = 0; i < m_states; ++i) m_pi[i] = gamma[0][i];

        for (int i = 0; i < m_states; ++i) {
            double gammaSum = 0.0;
            for (int t = 0; t < T - 1; ++t) gammaSum += gamma[t][i];
            for (int j = 0; j < m_states; ++j) {
                double xiSum = 0.0;
                for (int t = 0; t < T - 1; ++t) xiSum += xi[t][i][j];
                m_A[i][j] = (gammaSum > 0) ? xiSum / gammaSum : 1.0 / m_states;
            }
        }

        for (int i = 0; i < m_states; ++i) {
            double gammaSum = 0.0;
            for (int t = 0; t < T; ++t) gammaSum += gamma[t][i];
            for (int k = 0; k < m_symbols; ++k) {
                double emitSum = 0.0;
                for (int t = 0; t < T; ++t) {
                    if (observations[t] == k) emitSum += gamma[t][i];
                }
                m_B[i][k] = (gammaSum > 0) ? emitSum / gammaSum : 1.0 / m_symbols;
            }
        }
    }

    m_stats.totalTrainings++;
    m_stats.totalObservationsProcessed += T;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalDecodings + m_stats.totalTrainings, 1ULL);

    emit trainingCompleted(maxIterations, prevLogLik);
    return prevLogLik;
}

void HiddenMarkovModel::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
