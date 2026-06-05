/**
 * @file GraphSegmenter.cpp
 * @brief 图分割器实现 — Felzenszwalb-Huttenlocher算法
 */

#include "utils/segment2/GraphSegmenter.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数
 * @param sigma 高斯平滑标准差
 * @param threshold 合并阈值
 * @param minSize 最小区域尺寸
 * @param parent 父对象
 */
GraphSegmenter::GraphSegmenter(double sigma, double threshold,
                                 int minSize, QObject* parent)
    : QObject(parent)
    , m_sigma(qMax(0.0, sigma))
    , m_threshold(qMax(1.0, threshold))
    , m_minSize(qMax(1, minSize))
    , m_timeSum(0.0)
    , m_regionSum(0.0)
{
}

/**
 * @brief 执行图分割
 * @param data 输入数据矩阵
 * @return 标签矩阵
 *
 * 实现 Felzenszwalb-Huttenlocher 算法:
 * 将数据视为图，4邻域构建边，按边权排序后
 * 逐步合并满足阈值的区域。
 */
QVector<QVector<int>> GraphSegmenter::segment(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int rows = data.size();
    QVector<QVector<int>> labels(rows);
    if (rows == 0) {
        double elapsed = static_cast<double>(timer.elapsed());
        ++m_stats.totalSegments;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs =
            m_timeSum / static_cast<double>(m_stats.totalSegments);
        emit segmentCompleted(0);
        return labels;
    }

    int cols = data[0].size();
    int totalPixels = rows * cols;

    /* 步骤1: 高斯预平滑 */
    QVector<QVector<double>> smoothed(rows);
    for (int i = 0; i < rows; ++i) {
        smoothed[i] = gaussianSmooth(data[i]);
    }

    /* 步骤2: 构建4邻域边(右+下) */
    struct Edge {
        int    a, b;     ///< 两个端点(线性索引)
        double weight;   ///< 边权重(像素差绝对值)
    };

    QVector<Edge> edges;
    edges.reserve(totalPixels * 2);

    for (int r = 0; r < rows; ++r) {
        int rowCols = qMin(cols, static_cast<int>(smoothed[r].size()));
        for (int c = 0; c < rowCols; ++c) {
            int idx = r * cols + c;
            /* 右邻 */
            if (c + 1 < rowCols) {
                double w = qAbs(smoothed[r][c] - smoothed[r][c + 1]);
                edges.append({idx, r * cols + c + 1, w});
            }
            /* 下邻 */
            if (r + 1 < rows) {
                int nextRowCols = qMin(cols,
                    static_cast<int>(smoothed[r + 1].size()));
                if (c < nextRowCols) {
                    double w = qAbs(smoothed[r][c] - smoothed[r + 1][c]);
                    edges.append({idx, (r + 1) * cols + c, w});
                }
            }
        }
    }

    /* 步骤3: 按权重排序 */
    std::sort(edges.begin(), edges.end(),
              [](const Edge& e1, const Edge& e2) {
                  return e1.weight < e2.weight;
              });

    /* 步骤4: 并查集初始化 */
    QVector<int>    parent(totalPixels);
    QVector<int>    rank_(totalPixels, 0);
    QVector<double> maxInternal(totalPixels, 0.0);
    QVector<int>    regionSize(totalPixels, 1);

    for (int i = 0; i < totalPixels; ++i) parent[i] = i;

    /* 步骤5: 逐边判断合并 */
    for (const Edge& e : edges) {
        int rootA = findRoot(parent, e.a);
        int rootB = findRoot(parent, e.b);
        if (rootA == rootB) continue;

        /* 判断是否满足合并条件 */
        double diffBetween = e.weight;
        double threshA = maxInternal[rootA]
            + m_threshold / static_cast<double>(regionSize[rootA]);
        double threshB = maxInternal[rootB]
            + m_threshold / static_cast<double>(regionSize[rootB]);

        if (diffBetween <= qMin(threshA, threshB)) {
            /* 合并 */
            int newRoot = rootA;
            if (rank_[rootA] < rank_[rootB]) {
                newRoot = rootB;
            }
            int mergedChild = (newRoot == rootA) ? rootB : rootA;

            parent[mergedChild] = newRoot;
            if (rank_[rootA] == rank_[rootB] && newRoot == rootA) {
                ++rank_[newRoot];
            }

            /* 更新区域内部最大差异 */
            maxInternal[newRoot] = qMax(
                qMax(maxInternal[rootA], maxInternal[rootB]),
                diffBetween);
            regionSize[newRoot] =
                regionSize[rootA] + regionSize[rootB];
        }
    }

    /* 步骤6: 后处理 — 合并过小区域 */
    for (const Edge& e : edges) {
        int rootA = findRoot(parent, e.a);
        int rootB = findRoot(parent, e.b);
        if (rootA != rootB
            && (regionSize[rootA] < m_minSize
                || regionSize[rootB] < m_minSize)) {
            unionSet(parent, rank_, rootA, rootB);
            int newRoot = findRoot(parent, rootA);
            regionSize[newRoot] =
                regionSize[rootA] + regionSize[rootB];
        }
    }

    /* 步骤7: 提取标签 */
    QMap<int, int> rootToLabel;
    int nextLabel = 0;

    labels.resize(rows);
    for (int r = 0; r < rows; ++r) {
        int rowCols = qMin(cols, static_cast<int>(data[r].size()));
        labels[r].resize(rowCols);
        for (int c = 0; c < rowCols; ++c) {
            int root = findRoot(parent, r * cols + c);
            if (!rootToLabel.contains(root)) {
                rootToLabel[root] = nextLabel++;
            }
            labels[r][c] = rootToLabel[root];
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalSegments;
    m_timeSum += elapsed;
    m_regionSum += nextLabel;
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalSegments);
    m_stats.avgRegionCount =
        m_regionSum / static_cast<double>(m_stats.totalSegments);

    emit segmentCompleted(nextLabel);
    return labels;
}

/**
 * @brief 并查集查找根节点(路径压缩)
 * @param parent 父节点数组
 * @param x 待查找节点
 * @return 根节点索引
 */
int GraphSegmenter::findRoot(QVector<int>& parent, int x) const
{
    while (parent[x] != x) {
        parent[x] = parent[parent[x]]; /* 路径压缩 */
        x = parent[x];
    }
    return x;
}

/**
 * @brief 并查集合并
 * @param parent 父节点数组
 * @param rank 按秩数组
 * @param a 节点A @param b 节点B
 */
void GraphSegmenter::unionSet(QVector<int>& parent, QVector<int>& rank,
                                int a, int b) const
{
    int ra = findRoot(parent, a);
    int rb = findRoot(parent, b);
    if (ra == rb) return;

    if (rank[ra] < rank[rb]) {
        parent[ra] = rb;
    } else if (rank[ra] > rank[rb]) {
        parent[rb] = ra;
    } else {
        parent[rb] = ra;
        ++rank[ra];
    }
}

/**
 * @brief 高斯平滑一维数据
 * @param row 输入行数据
 * @return 平滑后的行数据
 *
 * 使用5点高斯核(sigma由m_sigma参数控制)进行卷积平滑，
 * 消除高频噪声对分割边界的干扰。
 */
QVector<double> GraphSegmenter::gaussianSmooth(
    const QVector<double>& row) const
{
    int n = row.size();
    if (n < 3 || m_sigma < 1e-9) return row;

    /* 构建5点高斯核 */
    double sigmaSq2 = 2.0 * m_sigma * m_sigma;
    double kernel[5];
    double kSum = 0.0;
    for (int k = -2; k <= 2; ++k) {
        kernel[k + 2] = qExp(-static_cast<double>(k * k) / sigmaSq2);
        kSum += kernel[k + 2];
    }
    for (double& v : kernel) v /= kSum;

    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double val = 0.0;
        for (int k = -2; k <= 2; ++k) {
            int idx = qBound(0, i + k, n - 1);
            val += row[idx] * kernel[k + 2];
        }
        result[i] = val;
    }
    return result;
}

/** @brief 设置高斯平滑参数 @param s 标准差 */
void GraphSegmenter::setSigma(double s)
{
    m_sigma = qMax(0.0, s);
}

/** @brief 设置合并阈值 @param t 阈值 */
void GraphSegmenter::setThreshold(double t)
{
    m_threshold = qMax(1.0, t);
}

/** @brief 设置最小区域尺寸 @param size 像素数 */
void GraphSegmenter::setMinRegionSize(int size)
{
    m_minSize = qMax(1, size);
}

/** @brief 重置统计 */
void GraphSegmenter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_regionSum = 0.0;
}
