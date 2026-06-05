/**
 * @file HammingCode.h
 * @brief 汉明码编解码器 — 单错纠正/双错检测
 *
 * 功能: 4位数据编码为7位汉明码，检测并纠正单比特错误，
 *       检测双比特错误，编解码统计。
 */
#ifndef HAMMINGCODE_H
#define HAMMINGCODE_H

#include <QObject>
#include <QVector>

/**
 * @brief 汉明码编解码器(7,4)
 */
class HammingCode : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalEncodes = 0;       ///< 累计编码次数
        quint64 totalDecodes = 0;       ///< 累计解码次数
        quint64 totalCorrections = 0;   ///< 累计纠正次数
        quint64 totalDoubleErrors = 0;  ///< 累计双错检测次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    /** @brief 解码结果 */
    struct DecodeResult {
        quint8 data = 0;                ///< 解码后的4位数据
        bool   singleBitError = false;  ///< 是否检测到单比特错误
        bool   doubleBitError = false;  ///< 是否检测到双比特错误
        int    errorPosition = 0;       ///< 错误位置(0=无错, 1-7=出错位)
    };

    explicit HammingCode(QObject* parent = nullptr);

    /** @brief 编码4位数据为7位汉明码 @param data 低4位有效 @return 7位编码(低7位有效) */
    quint8 encode(quint8 data);

    /** @brief 批量编码 @param data 输入数据(每字节低4位有效) @return 编码结果 */
    QVector<quint8> encodeBatch(const QVector<quint8>& data);

    /** @brief 解码7位汉明码 @param code 7位编码(低7位有效) @return 解码结果 */
    DecodeResult decode(quint8 code);

    /** @brief 批量解码 @param codes 编码列表 @return 解码结果列表 */
    QVector<DecodeResult> decodeBatch(const QVector<quint8>& codes);

    /** @brief 注入单比特错误 @param code 编码 @param bitPos 错误位(1-7) @return 翻转后的编码 */
    static quint8 injectError(quint8 code, int bitPos);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param data 原始数据 @param code 编码结果 */
    void encoded(quint8 data, quint8 code);
    /** @brief 解码完成 @param result 解码结果 */
    void decoded(const DecodeResult& result);
    /** @brief 错误纠正 @param position 错误位置 */
    void errorCorrected(int position);
    /** @brief 检测到双比特错误 */
    void doubleErrorDetected();

private:
    /** @brief 计算校正子 @param code 7位编码 @return 校正子 */
    int calculateSyndrome(quint8 code) const;

    Stats m_stats;
    double m_timeSum;
};

#endif // HAMMINGCODE_H
