/**
 * @file GaussianMixture10.cpp
 * @brief 高斯混合模型(GMM)实现，支持EM算法拟合与概率预测
 *
 * 实现了基于期望最大化(EM)算法的高斯混合模型聚类。
 * 支持多维数据拟合、BIC模型选择、后验概率预测等功能。
 * 适用于信号分量的概率建模和聚类分析场景。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/cluster51/GaussianMixture10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
GaussianMixture10::GaussianMixture10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置高斯分量数量
 * @param k 分量数，必须大于0
 */
void GaussianMixture10::setComponents(int k)
{
    m_k = qMax(1, k);
    m_weights.clear();
    m_means.clear();
    m_covs.clear();
    m_bic = 0.0;
}

/**
 * @brief 使用EM算法拟合高斯混合模型
 *
 * 算法流程：
 * 1. 随机初始化均值、协方差和权重
 * 2. E步：计算每个样本属于每个分量的后验概率
 * 3. M步：根据后验概率更新参数
 * 4. 计算对数似然和BIC准则
 *
 * @param data 输入数据矩阵，每行一个样本，每列一个特征维度
 * @param maxIter EM算法最大迭代次数，默认100
 * @return 每个样本的聚类标签（硬分配）
 */
QVector<int> GaussianMixture10::fit(const QVector<QVector<double>>& data, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    const int N = data.size();
    if (N == 0) return {};

    m_dim = data[0].size();

    /* 初始化权重为均匀分布 */
    m_weights.resize(m_k);
    m_means.resize(m_k);
    m_covs.resize(m_k);
    for (int j = 0; j < m_k; ++j) {
        m_weights[j] = 1.0 / m_k;
        m_means[j].resize(m_dim, 0.0);
        /* 使用随机样本初始化均值 */
        int idx = (j * N) / m_k;
        for (int d = 0; d < m_dim; ++d) {
            m_means[j][d] = data[idx][d];
        }
        /* 初始化协方差为单位矩阵 */
        m_covs[j].resize(m_dim);
        for (int d = 0; d < m_dim; ++d) {
            m_covs[j][d].resize(m_dim, 0.0);
            m_covs[j][d][d] = 1.0;
        }
    }

    /* 计算全局方差用于初始化协方差 */
    QVector<double> globalMean(m_dim, 0.0);
    for (int i = 0; i < N; ++i)
        for (int d = 0; d < m_dim; ++d)
            globalMean[d] += data[i][d];
    for (int d = 0; d < m_dim; ++d)
        globalMean[d] /= N;

    QVector<double> globalVar(m_dim, 0.0);
    for (int i = 0; i < N; ++i)
        for (int d = 0; d < m_dim; ++d)
            globalVar[d] += (data[i][d] - globalMean[d]) * (data[i][d] - globalMean[d]);
    for (int d = 0; d < m_dim; ++d)
        globalVar[d] = qMax(1e-6, globalVar[d] / N);

    /* 用全局方差初始化协方差 */
    for (int j = 0; j < m_k; ++j) {
        for (int d = 0; d < m_dim; ++d) {
            m_covs[j][d][d] = globalVar[d];
        }
    }

    QVector<int> labels(N, 0);

    /* EM迭代 */
    for (int iter = 0; iter < maxIter; ++iter) {
        /* === E步：计算后验概率 (responsibilities) === */
        QVector<QVector<double>> resp(N, QVector<double>(m_k, 0.0));

        for (int i = 0; i < N; ++i) {
            double sumProb = 0.0;
            for (int j = 0; j < m_k; ++j) {
                /* 计算多元高斯概率密度 */
                double det = 1.0;
                for (int d = 0; d < m_dim; ++d)
                    det *= qMax(1e-10, m_covs[j][d][d]);

                double norm = 1.0 / (qPow(2.0 * M_PI, m_dim / 2.0) * qSqrt(qMax(1e-30, det)));
                double mahal = 0.0;
                for (int d = 0; d < m_dim; ++d) {
                    double diff = data[i][d] - m_means[j][d];
                    mahal += diff * diff / qMax(1e-10, m_covs[j][d][d]);
                }
                resp[i][j] = m_weights[j] * norm * qExp(-0.5 * mahal);
                sumProb += resp[i][j];
            }
            /* 归一化后验概率 */
            if (sumProb > 1e-300) {
                for (int j = 0; j < m_k; ++j)
                    resp[i][j] /= sumProb;
            }
        }

        /* === M步：更新参数 === */
        for (int j = 0; j < m_k; ++j) {
            double Nj = 0.0;
            for (int i = 0; i < N; ++i)
                Nj += resp[i][j];

            if (Nj < 1e-10) continue;

            /* 更新权重 */
            m_weights[j] = Nj / N;

            /* 更新均值 */
            for (int d = 0; d < m_dim; ++d) {
                double sum = 0.0;
                for (int i = 0; i < N; ++i)
                    sum += resp[i][j] * data[i][d];
                m_means[j][d] = sum / Nj;
            }

            /* 更新对角协方差矩阵 */
            for (int d = 0; d < m_dim; ++d) {
                double sum = 0.0;
                for (int i = 0; i < N; ++i) {
                    double diff = data[i][d] - m_means[j][d];
                    sum += resp[i][j] * diff * diff;
                }
                m_covs[j][d][d] = qMax(1e-6, sum / Nj);
            }
        }

        /* 硬分配标签 */
        for (int i = 0; i < N; ++i) {
            double maxResp = -1.0;
            for (int j = 0; j < m_k; ++j) {
                if (resp[i][j] > maxResp) {
                    maxResp = resp[i][j];
                    labels[i] = j;
                }
            }
        }
    }

    /* 计算BIC: BIC = -2 * logLikelihood + p * log(N) */
    double logLik = 0.0;
    for (int i = 0; i < N; ++i) {
        double sampleProb = 0.0;
        for (int j = 0; j < m_k; ++j) {
            double det = 1.0;
            for (int d = 0; d < m_dim; ++d)
                det *= qMax(1e-10, m_covs[j][d][d]);
            double norm = 1.0 / (qPow(2.0 * M_PI, m_dim / 2.0) * qSqrt(qMax(1e-30, det)));
            double mahal = 0.0;
            for (int d = 0; d < m_dim; ++d) {
                double diff = data[i][d] - m_means[j][d];
                mahal += diff * diff / qMax(1e-10, m_covs[j][d][d]);
            }
            sampleProb += m_weights[j] * norm * qExp(-0.5 * mahal);
        }
        if (sampleProb > 1e-300)
            logLik += qLn(sampleProb);
    }
    int numParams = m_k * (1 + m_dim + m_dim) - 1;
    m_bic = -2.0 * logLik + numParams * qLn(N);

    /* 更新统计信息 */
    m_stats.totalFits++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitCompleted(m_k, m_bic);
    return labels;
}

/**
 * @brief 预测样本属于每个高斯分量的后验概率
 * @param sample 单个样本的特征向量
 * @return 每个分量的后验概率向量
 */
QVector<double> GaussianMixture10::predictProb(const QVector<double>& sample) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> probs(m_k, 0.0);
    double sumProb = 0.0;

    for (int j = 0; j < m_k; ++j) {
        double det = 1.0;
        for (int d = 0; d < m_dim; ++d)
            det *= qMax(1e-10, m_covs[j][d][d]);

        double norm = 1.0 / (qPow(2.0 * M_PI, m_dim / 2.0) * qSqrt(qMax(1e-30, det)));
        double mahal = 0.0;
        for (int d = 0; d < m_dim; ++d) {
            double diff = sample[d] - m_means[j][d];
            mahal += diff * diff / qMax(1e-10, m_covs[j][d][d]);
        }
        probs[j] = m_weights[j] * norm * qExp(-0.5 * mahal);
        sumProb += probs[j];
    }

    if (sumProb > 1e-300) {
        for (int j = 0; j < m_k; ++j)
            probs[j] /= sumProb;
    }

    /* 注意：这里不修改mutable统计，因为方法是const */
    Q_UNUSED(timer);
    return probs;
}

/**
 * @brief 重置所有统计数据
 */
void GaussianMixture10::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
