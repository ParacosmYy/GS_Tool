/**
 * @file PolarCode2.h
 * @brief 极化码编解码器 — SC/SCL译码
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 极化码(Polar Code)编解码器
 * 支持SC(连续消除)和SCL(列表)译码
 */
class PolarCode2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 译码算法 */
    enum DecodeMethod {
        SC,        ///< 连续消除译码
        SCL        ///< 连续消除列表译码
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalEncoded = 0;           ///< 累计编码次数
        int totalDecoded = 0;           ///< 累计译码次数
        int totalBitErrors = 0;         ///< 累计比特错误数
        double avgProcessingTimeMs = 0.0;
    };

    /**
     * @brief 构造函数
     * @param n 码长(必须是2的幂)
     * @param k 信息位数
     * @param listSize SCL列表大小
     * @param parent 父对象
     */
    explicit PolarCode2(int n = 256, int k = 128,
                        int listSize = 8, QObject* parent = nullptr);

    /** @brief 编码 @param infoBits 信息位 @return 编码后码字 */
    QVector<int> encode(const QVector<int>& infoBits);

    /** @brief 译码 @param llr 接收对数似然比 @param method 译码算法 @return 译码信息位 */
    QVector<int> decode(const QVector<double>& llr, DecodeMethod method = SCL);

    /** @brief 获取信息位索引集合 */
    const QVector<int>& infoIndices() const { return m_infoIndices; }

    /** @brief 获取冻结位索引集合 */
    const QVector<int>& frozenIndices() const { return m_frozenIndices; }

    /** @brief 获取码长 */
    int codeLength() const { return m_n; }

    /** @brief 获取信息位长度 */
    int infoLength() const { return m_k; }

    /** @brief 获取码率 */
    double codeRate() const;

    /** @brief Bhattacharyya参数信道极化分析 */
    QVector<double> channelPolarization(double initialError) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param codeLen 码长 @param infoLen 信息位长 */
    void encodingCompleted(int codeLen, int infoLen);
    /** @brief 译码完成 @param errors 比特错误数 */
    void decodingCompleted(int errors);

private:
    /** @brief SCL路径 */
    struct Path {
        QVector<int> bits;          ///< 当前比特估计
        QVector<double> llr;        ///< 当前LLR值
        double metric = 0.0;        ///< 路径度量
        bool active = true;         ///< 是否活跃
    };

    /** @brief 信道可靠度排序(简化Bhattacharyya) */
    void computeReliability();

    /** @brief 选择信息位和冻结位 */
    void selectIndices();

    /** @brief SC递归译码 */
    void scDecode(const QVector<double>& llr, QVector<int>& decoded);

    /** @brief SCL列表译码 */
    QVector<int> sclDecode(const QVector<double>& llr);

    /** @brief 计算路径度量 */
    double pathMetric(const Path& path) const;

    /** @brief 更新LLR(递归蝶形网络) */
    void updateLLR(QVector<double>& llr, int stage, int bitVal) const;

    int m_n;                           ///< 码长
    int m_k;                           ///< 信息位数
    int m_listSize;                    ///< SCL列表大小
    int m_stages;                      ///< log2(n)
    QVector<int> m_infoIndices;        ///< 信息位索引
    QVector<int> m_frozenIndices;      ///< 冻结位索引
    QVector<double> m_reliability;     ///< 信道可靠度

    Stats m_stats;
    double m_timeSum = 0.0;
};
