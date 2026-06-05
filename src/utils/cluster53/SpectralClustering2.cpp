/**
 * @file SpectralClustering2.cpp
 * @brief 谱聚类算法实现 — 相似度矩阵构建 + 特征向量嵌入 + K-Means聚类
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现基于拉普拉斯矩阵的谱聚类：先构建高斯相似度矩阵，
 * 计算归一化拉普拉斯矩阵的前 k 个最小特征向量，
 * 然后在低维嵌入空间上运行 K-Means 得到最终聚类标签。
 */

#include "utils/cluster53/SpectralClustering2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认聚类参数
 * @param parent 父QObject对象
 */
SpectralClustering2::SpectralClustering2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SpectralClustering2"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置聚类数目
 * @param k 目标聚类数，必须 >= 2
 */
void SpectralClustering2::setNumClusters(int k)
{
    m_k = qMax(2, k);
}

/**
 * @brief 设置高斯核函数的带宽参数 sigma
 *
 * sigma 越大，远距离点的相似度越高，聚类越倾向于合并。
 * sigma 越小，只有非常接近的点才被认为是相似的。
 *
 * @param sigma 高斯核带宽，必须 > 0
 */
void SpectralClustering2::setSigma(double sigma)
{
    m_sigma = qMax(1e-6, sigma);
}

// ──────────────────────────────────────────────
// 核心聚类接口
// ──────────────────────────────────────────────

/**
 * @brief 对输入数据点执行谱聚类
 *
 * 完整流程：
 * 1. 构建高斯相似度矩阵 W
 * 2. 计算度矩阵 D 并构建归一化拉普拉斯 L = D^{-1/2} W D^{-1/2}
 * 3. 对 L 进行幂迭代求前 k 个特征向量
 * 4. 在特征向量构成的低维空间上运行 K-Means
 *
 * @param points 输入数据点集合，每个元素为一维特征向量
 * @return 聚类标签数组，与输入点一一对应（0-based）
 */
QVector<int> SpectralClustering2::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int n = points.size();
    if (n == 0) {
        return {};
    }

    // 步骤1：构建相似度矩阵
    QVector<QVector<double>> W = buildSimilarityMatrix(points);

    // 步骤2：计算度矩阵 D 的对角线
    QVector<double> d(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            d[i] += W[i][j];
        }
    }

    // 计算 D^{-1/2}
    QVector<double> dInvSqrt(n, 0.0);
    for (int i = 0; i < n; ++i) {
        dInvSqrt[i] = (d[i] > 1e-12) ? 1.0 / qSqrt(d[i]) : 0.0;
    }

    // 归一化拉普拉斯 L_norm = D^{-1/2} W D^{-1/2}
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            L[i][j] = dInvSqrt[i] * W[i][j] * dInvSqrt[j];
        }
    }

    // 步骤3：幂迭代法求前 k 个特征向量
    m_eigvecs.clear();
    const int k = qMin(m_k, n);
    QVector<double> prevVec;

    for (int eig = 0; eig < k; ++eig) {
        // 随机初始化
        QVector<double> v(n, 0.0);
        for (int i = 0; i < n; ++i) {
            v[i] = qSin(i * 1.0 + eig * 7.3 + 0.5); // 确定性伪随机
        }

        // 归一化
        double norm = 0.0;
        for (int i = 0; i < n; ++i) {
            norm += v[i] * v[i];
        }
        norm = qSqrt(qMax(norm, 1e-12));
        for (int i = 0; i < n; ++i) {
            v[i] /= norm;
        }

        // 正交化：减去已求特征向量方向的投影
        for (const auto& ev : m_eigvecs) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i) {
                dot += v[i] * ev[i];
            }
            for (int i = 0; i < n; ++i) {
                v[i] -= dot * ev[i];
            }
        }

        // 幂迭代
        for (int iter = 0; iter < 200; ++iter) {
            prevVec = v;

            // 矩阵向量乘法
            QVector<double> newV(n, 0.0);
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    newV[i] += L[i][j] * v[j];
                }
            }

            // 再次正交化
            for (const auto& ev : m_eigvecs) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) {
                    dot += newV[i] * ev[i];
                }
                for (int i = 0; i < n; ++i) {
                    newV[i] -= dot * ev[i];
                }
            }

            // 归一化
            norm = 0.0;
            for (int i = 0; i < n; ++i) {
                norm += newV[i] * newV[i];
            }
            norm = qSqrt(qMax(norm, 1e-12));
            for (int i = 0; i < n; ++i) {
                newV[i] /= norm;
            }

            // 检查收敛
            double diff = 0.0;
            for (int i = 0; i < n; ++i) {
                diff += (newV[i] - prevVec[i]) * (newV[i] - prevVec[i]);
            }

            v = newV;
            if (diff < 1e-10) {
                break;
            }
        }

        m_eigvecs.append(v);
    }

    // 步骤4：构建嵌入矩阵（每行为一个点在k维空间的表示）
    QVector<QVector<double>> embedding(n, QVector<double>(k, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            embedding[i][j] = m_eigvecs[j][i];
        }
        // 归一化嵌入向量
        double norm = 0.0;
        for (int j = 0; j < k; ++j) {
            norm += embedding[i][j] * embedding[i][j];
        }
        norm = qSqrt(qMax(norm, 1e-12));
        for (int j = 0; j < k; ++j) {
            embedding[i][j] /= norm;
        }
    }

    // 步骤5：K-Means聚类
    QVector<int> labels = kMeansOnEmbedding(embedding, k);

    // 更新统计信息
    const double elapsed = timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.totalPoints += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    // 计算轮廓系数近似
    double silhouette = 0.0;
    if (n > 1 && k > 1) {
        int sampleCount = qMin(100, n);
        double silSum = 0.0;
        for (int s = 0; s < sampleCount; ++s) {
            int idx = s * n / sampleCount;
            double intraDist = 0.0;
            int intraCount = 0;
            double minInterDist = 1e18;
            int interLabel = -1;

            for (int j = 0; j < n; ++j) {
                if (j == idx) continue;
                double dist = 0.0;
                for (int d = 0; d < k; ++d) {
                    double diff = embedding[idx][d] - embedding[j][d];
                    dist += diff * diff;
                }
                if (labels[j] == labels[idx]) {
                    intraDist += qSqrt(dist);
                    intraCount++;
                } else {
                    double d = qSqrt(dist);
                    if (d < minInterDist) {
                        minInterDist = d;
                        interLabel = labels[j];
                    }
                }
            }
            double a = (intraCount > 0) ? intraDist / intraCount : 0.0;
            double b = (interLabel >= 0) ? minInterDist : 0.0;
            double denom = qMax(qMax(a, b), 1e-12);
            silSum += (b - a) / denom;
        }
        silhouette = silSum / sampleCount;
    }

    emit clusteringCompleted(k, silhouette);
    return labels;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含聚类次数、总点数和平均耗时的Stats结构
 */
SpectralClustering2::Stats SpectralClustering2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void SpectralClustering2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 相似度矩阵构建
// ──────────────────────────────────────────────

/**
 * @brief 基于高斯核函数构建相似度矩阵
 *
 * W[i][j] = exp(-||x_i - x_j||^2 / (2 * sigma^2))
 *
 * @param pts 输入数据点集合
 * @return n x n 相似度矩阵
 */
QVector<QVector<double>> SpectralClustering2::buildSimilarityMatrix(
    const QVector<QVector<double>>& pts)
{
    const int n = pts.size();
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));
    const double twoSigma2 = 2.0 * m_sigma * m_sigma;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double distSq = 0.0;
            const int dim = qMin(pts[i].size(), pts[j].size());
            for (int d = 0; d < dim; ++d) {
                double diff = pts[i][d] - pts[j][d];
                distSq += diff * diff;
            }
            double sim = qExp(-distSq / twoSigma2);
            W[i][j] = sim;
            W[j][i] = sim;
        }
        W[i][i] = 1.0; // 自相似度
    }

    return W;
}

// ──────────────────────────────────────────────
// 私有方法 — K-Means聚类
// ──────────────────────────────────────────────

/**
 * @brief 在低维嵌入空间上运行 K-Means 聚类
 *
 * 使用 K-Means++ 初始化策略，最多迭代 100 轮。
 *
 * @param emb 嵌入矩阵，每行为一个数据点的低维表示
 * @param k 聚类数目
 * @return 聚类标签数组
 */
QVector<int> SpectralClustering2::kMeansOnEmbedding(const QVector<QVector<double>>& emb, int k)
{
    const int n = emb.size();
    if (n == 0) return {};
    const int dim = emb[0].size();

    // K-Means++ 初始化中心
    QVector<QVector<double>> centers(k, QVector<double>(dim, 0.0));
    QVector<bool> chosen(n, false);

    // 选择第一个中心
    int first = n / 2;
    centers[0] = emb[first];
    chosen[first] = true;

    QVector<double> minDist(n, 1e18);

    for (int c = 1; c < k; ++c) {
        // 计算每个点到最近中心的距离
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = 0.0;
            for (int dd = 0; dd < dim; ++dd) {
                double diff = emb[i][dd] - centers[c - 1][dd];
                d += diff * diff;
            }
            minDist[i] = qMin(minDist[i], d);
            totalDist += minDist[i];
        }

        // 按概率选取下一个中心
        if (totalDist < 1e-12) {
            centers[c] = emb[c % n];
            continue;
        }

        double threshold = qSin(c * 3.7 + 1.1) * 0.5 + 0.5; // [0,1] 伪随机
        threshold = qBound(0.0, threshold, 1.0) * totalDist;
        double cumulative = 0.0;
        int selected = n - 1;
        for (int i = 0; i < n; ++i) {
            cumulative += minDist[i];
            if (cumulative >= threshold) {
                selected = i;
                break;
            }
        }
        centers[c] = emb[selected];
    }

    // 迭代 K-Means
    QVector<int> labels(n, 0);

    for (int iter = 0; iter < 100; ++iter) {
        bool changed = false;

        // 分配步骤：将每个点分配到最近的中心
        for (int i = 0; i < n; ++i) {
            double bestDist = 1e18;
            int bestLabel = 0;
            for (int c = 0; c < k; ++c) {
                double dist = 0.0;
                for (int dd = 0; dd < dim; ++dd) {
                    double diff = emb[i][dd] - centers[c][dd];
                    dist += diff * diff;
                }
                if (dist < bestDist) {
                    bestDist = dist;
                    bestLabel = c;
                }
            }
            if (labels[i] != bestLabel) {
                labels[i] = bestLabel;
                changed = true;
            }
        }

        if (!changed) break;

        // 更新步骤：重新计算聚类中心
        QVector<int> counts(k, 0);
        QVector<QVector<double>> newCenters(k, QVector<double>(dim, 0.0));
        for (int i = 0; i < n; ++i) {
            int lbl = labels[i];
            counts[lbl]++;
            for (int dd = 0; dd < dim; ++dd) {
                newCenters[lbl][dd] += emb[i][dd];
            }
        }
        for (int c = 0; c < k; ++c) {
            if (counts[c] > 0) {
                for (int dd = 0; dd < dim; ++dd) {
                    centers[c][dd] = newCenters[c][dd] / counts[c];
                }
            }
        }
    }

    return labels;
}
