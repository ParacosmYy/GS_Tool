#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 极化码编解码器
 *
 * 实现Polar码的编码和SC/SCL译码，基于信道极化理论，
 * 是5G通信标准中的控制信道编码方案。
 */
class PolarCode11 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalEncoded = 0;        ///< 已编码码字数
        int totalDecoded = 0;        ///< 已解码码字数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit PolarCode11(QObject* parent = nullptr);

    /** @brief 设置码长(2的幂) */
    void setCodeLength(int length);
    /** @brief 设置信息比特数 */
    void setInfoBits(int k);
    /** @brief 极化码编码 */
    QVector<int> encode(const QVector<int>& infoBits);
    /** @brief SC或SCL译码 */
    QVector<int> decode(const QVector<double>& llr);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编解码完成，返回帧大小 */
    void codingCompleted(int frameSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_codeLength = 256;
    int m_infoBits = 128;
};
