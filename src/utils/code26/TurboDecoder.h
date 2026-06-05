/**
 * @file TurboDecoder.h
 * @brief Turbo码解码器 — 迭代SISO/MAP算法/交织/外信息交换
 *
 * 功能: 实现Turbo码的迭代解码，支持MAP/Log-MAP/Max-Log-MAP算法、
 *       可配置交织器、外信息交换、早期终止判定。
 *
 * 协作: DataCompressor(数据传输) / FftEngine(频域分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Turbo码解码器 — 迭代SISO解码
 */
class TurboDecoder : public QObject {
    Q_OBJECT

public:
    /** @brief 解码算法 */
    enum class Algorithm {
        MAP,         ///< 标准MAP(Bahl算法)
        LogMAP,      ///< 对数域MAP
        MaxLogMAP    ///< Max-Log-MAP(近似,速度最快)
    };
    Q_ENUM(Algorithm)

    /** @brief 解码结果 */
    struct DecodeResult {
        QVector<int> bits;           ///< 解码比特(0/1)
        double llrSum = 0.0;         ///< 似然比总和
        int iterationsUsed = 0;      ///< 实际迭代次数
        bool converged = false;      ///< 是否收敛
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalDecodings = 0;          ///< 累计解码次数
        quint64 totalBitsDecoded = 0;        ///< 累计解码比特数
        double  avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
        double  avgIterations = 0.0;         ///< 平均迭代次数
        quint64 totalConverged = 0;          ///< 收敛次数
    };

    explicit TurboDecoder(QObject* parent = nullptr);

    /** @brief 设置最大迭代次数 @param maxIter 最大迭代(默认8) */
    void setMaxIterations(int maxIter);

    /** @brief 设置解码算法 @param algo 算法 */
    void setAlgorithm(Algorithm algo);

    /** @brief 设置交织器 @param interleaver 交织图案(索引表) */
    void setInterleaver(const QVector<int>& interleaver);

    /** @brief Turbo解码 @param systematic 系统位LLR @param parity1 第一分量校验LLR @param parity2 第二分量校验LLR @return 解码结果 */
    DecodeResult decode(const QVector<double>& systematic,
                        const QVector<double>& parity1,
                        const QVector<double>& parity2);

    /** @brief 生成交织图案(伪随机) @param length 长度 @return 交织索引 */
    static QVector<int> generateInterleaver(int length);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 解码完成 @param bits 比特数 @param iterations 迭代次数 @param converged 是否收敛 */
    void decodeComplete(int bits, int iterations, bool converged);

private:
    void sisoDecode(const QVector<double>& sys, const QVector<double>& parity,
                    const QVector<double>& prior,
                    QVector<double>& extrinsic) const;
    void logMapSiso(const QVector<double>& sys, const QVector<double>& parity,
                    const QVector<double>& prior,
                    QVector<double>& extrinsic) const;
    void maxLogMapSiso(const QVector<double>& sys,
                       const QVector<double>& parity,
                       const QVector<double>& prior,
                       QVector<double>& extrinsic) const;
    double logSum(double a, double b) const;
    QVector<int> interleave(const QVector<int>& data) const;
    QVector<double> interleaveLLR(const QVector<double>& llr) const;
    QVector<double> deinterleaveLLR(const QVector<double>& llr) const;

    int m_maxIterations = 8;          ///< 最大迭代次数
    Algorithm m_algorithm = Algorithm::MaxLogMAP; ///< 解码算法
    QVector<int> m_interleaver;       ///< 交织图案

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_iterSum = 0.0;
};
