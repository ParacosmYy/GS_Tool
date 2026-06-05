/**
 * @file ShannonCoder.h
 * @brief Shannon-Fano编码器 — 信息论最优前缀码
 *
 * 功能: 基于符号频率的Shannon-Fano编码/解码，
 *       计算信源熵/编码效率/冗余度，
 *       统计编解码次数/符号数/耗时。
 */
#ifndef SHANNONCODER_H
#define SHANNONCODER_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QMap>

/**
 * @brief Shannon-Fano编码器
 */
class ShannonCoder : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalEncodes = 0;       ///< 累计编码次数
        quint64 totalDecodes = 0;       ///< 累计解码次数
        double  avgEfficiency = 0.0;    ///< 平均编码效率
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    /** @brief 符号统计 */
    struct SymbolInfo {
        int symbol = 0;                 ///< 符号值
        double probability = 0.0;       ///< 概率
        QByteArray code;                ///< 编码位串
    };

    explicit ShannonCoder(QObject* parent = nullptr);

    /** @brief 编码 @param data 输入数据 @return 编码结果(头部+压缩数据) */
    QByteArray encode(const QByteArray& data);

    /** @brief 解码 @param data 编码数据 @return 原始数据 */
    QByteArray decode(const QByteArray& data);

    /** @brief 计算Shannon熵 @param data 数据 @return 熵(bit/symbol) */
    double entropy(const QByteArray& data) const;

    /** @brief 获取符号表 @return 符号信息列表 */
    QVector<SymbolInfo> symbolTable() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param result 编码结果 @param efficiency 编码效率 */
    void encoded(const QByteArray& result, double efficiency);
    /** @brief 解码完成 @param result 解码结果 */
    void decoded(const QByteArray& result);
    /** @brief 错误 @param message 错误信息 */
    void error(const QString& message);

private:
    /** @brief 构建编码表 @param freq 频率表 @param total 总数 */
    void buildCodeTable(const QVector<quint32>& freq, int total);
    /** @brief 构建频率表 @param data 数据 @return 256项频率表 */
    QVector<quint32> buildFreqTable(const QByteArray& data) const;

    QMap<int, QByteArray> m_codeTable;  ///< 符号->编码映射
    QVector<SymbolInfo> m_symbolTable;  ///< 符号信息表
    Stats m_stats;
    double m_timeSum;                   ///< 处理时间累加器
};

#endif // SHANNONCODER_H
