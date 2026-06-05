/**
 * @file SpectralBicluster.cpp
 * @brief 谱双聚类引擎实现 — 二部图谱分解/SVD/棋盘结构检测
 */

#include "utils/cluster18/SpectralBicluster.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
SpectralBicluster::SpectralBicluster(QObject* parent)
    : QObject(parent)
    , m_numRowClusters(2)
    , m_numColClusters(2)
    , m_timeSum(0.0)
{
}

/** @brief 设置目标行簇数 @param n 行簇数 */
void SpectralBicluster::setNumRowClusters(int n)
{
    m_numRowClusters = qMax(2, n);
}

/** @brief 设置目标列簇数 @param n 列簇数 */
void SpectralBicluster::setNumColClusters(int n)
{
    m_numColClusters = qMax(2, n);
}

/** @brief 对矩阵执行谱双聚类 @param matrix 行优先数据矩阵 @param rows 行数 @param cols 列数 @return 聚类结果 */
SpectralBicluster::BiclusterResult SpectralBicluster::fit(
    const QVector<double>& matrix, int rows, int cols)
{
    QElapsedTimer timer;
    timer.start();

    BiclusterResult result;
    if (rows < 2 || cols < 2 || matrix.size() < rows * cols) {
        return result;
    }

    /* 第一步: 复制并归一化数据矩阵 */
    QVector<double> normed = matrix;

    /* 行归一化: 使每行L2范数为1 */
    normalizeRows(normed, rows, cols);

    /* 列归一化: 使每列L2范数为1 */
    normalizeCols(normed, rows, cols);

    /* 第二步: 截断SVD分解 — 提取前k个奇异向量 */
    int k = qMin(m_numRowClusters, m_numColClusters);
    k = qMin(k, qMin(rows, cols) - 1);
    k = qMax(1, k);

    QVector<double> u, s, vt;
    svdTruncated(normed, rows, cols, u, s, vt, k);

    /* 第三步: 从左奇异向量提取行嵌入 */
    int rowDim = k;
    m_rowEmbed.resize(rows * rowDim);
    for (int i = 0; i < rows; ++i) {
        double norm = 0.0;
        for (int j = 0; j < rowDim; ++j) {
            double v = u[i * k + j];
            m_rowEmbed[i * rowDim + j] = v;
            norm += v * v;
        }
        /* L2归一化 */
        norm = qSqrt(norm);
        if (norm > 1e-10) {
            for (int j = 0; j < rowDim; ++j) {
                m_rowEmbed[i * rowDim + j] /= norm;
            }
        }
    }

    /* 第四步: 从右奇异向量提取列嵌入 */
    m_colEmbed.resize(cols * rowDim);
    for (int i = 0; i < cols; ++i) {
        double norm = 0.0;
        for (int j = 0; j < rowDim; ++j) {
            double v = vt[j * cols + i];
            m_colEmbed[i * rowDim + j] = v;
            norm += v * v;
        }
        norm = qSqrt(norm);
        if (norm > 1e-10) {
            for (int j = 0; j < rowDim; ++j) {
                m_colEmbed[i * rowDim + j] /= norm;
            }
        }
    }

    /* 第五步: 对行嵌入和列嵌入分别做K-means聚类 */
    result.rowIndices = kMeansClustering(m_rowEmbed, rows, rowDim, m_numRowClusters);
    result.colIndices = kMeansClustering(m_colEmbed, cols, rowDim, m_numColClusters);
    result.numRowClusters = m_numRowClusters;
    result.numColClusters = m_numColClusters;

    /* 第六步: 计算棋盘结构得分 */
    result.score = checkerboardScore(matrix, result, rows, cols);

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalBiclusters;
    ++m_stats.totalMatricesProcessed;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalBiclusters);
    if (result.score > m_stats.bestScore) {
        m_stats.bestScore = result.score;
    }

    emit biclusterComplete(result.numRowClusters, result.numColClusters, result.score);
    return result;
}

/** @brief 计算棋盘结构得分 @param matrix 数据矩阵 @param result 聚类结果 @param rows 行数 @param cols 列数 @return 得分 */
double SpectralBicluster::checkerboardScore(
    const QVector<double>& matrix,
    const BiclusterResult& result,
    int rows, int cols) const
{
    if (rows == 0 || cols == 0) return 0.0;

    /* 计算每个(rowCluster, colCluster)块的平均值 */
    QVector<QVector<double>> blockSum(m_numRowClusters,
                                       QVector<double>(m_numColClusters, 0.0));
    QVector<QVector<int>> blockCount(m_numRowClusters,
                                      QVector<int>(m_numColClusters, 0));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int ri = qBound(0, result.rowIndices[i], m_numRowClusters - 1);
            int ci = qBound(0, result.colIndices[j], m_numColClusters - 1);
            blockSum[ri][ci] += matrix[i * cols + j];
            ++blockCount[ri][ci];
        }
    }

    /* 块均值矩阵 */
    QVector<double> blockMean;
    for (int i = 0; i < m_numRowClusters; ++i) {
        for (int j = 0; j < m_numColClusters; ++j) {
            double avg = (blockCount[i][j] > 0)
                         ? blockSum[i][j] / blockCount[i][j] : 0.0;
            blockMean.append(avg);
        }
    }

    /* 计算块间方差相对于总方差的比例作为得分 */
    double globalMean = 0.0;
    int totalBlocks = m_numRowClusters * m_numColClusters;
    for (double v : blockMean) globalMean += v;
    globalMean /= totalBlocks;

    double betweenVar = 0.0;
    for (double v : blockMean) {
        double diff = v - globalMean;
        betweenVar += diff * diff;
    }
    betweenVar /= totalBlocks;

    /* 总方差 */
    double totalVar = 0.0;
    int n = rows * cols;
    for (int i = 0; i < n; ++i) {
        double diff = matrix[i] - globalMean;
        totalVar += diff * diff;
    }
    totalVar /= n;

    return (totalVar > 1e-10) ? betweenVar / totalVar : 0.0;
}

/** @brief 获取行谱嵌入 @return 行嵌入向量 */
QVector<double> SpectralBicluster::rowEmbedding() const
{
    return m_rowEmbed;
}

/** @brief 获取列谱嵌入 @return 列嵌入向量 */
QVector<double> SpectralBicluster::colEmbedding() const
{
    return m_colEmbed;
}

/** @brief 重置统计 */
void SpectralBicluster::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 行归一化(每行L2范数=1) @param matrix 矩阵 @param rows 行数 @param cols 列数 */
void SpectralBicluster::normalizeRows(QVector<double>& matrix,
                                       int rows, int cols) const
{
    for (int i = 0; i < rows; ++i) {
        double norm = 0.0;
        for (int j = 0; j < cols; ++j) {
            double v = matrix[i * cols + j];
            norm += v * v;
        }
        norm = qSqrt(norm);
        if (norm > 1e-10) {
            for (int j = 0; j < cols; ++j) {
                matrix[i * cols + j] /= norm;
            }
        }
    }
}

/** @brief 列归一化(每列L2范数=1) @param matrix 矩阵 @param rows 行数 @param cols 列数 */
void SpectralBicluster::normalizeCols(QVector<double>& matrix,
                                       int rows, int cols) const
{
    for (int j = 0; j < cols; ++j) {
        double norm = 0.0;
        for (int i = 0; i < rows; ++i) {
            double v = matrix[i * cols + j];
            norm += v * v;
        }
        norm = qSqrt(norm);
        if (norm > 1e-10) {
            for (int i = 0; i < rows; ++i) {
                matrix[i * cols + j] /= norm;
            }
        }
    }
}

/**
 * @brief 截断SVD(幂迭代法) @param matrix 矩阵 @param rows 行数 @param cols 列数
 * @param u 左奇异向量输出 @param s 奇异值输出 @param vt 右奇异向量输出 @param k 截断秩
 */
void SpectralBicluster::svdTruncated(const QVector<double>& matrix,
                                      int rows, int cols,
                                      QVector<double>& u,
                                      QVector<double>& s,
                                      QVector<double>& vt, int k) const
{
    u.resize(rows * k);
    s.resize(k);
    vt.resize(k * cols);

    QVector<double> residual = matrix;

    for (int t = 0; t < k; ++t) {
        /* 初始化右奇异向量为随机值 */
        static std::mt19937 rng(42);
        std::uniform_real_distribution<double> dist(-1.0, 1.0);

        QVector<double> v(cols);
        for (int j = 0; j < cols; ++j) v[j] = dist(rng);

        /* 幂迭代: v = A^T * A * v / norm */
        for (int iter = 0; iter < 100; ++iter) {
            /* u_tmp = A * v */
            QVector<double> uTmp(rows, 0.0);
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    uTmp[i] += residual[i * cols + j] * v[j];
                }
            }

            /* v = A^T * u_tmp */
            QVector<double> vNew(cols, 0.0);
            for (int j = 0; j < cols; ++j) {
                for (int i = 0; i < rows; ++i) {
                    vNew[j] += residual[i * cols + j] * uTmp[i];
                }
            }

            /* 归一化 */
            double norm = 0.0;
            for (double x : vNew) norm += x * x;
            norm = qSqrt(norm);
            if (norm < 1e-14) break;
            for (int j = 0; j < cols; ++j) v[j] = vNew[j] / norm;
        }

        /* 计算 u = A * v */
        QVector<double> uFinal(rows, 0.0);
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                uFinal[i] += residual[i * cols + j] * v[j];
            }
        }

        /* 奇异值 */
        double sigma = 0.0;
        for (double x : uFinal) sigma += x * x;
        sigma = qSqrt(sigma);
        if (sigma > 1e-14) {
            for (int i = 0; i < rows; ++i) uFinal[i] /= sigma;
        }

        /* 存储结果 */
        for (int i = 0; i < rows; ++i) u[i * k + t] = uFinal[i];
        s[t] = sigma;
        for (int j = 0; j < cols; ++j) vt[t * cols + j] = v[j];

        /* 减去当前秩1分量 */
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                residual[i * cols + j] -= sigma * uFinal[i] * v[j];
            }
        }
    }
}

/**
 * @brief K-means聚类 @param data 数据(n个点，dim维) @param n 点数 @param dim 维度 @param k 簇数 @return 每个点的簇标签
 */
QVector<int> SpectralBicluster::kMeansClustering(const QVector<double>& data,
                                                   int n, int dim, int k) const
{
    if (n <= k) {
        QVector<int> labels(n);
        for (int i = 0; i < n; ++i) labels[i] = qMin(i, k - 1);
        return labels;
    }

    static std::mt19937 rng(12345);
    std::uniform_int_distribution<int> distIdx(0, n - 1);

    /* K-means++初始化质心 */
    QVector<QVector<double>> centroids(k, QVector<double>(dim, 0.0));
    int firstIdx = distIdx(rng);
    for (int d = 0; d < dim; ++d) {
        centroids[0][d] = data[firstIdx * dim + d];
    }

    for (int c = 1; c < k; ++c) {
        /* 计算每个点到最近质心的距离 */
        QVector<double> minDist(n, 1e300);
        for (int i = 0; i < n; ++i) {
            for (int pc = 0; pc < c; ++pc) {
                double dist = squaredEuclidean(&data[i * dim],
                                                centroids[pc].constData(), dim);
                if (dist < minDist[i]) minDist[i] = dist;
            }
        }
        /* 按距离加权随机选择下一个质心 */
        double totalDist = 0.0;
        for (double d : minDist) totalDist += d;
        double threshold = distIdx(rng) * totalDist / n;
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        for (int d = 0; d < dim; ++d) {
            centroids[c][d] = data[chosen * dim + d];
        }
    }

    /* 迭代优化 */
    QVector<int> labels(n, 0);
    for (int iter = 0; iter < 50; ++iter) {
        bool changed = false;

        /* 分配步骤: 每个点归到最近质心 */
        for (int i = 0; i < n; ++i) {
            double bestDist = 1e300;
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double d = squaredEuclidean(&data[i * dim],
                                             centroids[c].constData(), dim);
                if (d < bestDist) {
                    bestDist = d;
                    bestC = c;
                }
            }
            if (labels[i] != bestC) {
                labels[i] = bestC;
                changed = true;
            }
        }

        if (!changed) break;

        /* 更新步骤: 重新计算质心 */
        QVector<int> counts(k, 0);
        for (auto& c : centroids) c.fill(0.0);
        for (int i = 0; i < n; ++i) {
            int c = labels[i];
            ++counts[c];
            for (int d = 0; d < dim; ++d) {
                centroids[c][d] += data[i * dim + d];
            }
        }
        for (int c = 0; c < k; ++c) {
            if (counts[c] > 0) {
                for (int d = 0; d < dim; ++d) {
                    centroids[c][d] /= counts[c];
                }
            }
        }
    }

    return labels;
}

/** @brief 欧氏距离平方 @param a 向量a @param b 向量b @param dim 维度 @return 距离平方 */
double SpectralBicluster::squaredEuclidean(const double* a,
                                            const double* b, int dim) const
{
    double d = 0.0;
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return d;
}
