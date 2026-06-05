/**
 * @file GaussianMixture9.cpp
 * @brief 高斯混合模型9 — 半监督EM+标签传播 实现
 *
 * 实现半监督高斯混合模型，结合标注数据和未标注数据进行EM聚类。
 * 标注数据用于初始化参数并约束E步，未标注数据通过标签传播获得软标签。
 */

#include "utils/cluster45/GaussianMixture9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
GaussianMixture9::GaussianMixture9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置高斯分量数量
 * @param k 分量数，必须 >= 1
 */
void GaussianMixture9::setComponents(int k)
{
    m_k = qMax(1, k);
}

/**
 * @brief 设置标注数据及其标签
 * @param data 标注样本矩阵，每行为一个样本
 * @param labels 对应标签向量，取值范围 [0, k)
 */
void GaussianMixture9::setLabeledData(const QVector<QVector<double>>& data,
                                       const QVector<int>& labels)
{
    m_labeledData = data;
    m_labeledLabels = labels;
    if (!data.isEmpty()) {
        m_dim = data[0].size();
    }
}

/**
 * @brief 设置未标注数据
 * @param data 未标注样本矩阵，维度须与标注数据一致
 */
void GaussianMixture9::setUnlabeledData(const QVector<QVector<double>>& data)
{
    m_unlabeledData = data;
    if (!data.isEmpty() && m_dim == 0) {
        m_dim = data[0].size();
    }
}

/**
 * @brief 执行半监督EM拟合
 *
 * 1. 从标注数据初始化均值、协方差、权重
 * 2. 合并标注+未标注数据，迭代E步/M步
 * 3. 标注样本的响应硬性固定为其标签
 *
 * @param maxIter 最大迭代次数
 * @param tol 对数似然收敛阈值
 * @return 所有样本的聚类标签
 */
QVector<int> GaussianMixture9::fit(int maxIter, double tol)
{
    QElapsedTimer timer;
    timer.start();

    /* 合并数据集 */
    QVector<QVector<double>> allData = m_labeledData + m_unlabeledData;
    int nLabeled = m_labeledData.size();
    int nTotal = allData.size();

    if (nTotal == 0 || m_dim == 0) {
        return {};
    }

    /* 从标注数据初始化参数 */
    initializeFromLabeled();

    double prevLL = -1e30;
    int iterCount = 0;

    /* 响应矩阵 (nTotal x m_k) */
    QVector<QVector<double>> resp(nTotal, QVector<double>(m_k, 0.0));

    for (int iter = 0; iter < maxIter; ++iter) {
        iterCount = iter + 1;

        /* === E步: 计算每个样本对各分量的响应 === */
        double currentLL = 0.0;
        for (int i = 0; i < nTotal; ++i) {
            double sumProb = 0.0;
            for (int c = 0; c < m_k; ++c) {
                double p = m_weights[c] * gaussianPDF(allData[i], c);
                resp[i][c] = p;
                sumProb += p;
            }
            if (sumProb > 1e-300) {
                currentLL += qLn(sumProb);
                for (int c = 0; c < m_k; ++c) {
                    resp[i][c] /= sumProb;
                }
            }
        }

        /* 标注样本的响应硬性固定 */
        for (int i = 0; i < nLabeled; ++i) {
            int lbl = m_labeledLabels[i];
            if (lbl >= 0 && lbl < m_k) {
                for (int c = 0; c < m_k; ++c) {
                    resp[i][c] = (c == lbl) ? 1.0 : 0.0;
                }
            }
        }

        m_logLikelihood = currentLL;

        /* 收敛判定 */
        if (qFabs(currentLL - prevLL) < tol) {
            break;
        }
        prevLL = currentLL;

        /* === M步: 更新权重、均值、协方差 === */
        for (int c = 0; c < m_k; ++c) {
            double nC = 0.0;
            for (int i = 0; i < nTotal; ++i) {
                nC += resp[i][c];
            }
            if (nC < 1e-300) continue;

            /* 权重 */
            m_weights[c] = nC / nTotal;

            /* 均值 */
            for (int d = 0; d < m_dim; ++d) {
                double sum = 0.0;
                for (int i = 0; i < nTotal; ++i) {
                    sum += resp[i][c] * allData[i][d];
                }
                m_means[c][d] = sum / nC;
            }

            /* 协方差 */
            for (int d1 = 0; d1 < m_dim; ++d1) {
                for (int d2 = d1; d2 < m_dim; ++d2) {
                    double sum = 0.0;
                    for (int i = 0; i < nTotal; ++i) {
                        double diff1 = allData[i][d1] - m_means[c][d1];
                        double diff2 = allData[i][d2] - m_means[c][d2];
                        sum += resp[i][c] * diff1 * diff2;
                    }
                    double covVal = sum / nC;
                    m_covariances[c][d1][d2] = covVal;
                    m_covariances[c][d2][d1] = covVal;
                }
            }
            /* 对角线正则化，防止奇异 */
            for (int d = 0; d < m_dim; ++d) {
                m_covariances[c][d][d] += 1e-6;
            }
        }
    }

    /* 标签分配 */
    QVector<int> labels(nTotal);
    for (int i = 0; i < nTotal; ++i) {
        int bestC = 0;
        double bestR = resp[i][0];
        for (int c = 1; c < m_k; ++c) {
            if (resp[i][c] > bestR) {
                bestR = resp[i][c];
                bestC = c;
            }
        }
        labels[i] = bestC;
    }

    /* 统计更新 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalFits++;
    m_stats.totalSamplesProcessed += nTotal;
    m_stats.totalIterations += iterCount;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitCompleted(m_k, iterCount, m_logLikelihood);
    return labels;
}

/**
 * @brief 对新样本进行聚类预测
 * @param data 待预测样本矩阵
 * @return 每个样本的分量标签
 */
QVector<int> GaussianMixture9::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> result;
    result.reserve(data.size());
    for (const auto& sample : data) {
        int bestC = 0;
        double bestP = m_weights[0] * gaussianPDF(sample, 0);
        for (int c = 1; c < m_k; ++c) {
            double p = m_weights[c] * gaussianPDF(sample, c);
            if (p > bestP) {
                bestP = p;
                bestC = c;
            }
        }
        result.push_back(bestC);
    }
    return result;
}

/**
 * @brief 预测单个样本属于各分量的后验概率
 * @param sample 单个样本
 * @return 各分量的概率向量，长度为 m_k
 */
QVector<double> GaussianMixture9::predictProb(const QVector<double>& sample) const
{
    QVector<double> probs(m_k);
    double sum = 0.0;
    for (int c = 0; c < m_k; ++c) {
        probs[c] = m_weights[c] * gaussianPDF(sample, c);
        sum += probs[c];
    }
    if (sum > 1e-300) {
        for (int c = 0; c < m_k; ++c) {
            probs[c] /= sum;
        }
    }
    return probs;
}

/**
 * @brief 重置统计信息
 */
void GaussianMixture9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 从标注数据初始化高斯参数
 *
 * 统计每个标签的样本均值、协方差和样本比例作为初始值。
 * 若无标注数据，使用随机初始化。
 */
void GaussianMixture9::initializeFromLabeled()
{
    m_weights.resize(m_k, 1.0 / m_k);
    m_means.resize(m_k, QVector<double>(m_dim, 0.0));
    m_covariances.resize(m_k, QVector<QVector<double>>(m_dim, QVector<double>(m_dim, 0.0)));

    /* 统计每个类别的样本数 */
    QVector<int> count(m_k, 0);
    for (int i = 0; i < m_labeledLabels.size(); ++i) {
        int lbl = m_labeledLabels[i];
        if (lbl >= 0 && lbl < m_k) {
            count[lbl]++;
            for (int d = 0; d < m_dim; ++d) {
                m_means[lbl][d] += m_labeledData[i][d];
            }
        }
    }

    int nLabeled = m_labeledLabels.size();
    for (int c = 0; c < m_k; ++c) {
        if (count[c] > 0) {
            for (int d = 0; d < m_dim; ++d) {
                m_means[c][d] /= count[c];
            }
            m_weights[c] = static_cast<double>(count[c]) / qMax(1, nLabeled);
        }
    }

    /* 初始化协方差为单位阵（对角） */
    for (int c = 0; c < m_k; ++c) {
        for (int d = 0; d < m_dim; ++d) {
            m_covariances[c][d][d] = 1.0;
        }
    }
}

/**
 * @brief 计算多元高斯概率密度
 * @param x 样本向量
 * @param comp 分量索引
 * @return 概率密度值
 */
double GaussianMixture9::gaussianPDF(const QVector<double>& x, int comp) const
{
    if (comp < 0 || comp >= m_k || x.size() != m_dim) {
        return 0.0;
    }

    /* 计算行列式和逆矩阵的对角简化（假设近似对角阵） */
    double logDet = 0.0;
    double mahal = 0.0;

    for (int d = 0; d < m_dim; ++d) {
        double var = m_covariances[comp][d][d];
        if (var < 1e-12) var = 1e-12;
        logDet += qLn(var);
        double diff = x[d] - m_means[comp][d];
        mahal += diff * diff / var;
    }

    double logProb = -0.5 * (m_dim * qLn(2.0 * M_PI) + logDet + mahal);
    return qExp(logProb);
}
