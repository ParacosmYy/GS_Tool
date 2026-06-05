/**
 * @file HammingCode2.h
 * @brief 扩展汉明码(SEC-DED) — 编码/解码/纠错/校验矩阵生成
 *
 * 功能: 实现(7,4)/(15,11)/(31,26)等扩展汉明码，支持单纠错双检
 *       测(SEC-DED)，校验子表查找，奇偶校验矩阵生成，用于串口
 *       通信中的前向纠错编码。
 *
 * 协作: PacketBuilder(数据组包) / CrcStreamVerifier(完整性校验)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QBitArray>
#include <QMap>

/**
 * @brief 扩展汉明码编解码器 — SEC-DED纠错
 */
class HammingCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief 编码参数 */
    struct CodeParams {
        int n = 0;           ///< 码字长度(含校验位)
        int k = 0;           ///< 信息位长度
        int m = 0;           ///< 校验位数量
        int dmin = 3;        ///< 最小汉明距离
    };

    /** @brief 解码结果 */
    struct DecodeResult {
        bool     success = false;      ///< 是否成功解码
        bool     corrected = false;    ///< 是否执行了纠错
        int      errorPosition = -1;   ///< 错误比特位置(-1=无错)
        int      errorsDetected = 0;   ///< 检测到的错误数
        QBitArray correctedCodeword;   ///< 纠错后的码字
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEncoded = 0;          ///< 累计编码次数
        quint64 totalDecoded = 0;          ///< 累计解码次数
        quint64 totalCorrections = 0;      ///< 累计纠正次数
        quint64 totalUncorrectable = 0;    ///< 累计不可纠正错误
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit HammingCode2(QObject* parent = nullptr);

    /** @brief 设置码参数(2^m-1位码) @param m 校验位数(≥3) */
    void setCodeOrder(int m);

    /** @brief 获取当前码参数 @return 码参数 */
    CodeParams codeParams() const;

    /** @brief 编码 @param data 信息位 @return 码字(含扩展奇偶位) */
    QBitArray encode(const QBitArray& data);

    /** @brief 解码并纠错 @param codeword 接收码字 @return 解码结果 */
    DecodeResult decode(const QBitArray& codeword);

    /** @brief 从字节数组编码 @param bytes 原始数据 @return 编码后比特 */
    QBitArray encodeBytes(const QByteArray& bytes);

    /** @brief 解码到字节数组 @param bits 编码比特 @return (数据字节, 解码结果) */
    QPair<QByteArray, DecodeResult> decodeToBytes(const QBitArray& bits);

    /** @brief 生成奇偶校验矩阵H @return H矩阵(行为校验方程) */
    QVector<QBitArray> parityCheckMatrix() const;

    /** @brief 生成生成矩阵G @return G矩阵 */
    QVector<QBitArray> generatorMatrix() const;

    /** @brief 构建校验子查找表 @return 校验子→错误位置映射 */
    QMap<int, int> syndromeTable() const;

    /** @brief 计算汉明距离 @param a 比特串a @param b 比特串b @return 汉明距离 */
    static int hammingDistance(const QBitArray& a, const QBitArray& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param infoBits 信息位数 @param codeBits 码字位数 */
    void encoded(int infoBits, int codeBits);

    /** @brief 纠错完成 @param position 错误位置 @param corrected 是否成功纠正 */
    void errorCorrected(int position, bool corrected);

private:
    bool isPowerOfTwo(int x) const;
    int  syndromeToPosition(int syndrome) const;
    QBitArray computeSyndrome(const QBitArray& codeword) const;
    bool computeOverallParity(const QBitArray& bits) const;

    int m_m;                       ///< 校验位数

    Stats m_stats;
    double m_timeSum = 0.0;        ///< 处理时间累加器
};
