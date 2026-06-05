/**
 * @file PolarDecoder.h
 * @brief Polar码解码器 — SC/SCL译码
 *
 * 功能: 支持连续消除(SC)和连续消除列表(SCL)译码算法，
 *       路径度量排序与剪枝、冻结集管理、CRC辅助SCL，
 *       统计译码次数/平均路径数/平均处理耗时。
 */
#ifndef POLARDECODER_H
#define POLARDECODER_H

#include <QObject>
#include <QVector>
#include <QList>

/**
 * @class PolarDecoder
 * @brief Polar码解码器，支持SC和SCL译码
 */
class PolarDecoder : public QObject {
    Q_OBJECT
public:
    /** 译码算法 */
    enum class Algorithm {
        SC,     ///< 连续消除
        SCL     ///< 连续消除列表
    };

    /** 译码结果 */
    struct DecodeResult {
        QVector<int> decodedBits;       ///< 译码输出比特
        double pathMetric;              ///< 路径度量值
        bool crcPassed;                 ///< CRC校验结果
    };

    /** 统计信息 */
    struct Stats {
        quint64 totalDecodes = 0;           ///< 总译码次数
        quint64 totalPathExtensions = 0;    ///< 累计路径扩展次数
        double  avgActivePaths = 0.0;       ///< 平均活跃路径数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit PolarDecoder(QObject* parent = nullptr);

    /** 设置参数 */
    void setCodeLength(int n);
    void setInfoLength(int k);
    void setListSize(int l);
    void setAlgorithm(Algorithm algo);
    void setCrcPolynomial(quint32 poly);

    /** 执行译码 */
    DecodeResult decode(const QVector<double>& llrInput);

    /** 设置冻结集(位索引列表) */
    void setFrozenSet(const QVector<int>& frozenIndices);

    /** 自动根据可靠度排序生成冻结集 */
    QVector<int> generateFrozenSet() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** 译码完成信号 */
    void decodeComplete(bool crcPassed, double pathMetric);
    /** 路径扩展信号 */
    void pathExtended(int activeCount, int listSize);

private:
    /** SC译码递归核心 */
    QVector<int> scDecode(const QVector<double>& llr);
    /** SC递归计算 */
    void scRecursive(const QVector<double>& llr, QVector<int>& bits,
                     int offset, int len);

    /** SCL译码核心 */
    DecodeResult sclDecode(const QVector<double>& llr);
    /** SCL路径扩展 */
    void extendPaths(QList<QVector<int>>& paths, QList<double>& metrics,
                     const QVector<double>& llr, int bitIndex);
    /** 路径剪枝，保留L条最优路径 */
    void prunePaths(QList<QVector<int>>& paths, QList<double>& metrics);

    /** CRC校验 */
    bool checkCrc(const QVector<int>& infoBits) const;

    /** 计算位可靠度(巴塔查里亚参数近似) */
    QVector<double> computeReliability() const;

    int m_codeLength;
    int m_infoLength;
    int m_listSize;
    Algorithm m_algorithm;
    quint32 m_crcPoly;
    QVector<int> m_frozenSet;
    Stats m_stats;
    double m_timeSum;
};

#endif // POLARDECODER_H
