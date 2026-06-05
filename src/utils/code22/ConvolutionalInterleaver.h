/**
 * @file ConvolutionalInterleaver.h
 * @brief 卷积交织器 — FIFO移位寄存器交织/解交织
 *
 * 功能: 实现周期性卷积交织与解交织，通过FIFO移位寄存器阵列
 *       分散突发错误，支持可配置延迟和分支数。
 *
 * 协作: DataTransformer(数据变换) / SerialProtocolFuzzer(协议测试)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief 卷积交织/解交织器
 */
class ConvolutionalInterleaver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalInterleaves = 0;       ///< 累计交织操作次数
        quint64 totalDeinterleaves = 0;     ///< 累计解交织操作次数
        quint64 totalSymbolsProcessed = 0;  ///< 累计处理符号数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        quint64 totalBurstErrorsSpread = 0; ///< 累计分散的突发错误数
    };

    explicit ConvolutionalInterleaver(QObject* parent = nullptr);

    /** @brief 配置交织参数 @param branches 分支数B @param delayPerBranch 每分支延迟单元数 */
    void configure(int branches, int delayPerBranch);

    /** @brief 交织一个符号 @param symbol 输入符号 @return 交织后符号 */
    quint8 interleave(quint8 symbol);

    /** @brief 解交织一个符号 @param symbol 输入符号 @return 解交织后符号 */
    quint8 deinterleave(quint8 symbol);

    /** @brief 批量交织 @param data 输入数据 @return 交织后数据 */
    QByteArray interleaveBlock(const QByteArray& data);

    /** @brief 批量解交织 @param data 输入数据 @return 解交织后数据 */
    QByteArray deinterleaveBlock(const QByteArray& data);

    /** @brief 获取当前交织延迟(符号数) @return 总延迟 */
    int totalDelay() const;

    /** @brief 获取分支数 @return 分支数 */
    int branchCount() const;

    /** @brief 重置FIFO移位寄存器状态 */
    void resetState();

    /** @brief 验证交织/解交织往返一致性 @param testData 测试数据 @return true=一致 */
    bool verifyRoundTrip(const QByteArray& testData);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 交织完成 @param symbolCount 符号数 */
    void interleaveComplete(int symbolCount);

    /** @brief 解交织完成 @param symbolCount 符号数 */
    void deinterleaveComplete(int symbolCount);

private:
    quint8 shiftRegister(int branch, quint8 symbol, bool inverse);

    int m_branches;                ///< 分支数B
    int m_delayPerBranch;          ///< 每分支延迟单元数M
    int m_currentBranch;           ///< 当前分支指针(交织)
    int m_currentBranchDe;         ///< 当前分支指针(解交织)

    QVector<QVector<quint8>> m_fifo;       ///< 交织FIFO阵列
    QVector<QVector<quint8>> m_fifoDe;     ///< 解交织FIFO阵列

    Stats m_stats;
    double m_timeSum = 0.0;        ///< 处理时间累加器
};
