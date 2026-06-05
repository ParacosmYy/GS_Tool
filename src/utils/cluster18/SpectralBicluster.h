/**
 * @file SpectralBicluster.h
 * @brief 谱双聚类引擎 — 二部图谱分解/SVD/棋盘结构检测
 *
 * 功能: 对数据矩阵执行谱双聚类(Spectral Biclustering)，
 *       基于二部图的拉普拉斯矩阵谱分解和SVD，
 *       检测数据中的棋盘(checkerboard)结构。
 *
 * 协作: SpectrumAnalyzer(频域分析) / DataClassifier(分类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 谱双聚类引擎 — 二部图谱分解与棋盘结构检测
 */
class SpectralBicluster : public QObject {
    Q_OBJECT

public:
    /** @brief 双聚类结果 */
    struct BiclusterResult {
        QVector<int> rowIndices;        ///< 行簇索引(每行归属)
        QVector<int> colIndices;        ///< 列簇索引(每列归属)
        int numRowClusters = 0;         ///< 行簇数量
        int numColClusters = 0;         ///< 列簇数量
        double score = 0.0;            ///< 聚类质量评分
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalBiclusters = 0;           ///< 累计聚类次数
        quint64 totalMatricesProcessed = 0;    ///< 累计处理矩阵数
        double  avgProcessingTimeMs = 0.0;     ///< 平均处理时间(ms)
        double  bestScore = 0.0;              ///< 最佳聚类评分
    };

    explicit SpectralBicluster(QObject* parent = nullptr);

    /** @brief 设置目标行簇数 @param n 行簇数 */
    void setNumRowClusters(int n);

    /** @brief 设置目标列簇数 @param n 列簇数 */
    void setNumColClusters(int n);

    /** @brief 对矩阵执行谱双聚类 @param matrix 行优先数据矩阵 @param rows 行数 @param cols 列数 @return 聚类结果 */
    BiclusterResult fit(const QVector<double>& matrix, int rows, int cols);

    /** @brief 计算棋盘结构得分 @param matrix 数据矩阵 @param result 聚类结果 @param rows 行数 @param cols 列数 @return 得分 */
    double checkerboardScore(const QVector<double>& matrix,
                             const BiclusterResult& result,
                             int rows, int cols) const;

    /** @brief 获取归一化后的行谱嵌入 @return 行嵌入向量 */
    QVector<double> rowEmbedding() const;

    /** @brief 获取归一化后的列谱嵌入 @return 列嵌入向量 */
    QVector<double> colEmbedding() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param rowClusters 行簇数 @param colClusters 列簇数 @param score 质量评分 */
    void biclusterComplete(int rowClusters, int colClusters, double score);

private:
    void normalizeRows(QVector<double>& matrix, int rows, int cols) const;
    void normalizeCols(QVector<double>& matrix, int rows, int cols) const;
    void svdTruncated(const QVector<double>& matrix, int rows, int cols,
                      QVector<double>& u, QVector<double>& s,
                      QVector<double>& vt, int k) const;
    QVector<int> kMeansClustering(const QVector<double>& data,
                                  int n, int dim, int k) const;
    double squaredEuclidean(const double* a, const double* b, int dim) const;

    int m_numRowClusters;           ///< 目标行簇数
    int m_numColClusters;           ///< 目标列簇数
    QVector<double> m_rowEmbed;     ///< 行谱嵌入缓存
    QVector<double> m_colEmbed;     ///< 列谱嵌入缓存

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};
