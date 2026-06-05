#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Polar极化码实现 (12位参数配置)
 *
 * 提供Polar码的编码与连续消除列表(SCL)译码，支持信道极化和信息位选择。
 */
class PolarCode12 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalEncodingRuns = 0;      ///< 总编码次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int totalDecodingSuccess = 0;   ///< 总成功译码次数
    };

    explicit PolarCode12(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief Polar码编码
     * @param informationBits 信息比特序列
     * @return 编码后的码字
     */
    QVector<int> encode(const QVector<int>& informationBits);

    /**
     * @brief SCL连续消除列表译码
     * @param llrValues 对数似然比值序列
     * @param listSize 列表大小
     * @return 译码后的信息比特
     */
    QVector<int> decodeSCL(const QVector<double>& llrValues, int listSize = 8);

    /**
     * @brief 设置码参数
     * @param blockLength 码块长度 (2的幂)
     * @param infoLength 信息位长度
     */
    void setCodeParameters(int blockLength, int infoLength);

    /**
     * @brief 设置信息位位置（可靠性序列）
     * @param frozenBits 冻结位索引集合
     */
    void setFrozenBits(const QVector<int>& frozenBits);

    /**
     * @brief 获取当前码率
     * @return 码率 R = k/n
     */
    double codeRate() const;

signals:
    /// 编码完成信号
    void encodingCompleted(int blockLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_blockLength = 0;  ///< 码块长度n
    int m_infoLength = 0;   ///< 信息位长度k
    QVector<int> m_frozenBits; ///< 冻结位索引
};
