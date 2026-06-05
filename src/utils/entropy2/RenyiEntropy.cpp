/**
 * @file RenyiEntropy.cpp
 * @brief Rényi熵和散度计算器实现
 */

#include "utils/entropy2/RenyiEntropy.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
RenyiEntropy::RenyiEntropy(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief Shannon熵(自然对数底)
 *  @param probabilities 概率分布
 *  @return Shannon熵(nats)
 */
double RenyiEntropy::shannon(const QVector<double>& probabilities) const
{
    double h = 0.0;
    for (double pi : probabilities) {
        if (pi > 0.0) {
            h -= pi * qLn(pi);
        }
    }
    return h;
}

/** @brief 计算Rényi熵
 *  @param probabilities 概率分布
 *  @param alpha 阶数参数
 *  @return Rényi熵值(nats)
 */
double RenyiEntropy::entropy(const QVector<double>& probabilities,
                             double alpha)
{
    QElapsedTimer timer;
    timer.start();

    double result = 0.0;

    if (probabilities.isEmpty()) {
        result = 0.0;
    } else if (alpha <= 0.0) {
        /* alpha->0: Hartley熵 = ln(非零概率数) */
        int count = 0;
        for (double pi : probabilities) {
            if (pi > 0.0) ++count;
        }
        result = (count > 0) ? qLn(static_cast<double>(count)) : 0.0;
    } else if (qAbs(alpha - 1.0) < 1e-10) {
        /* alpha->1: 退化为Shannon熵 */
        result = shannon(probabilities);
    } else {
        /* 通用Rényi熵: H_alpha = ln(sum(p^alpha)) / (1 - alpha) */
        double sumP = 0.0;
        for (double pi : probabilities) {
            if (pi > 0.0) {
                sumP += qPow(pi, alpha);
            }
        }
        sumP = qMax(sumP, 1e-300);
        result = qLn(sumP) / (1.0 - alpha);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(result, tr("Renyi"));
    return result;
}

/** @brief 计算Tsallis熵
 *  @param probabilities 概率分布
 *  @param q 非广延参数
 *  @return Tsallis熵值
 */
double RenyiEntropy::tsallisEntropy(const QVector<double>& probabilities,
                                    double q)
{
    QElapsedTimer timer;
    timer.start();

    double result = 0.0;

    if (probabilities.isEmpty()) {
        result = 0.0;
    } else if (qAbs(q - 1.0) < 1e-10) {
        /* q->1: 退化为Shannon熵 */
        result = shannon(probabilities);
    } else {
        /* Tsallis熵: S_q = (sum(p^q) - 1) / (1 - q) */
        double sumP = 0.0;
        for (double pi : probabilities) {
            if (pi > 0.0) {
                sumP += qPow(pi, q);
            }
        }
        result = (sumP - 1.0) / (1.0 - q);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(result, tr("Tsallis"));
    return result;
}

/** @brief 计算Rényi散度
 *  @param p 第一个概率分布
 *  @param q 第二个概率分布(参考分布)
 *  @param alpha 阶数参数
 *  @return Rényi散度值
 */
double RenyiEntropy::divergence(const QVector<double>& p,
                                const QVector<double>& q,
                                double alpha)
{
    QElapsedTimer timer;
    timer.start();

    double result = 0.0;

    int n = qMin(p.size(), q.size());
    if (n == 0 || alpha <= 0.0 || qAbs(alpha - 1.0) < 1e-10) {
        /* alpha->1: 退化为KL散度 */
        double kl = 0.0;
        for (int i = 0; i < n; ++i) {
            if (p[i] > 0.0 && q[i] > 0.0) {
                kl += p[i] * qLn(p[i] / q[i]);
            }
        }
        result = kl;
    } else {
        /* D_alpha(p||q) = ln(sum(p^alpha * q^(1-alpha))) / (alpha - 1) */
        double sumVal = 0.0;
        for (int i = 0; i < n; ++i) {
            if (p[i] > 0.0 && q[i] > 0.0) {
                sumVal += qPow(p[i], alpha) * qPow(q[i], 1.0 - alpha);
            }
        }
        sumVal = qMax(sumVal, 1e-300);
        result = qLn(sumVal) / (alpha - 1.0);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(result, tr("RenyiDivergence"));
    return result;
}

/** @brief 计算互信息I(X;Y)
 *  @param jointDist 联合概率分布矩阵
 *  @return 互信息值(nats)
 */
double RenyiEntropy::mutualInformation(
    const QVector<QVector<double>>& jointDist)
{
    QElapsedTimer timer;
    timer.start();

    int nRows = jointDist.size();
    if (nRows == 0) {
        emit computationCompleted(0.0, tr("MutualInfo"));
        return 0.0;
    }
    int nCols = jointDist[0].size();

    /* 计算边缘分布 */
    QVector<double> pX(nRows, 0.0);  ///< X边缘分布
    QVector<double> pY(nCols, 0.0);  ///< Y边缘分布

    for (int i = 0; i < nRows; ++i) {
        for (int j = 0; j < nCols; ++j) {
            double p = jointDist[i][j];
            pX[i] += p;
            pY[j] += p;
        }
    }

    /* I(X;Y) = sum p(x,y) * ln(p(x,y) / (p(x)*p(y))) */
    double mi = 0.0;
    for (int i = 0; i < nRows; ++i) {
        for (int j = 0; j < nCols; ++j) {
            double pxy = jointDist[i][j];
            if (pxy > 0.0 && pX[i] > 0.0 && pY[j] > 0.0) {
                mi += pxy * qLn(pxy / (pX[i] * pY[j]));
            }
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(mi, tr("MutualInfo"));
    return mi;
}

/** @brief 计算条件熵H(Y|X)
 *  @param jointDist 联合概率分布矩阵
 *  @return 条件熵值(nats)
 */
double RenyiEntropy::conditionalEntropy(
    const QVector<QVector<double>>& jointDist)
{
    QElapsedTimer timer;
    timer.start();

    int nRows = jointDist.size();
    if (nRows == 0) {
        emit computationCompleted(0.0, tr("ConditionalEntropy"));
        return 0.0;
    }
    int nCols = jointDist[0].size();

    /* 计算X边缘分布 */
    QVector<double> pX(nRows, 0.0);
    for (int i = 0; i < nRows; ++i) {
        for (int j = 0; j < nCols; ++j) {
            pX[i] += jointDist[i][j];
        }
    }

    /* H(Y|X) = sum_x sum_y p(x,y) * ln(p(x) / p(x,y)) */
    double condH = 0.0;
    for (int i = 0; i < nRows; ++i) {
        if (pX[i] <= 0.0) continue;
        for (int j = 0; j < nCols; ++j) {
            double pxy = jointDist[i][j];
            if (pxy > 0.0) {
                condH += pxy * qLn(pX[i] / pxy);
            }
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(condH, tr("ConditionalEntropy"));
    return condH;
}

/** @brief 重置统计信息 */
void RenyiEntropy::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
