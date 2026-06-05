/**
 * @file LdpcDecoder2.h
 * @brief LDPC解码器 — 置信传播(和积)算法实现
 *
 * 功能: 基于Tanner图的置信传播(Belief Propagation)解码器，支持
 *       和积(Sum-Product)与最小和(Min-Sum)两种变体。输入LLR软信息，
 *       迭代纠错输出硬判决比特。
 *
 * 协作: SerialFrameDecoder(帧解码) / CRC(校验)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief LDPC解码器 — 置信传播算法
 */
class LdpcDecoder2 : public QObject {
    Q_OBJECT

public:
    /** @brief 解码算法变体 */
    enum class Algorithm {
        SumProduct,         ///< 和积算法(精确)
        MinSum,             ///< 最小和算法(近似，更快)
        OffsetMinSum        ///< 偏移最小和(修正近似误差)
    };
    Q_ENUM(Algorithm)

    /** @brief 解码结果 */
    struct DecodeResult {
        QVector<int> decodedBits;           ///< 解码后的比特(0/1)
        bool converged = false;             ///< 是否收敛
        int iterations = 0;                 ///< 实际迭代次数
        double finalSyndromeWeight = 0.0;   ///< 最终校验子权重
        double estimatedBER = 0.0;          ///< 估计误比特率
    };

    /** @brief 运行统计 */
    struct Stats {
        int totalDecoded = 0;               ///< 累计解码帧数
        int totalConverged = 0;             ///< 累计收敛帧数
        int totalIterations = 0;            ///< 累计迭代次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
        double avgIterations = 0.0;         ///< 平均迭代次数
    };

    explicit LdpcDecoder2(QObject* parent = nullptr);

    /** @brief 从稀疏矩阵设置校验矩阵H @param rowColPairs 非零位置(row,col)列表 @param rows 行数m @param cols 列数n */
    void setParityMatrix(const QList<QPair<int, int>>& rowColPairs, int rows, int cols);

    /** @brief 设置解码算法 @param algo 算法类型 */
    void setAlgorithm(Algorithm algo);

    /** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
    void setMaxIterations(int maxIter);

    /** @brief 设置偏移最小和的缩放因子 @param factor 缩放因子(通常0.75) */
    void setMinSumScaleFactor(double factor);

    /** @brief 解码LLR软信息 @param llr 输入LLR(对数似然比) @return 解码结果 */
    DecodeResult decode(const QVector<double>& llr);

    /** @brief 批量解码 @param llrFrames LLR帧列表 @return 解码结果列表 */
    QList<DecodeResult> decodeBatch(const QList<QVector<double>>& llrFrames);

    /** @brief 编码(简单: 系统码，追加校验位) @param infoBits 信息比特 @return 码字比特 */
    QVector<int> encode(const QVector<int>& infoBits) const;

    /** @brief 校验码字是否满足校验方程 @param codeword 码字 @return 是否通过校验 */
    bool checkSyndrome(const QVector<int>& codeword) const;

    /** @brief 获取码长 @return 码长n */
    int codeLength() const;

    /** @brief 获取信息位长度 @return 信息位长度k */
    int infoLength() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 单帧解码完成 @param frameIndex 帧索引 @param converged 是否收敛 @param iterations 迭代次数 */
    void frameDecoded(int frameIndex, bool converged, int iterations);

    /** @brief 批量解码进度 @param completed 已完成 @param total 总数 */
    void batchProgress(int completed, int total);

private:
    /** @brief 初始化Tanner图消息 */
    void initializeMessages();

    /** @brief 校验节点更新(和积) @param iteration 当前迭代 */
    void checkNodeUpdateSumProduct(int iteration);

    /** @brief 校验节点更新(最小和) @param iteration 当前迭代 */
    void checkNodeUpdateMinSum();

    /** @brief 变量节点更新 */
    void variableNodeUpdate();

    /** @brief 硬判决 @param llr LLR值 @return 比特 */
    int hardDecision(double llr) const;

    /** @brief 计算校验子 @param bits 比特向量 @return 校验子 */
    QVector<int> computeSyndrome(const QVector<int>& bits) const;

    /** @brief phi函数: ln(tanh(|x|/2)) @param x 输入 @return 函数值 */
    double phiFunction(double x) const;

    int m_m = 0;                            ///< 校验行数
    int m_n = 0;                            ///< 码字列数
    int m_k = 0;                            ///< 信息位长度(n-m)
    int m_maxIterations = 50;               ///< 最大迭代次数
    Algorithm m_algo = Algorithm::SumProduct;///< 解码算法
    double m_minSumFactor = 0.75;           ///< 最小和缩放因子

    /** @brief Tanner图边: 每个校验节点连接的变量节点 */
    QVector<QList<int>> m_checkEdges;
    /** @brief Tanner图边: 每个变量节点连接的校验节点 */
    QVector<QList<int>> m_varEdges;

    /** @brief 消息: R(校验->变量) */
    QVector<QVector<double>> m_R;
    /** @brief 消息: Q(变量->校验) */
    QVector<QVector<double>> m_Q;

    Stats m_stats;                          ///< 运行统计
    double m_timeSum = 0.0;                 ///< 处理时间累加器
};
