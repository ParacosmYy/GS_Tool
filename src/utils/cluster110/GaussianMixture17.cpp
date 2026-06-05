#include "GaussianMixture17.h"
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化高斯混合模型引擎
 * @param parent 父对象指针
 */
GaussianMixture17::GaussianMixture17(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void GaussianMixture17::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置高斯分量数量K
 * @param k 分量数量
 */
void GaussianMixture17::setComponentCount(int k)
{
    m_components = qMax(1, k);
}

/**
 * @brief 设置协方差类型："diag"对角或"full"满矩阵
 * @param type 协方差类型字符串
 */
void GaussianMixture17::setCovarianceType(const QString& type)
{
    m_covType = type;
}

/**
 * @brief 设置EM算法最大迭代次数和收敛阈值
 * @param maxIterations 最大迭代次数
 * @param tolerance 对数似然收敛阈值
 */
void GaussianMixture17::setEMParams(int maxIterations, double tolerance)
{
    m_maxIter = qMax(10, maxIterations);
    m_tolerance = qMax(1e-10, tolerance);
}

/**
 * @brief 计算多元高斯对数概率密度（对角协方差）
 * @param x 样本向量
 * @param mean 均值向量
 * @param diagVar 对角方差向量
 * @return 对数概率密度值
 */
static double logGaussianPDF(const QVector<double>& x,
                              const QVector<double>& mean,
                              const QVector<double>& diagVar)
{
    const int dim = qMin(x.size(), qMin(mean.size(), diagVar.size()));
    double logDet = 0.0;
    double mahal = 0.0;
    for (int d = 0; d < dim; ++d) {
        double var = qMax(diagVar[d], 1e-8);
        logDet += qLn(var);
        double diff = x[d] - mean[d];
        mahal += diff * diff / var;
    }
    return -0.5 * (dim * qLn(2.0 * M_PI) + logDet + mahal);
}

/**
 * @brief log-sum-exp技巧，数值稳定地计算 log(sum(exp(x)))
 * @param values 对数值列表
 * @return log(sum(exp(values)))
 */
static double logSumExp(const QVector<double>& values)
{
    if (values.isEmpty()) return 0.0;
    double maxVal = values[0];
    for (double v : values) maxVal = qMax(maxVal, v);
    double sum = 0.0;
    for (double v : values) sum += qExp(v - maxVal);
    return maxVal + qLn(sum);
}

/**
 * @brief 对输入数据拟合GMM，返回每个样本的簇概率分布
 *
 * EM算法流程：
 * 1. 使用K-Means++风格选择初始均值
 * 2. E步：计算每个样本在各分量下的后验概率（责任度）
 * 3. M步：根据责任度更新均值、协方差和混合权重
 * 4. 计算对数似然判断收敛
 *
 * @param data n×dim数据矩阵
 * @return n×k后验概率矩阵
 */
QVector<QVector<double>> GaussianMixture17::fitPredict(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    const int dim = (n > 0) ? data[0].size() : 0;
    const int k = qMin(m_components, n);
    QVector<QVector<double>> resp(n, QVector<double>(k, 1.0 / k));

    if (n < k || dim < 1) {
        emit fittingCompleted(k, 0.0);
        return resp;
    }

    /* K-Means++风格初始化均值 */
    QVector<QVector<double>> means(k, QVector<double>(dim, 0.0));
    means[0] = data[0];
    for (int c = 1; c < k; ++c) {
        QVector<double> minDists(n, 1e18);
        for (int i = 0; i < n; ++i) {
            for (int cc = 0; cc < c; ++cc) {
                double d = 0.0;
                for (int dd = 0; dd < dim; ++dd) {
                    double diff = data[i][dd] - means[cc][dd];
                    d += diff * diff;
                }
                minDists[i] = qMin(minDists[i], d);
            }
        }
        double total = 0.0;
        for (double d : minDists) total += d;
        if (total < 1e-12) {
            means[c] = data[c % n];
            continue;
        }
        double r = QRandomGenerator::global()->generateDouble() * total;
        double cum = 0.0;
        int chosen = c % n;
        for (int i = 0; i < n; ++i) {
            cum += minDists[i];
            if (cum >= r) { chosen = i; break; }
        }
        means[c] = data[chosen];
    }

    /* 初始化协方差（使用全局方差）和权重 */
    QVector<QVector<double>> variances(k, QVector<double>(dim, 1.0));
    QVector<double> weights(k, 1.0 / k);

    for (int d = 0; d < dim; ++d) {
        double globalMean = 0.0;
        for (int i = 0; i < n; ++i) globalMean += data[i][d];
        globalMean /= n;
        double globalVar = 0.0;
        for (int i = 0; i < n; ++i) {
            double diff = data[i][d] - globalMean;
            globalVar += diff * diff;
        }
        globalVar /= n;
        for (int c = 0; c < k; ++c) variances[c][d] = qMax(globalVar, 1e-6);
    }

    double prevLL = -1e18;

    /* EM迭代 */
    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* E步：计算后验概率（责任度） */
        for (int i = 0; i < n; ++i) {
            QVector<double> logProbs(k);
            for (int c = 0; c < k; ++c) {
                logProbs[c] = qLn(qMax(weights[c], 1e-300))
                              + logGaussianPDF(data[i], means[c], variances[c]);
            }
            double lse = logSumExp(logProbs);
            for (int c = 0; c < k; ++c) {
                resp[i][c] = qExp(logProbs[c] - lse);
            }
        }

        /* M步：更新参数 */
        QVector<double> nk(k, 0.0);
        for (int i = 0; i < n; ++i)
            for (int c = 0; c < k; ++c) nk[c] += resp[i][c];

        for (int c = 0; c < k; ++c) {
            if (nk[c] < 1e-10) continue;
            weights[c] = nk[c] / n;

            /* 更新均值 */
            for (int d = 0; d < dim; ++d) {
                double sum = 0.0;
                for (int i = 0; i < n; ++i) sum += resp[i][c] * data[i][d];
                means[c][d] = sum / nk[c];
            }

            /* 更新对角协方差 */
            for (int d = 0; d < dim; ++d) {
                double sum = 0.0;
                for (int i = 0; i < n; ++i) {
                    double diff = data[i][d] - means[c][d];
                    sum += resp[i][c] * diff * diff;
                }
                variances[c][d] = qMax(sum / nk[c], 1e-6);
            }
        }

        /* 计算对数似然 */
        double ll = 0.0;
        for (int i = 0; i < n; ++i) {
            QVector<double> logProbs(k);
            for (int c = 0; c < k; ++c) {
                logProbs[c] = qLn(qMax(weights[c], 1e-300))
                              + logGaussianPDF(data[i], means[c], variances[c]);
            }
            ll += logSumExp(logProbs);
        }

        if (qAbs(ll - prevLL) < m_tolerance) break;
        prevLL = ll;
    }

    /* 缓存模型参数供scoreSamples使用 */
    m_means = means;
    m_variances = variances;
    m_weights = weights;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalFitted++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFitted;

    emit fittingCompleted(k, prevLL);
    return resp;
}

/**
 * @brief 计算新样本在各分量下的对数似然值
 *
 * 使用log-sum-exp技巧保证数值稳定性。
 * 需要先调用fitPredict()拟合模型。
 *
 * @param samples 待评估样本集合
 * @return 各样本的对数似然值
 */
QVector<double> GaussianMixture17::scoreSamples(const QVector<QVector<double>>& samples)
{
    const int n = samples.size();
    if (n == 0 || m_means.isEmpty()) return {};

    const int k = m_means.size();
    QVector<double> scores(n, 0.0);

    for (int i = 0; i < n; ++i) {
        QVector<double> logProbs(k);
        for (int c = 0; c < k; ++c) {
            logProbs[c] = qLn(qMax(m_weights[c], 1e-300))
                          + logGaussianPDF(samples[i], m_means[c], m_variances[c]);
        }
        scores[i] = logSumExp(logProbs);
    }
    return scores;
}
