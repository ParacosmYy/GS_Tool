/**
 * @file SpinalCode.h
 * * @brief Spinal码 — 基于序列哈希编码的无率码(Rateless Code)编解码器
 *
 * 功能: 实现Spinal码的编码器和渐进解码器。编码器将消息分割为k-bit符号，
 *       通过级联哈希函数(f)逐脊骨(spine)生成伪随机编码符号。
 *       解码器使用BFS/DFS在脊骨值树上搜索最大似然路径。
 *
 * 协作: CrcStreamVerifier(校验) / DataCompressor(数据压缩)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
#include <QByteArray>

/**
 * @brief Spinal码编解码器 — 序列哈希编码 + BFS渐进解码
 */
class SpinalCode : public QObject {
    Q_OBJECT

public:
    /** @brief 编码参数 */
    struct CodeParams {
        int k = 4;                     ///< 每符号比特数
        int spineLength = 0;           ///< 脊骨长度(自动计算)
        int numPasses = 8;             ///< 编码遍数
        int bfsWidth = 16;             ///< BFS搜索宽度
        int hashBits = 32;             ///< 哈希输出比特数
    };

    /** @brief 解码结果 */
    struct DecodeResult {
        QByteArray decodedData;        ///< 解码输出
        double confidence = 0.0;       ///< 解码置信度[0,1]
        int passesUsed = 0;            ///< 使用的编码遍数
        bool success = false;          ///< 是否解码成功
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        int totalEncodes = 0;                  ///< 累计编码次数
        int totalDecodes = 0;                  ///< 累计解码次数
        int totalBytesProcessed = 0;           ///< 累计处理字节数
        int totalSymbolsGenerated = 0;         ///< 累计生成符号数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit SpinalCode(QObject* parent = nullptr);

    /** @brief 设置编码参数 @param params 参数 */
    void setParams(const CodeParams& params);

    /** @brief 编码数据 @param data 输入字节 @return 编码符号(浮点映射) */
    QVector<double> encode(const QByteArray& data);

    /** @brief 解码符号 @param symbols 接收符号 @param originalSize 原始字节数 @return 解码结果 */
    DecodeResult decode(const QVector<double>& symbols, int originalSize);

    /** @brief 计算当前码率 @param dataLen 数据长度 @param symbolCount 符号数 @return 码率 */
    double codeRate(int dataLen, int symbolCount) const;

    /** @brief 获取当前统计 @return 统计常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 编码完成 @param bits 输入比特数 @param symbols 输出符号数 */
    void encodeComplete(int bits, int symbols);

    /** @brief 解码完成 @param success 是否成功 @param confidence 置信度 */
    void decodeComplete(bool success, double confidence);

private:
    /** @brief 脊骨哈希函数f: 将spine值和符号映射为伪随机输出 @param spine 当前脊骨值 @param symbol 输入符号 @return 新的脊骨值 */
    quint32 spineHash(quint32 spine, int symbol) const;

    /** @brief 从脊骨值生成RNG种子并映射到符号 @param spineValue 脊骨值 @param passIndex 编码遍索引 @return 映射后的符号值 */
    double mapToSymbol(quint32 spineValue, int passIndex) const;

    /** @brief 将字节数组展开为k-bit符号序列 @param data 输入数据 @return 符号序列 */
    QVector<int> bytesToSymbols(const QByteArray& data) const;

    /** @brief 将k-bit符号序列还原为字节数组 @param symbols 符号序列 @param byteCount 目标字节数 @return 字节数组 */
    QByteArray symbolsToBytes(const QVector<int>& symbols, int byteCount) const;

    /** @brief 符号距离度量(欧氏距离平方) @param a 符号a @param b 符号b @return 距离 */
    static double symbolDistance(double a, double b);

    CodeParams m_params;               ///< 编码参数

    Stats m_stats;                      ///< 运行时统计
    double m_timeSum = 0.0;             ///< 累计耗时(ms)
};
