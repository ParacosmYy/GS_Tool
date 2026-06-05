/**
 * @file PolarCode.h
 * @brief Polar码编码器/解码器,支持SC(逐次消除)译码
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Polar码编码器与SC解码器
 *
 * 基于信道极化理论实现Polar码的编码和逐次消除(SC)译码。
 * 支持任意码长(N=2^n)和码率、BEC/BSC/AWGN信道模型、
 * 可靠度序列的Bhattacharyya参数排序。
 */
class PolarCode : public QObject
{
    Q_OBJECT

public:
    /** @brief 信道类型 */
    enum ChannelType {
        BEC = 0,    ///< 二进制擦除信道
        BSC = 1,    ///< 二进制对称信道
        AWGN = 2    ///< 加性高斯白噪声信道
    };
    Q_ENUM(ChannelType)

    /** @brief 统计信息 */
    struct Stats {
        int totalEncodes = 0;           ///< 总编码次数
        int totalDecodes = 0;           ///< 总解码次数
        double avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    explicit PolarCode(QObject* parent = nullptr);

    /**
     * @brief 初始化Polar码参数
     * @param n Log2码长(实际码长 N = 2^n)
     * @param k 信息比特数
     * @param channelType 信道类型
     * @param channelParam 信道参数(BEC擦除率/BSC错误率/AWGN信噪比)
     * @return 是否初始化成功
     */
    bool initialize(int n, int k,
                    ChannelType channelType = AWGN,
                    double channelParam = 0.5);

    /**
     * @brief Polar码编码
     * @param infoBits 信息比特(长度必须为k)
     * @return 编码后码字(长度N)
     */
    QVector<int> encode(const QVector<int>& infoBits);

    /**
     * @brief SC(逐次消除)译码
     * @param received 接收序列(0/1硬判决 或 LLR软信息)
     * @param isLLR 接收序列是否为LLR形式
     * @return 译码后的信息比特(长度k)
     */
    QVector<int> decodeSC(const QVector<double>& received, bool isLLR = false);

    /**
     * @brief 计算Bhattacharyya参数(信道可靠度)
     * @param n 码长幂次(N=2^n)
     * @param channelType 信道类型
     * @param channelParam 信道参数
     * @return 各比特位置的Bhattacharyya参数(越小越可靠)
     */
    QVector<double> bhattacharyyaParams(int n,
                                        ChannelType channelType,
                                        double channelParam) const;

    /**
     * @brief 获取当前码参数
     * @return {N, k}
     */
    QPair<int, int> codeParameters() const;

    /**
     * @brief 获取信息比特位置索引
     * @return 信息比特在码字中的位置
     */
    QVector<int> infoBitIndices() const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 编码完成信号 */
    void encodeCompleted(int codeLength);

    /** @brief 译码完成信号 */
    void decodeCompleted(int infoLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    int m_n = 0;                ///< log2(N)
    int m_N = 0;                ///< 码长
    int m_k = 0;                ///< 信息比特数
    ChannelType m_channelType;  ///< 信道类型
    double m_channelParam;      ///< 信道参数
    QVector<int> m_frozenSet;   ///< 冻结比特位置
    QVector<int> m_infoSet;     ///< 信息比特位置
    QVector<int> m_generator;   ///< 生成多项式矩阵(扁平化)

    void computeReliabilitySequence();
    void buildGeneratorMatrix();
    QVector<double> computeLLR_BEC(const QVector<double>& received) const;
    QVector<double> computeLLR_AWGN(const QVector<double>& received) const;
    double bhattacharyyaBEC(double erasureProb, int level) const;
    double bhattacharyyaAWGN(double snr, int level) const;
};
