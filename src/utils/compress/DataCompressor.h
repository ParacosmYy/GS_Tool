/**
 * @file DataCompressor.h
 * @brief 数据压缩器 — 提供串口通信带宽优化的压缩/解压操作
 *
 * 支持 RunLength / Huffman / LZ77 / Deflate 四种压缩算法，
 * 提供压缩率预估、压缩数据检测与压缩级别控制，
 * 适用于嵌入式调试场景中串口带宽有限时的数据传输优化。
 * 统计模块记录压缩解压次数、字节数、错误数及各算法调用分布。
 */
#ifndef DATA_COMPRESSOR_H
#define DATA_COMPRESSOR_H

#include <QObject>
#include <QByteArray>

/**
 * @class DataCompressor
 * @brief 数据压缩/解压引擎 — RLE/Huffman/LZ77/Deflate 多算法统一接口
 *
 * 典型用法:
 * @code
 *   DataCompressor compressor;
 *   compressor.setCompressionLevel(6);
 *   QByteArray compressed = compressor.compress(data, DataCompressor::Algorithm::Deflate);
 *   QByteArray recovered = compressor.decompress(compressed, DataCompressor::Algorithm::Deflate);
 * @endcode
 */
class DataCompressor : public QObject {
    Q_OBJECT

public:
    /** @brief 支持的压缩算法枚举 */
    enum class Algorithm {
        RunLength = 0, ///< 游程编码(适合重复字节模式)
        Huffman,       ///< 霍夫曼变长编码(适合已知频率分布)
        LZ77,          ///< LZ77滑动窗口压缩(适合通用数据)
        Deflate,       ///< 类Deflate: LZ77+Huffman组合(最佳压缩比)
        None           ///< 无压缩(透传)
    };
    Q_ENUM(Algorithm)

    /** @brief 压缩/解压操作统计结构 */
    struct Stats {
        quint64 totalCompressions = 0;          ///< 压缩操作总次数
        quint64 totalDecompressions = 0;        ///< 解压操作总次数
        quint64 totalBytesIn = 0;               ///< 输入字节数总计(压缩前)
        quint64 totalBytesOut = 0;              ///< 输出字节数总计(压缩后)
        double  avgRatio = 0.0;                 ///< 平均压缩比(0.0~1.0)
        quint64 operationsByAlgorithm[5] = {};  ///< 各算法操作次数[RLE..None]
        quint64 compressErrors = 0;             ///< 压缩/解压错误次数
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit DataCompressor(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~DataCompressor() override;

    // ── 核心操作 ──

    /**
     * @brief 压缩数据
     * @param data 原始数据
     * @param algorithm 压缩算法
     * @return 增加压缩头部的压缩数据; 失败返回空并发射 error 信号
     */
    QByteArray compress(const QByteArray& data, Algorithm algorithm);

    /**
     * @brief 解压数据
     * @param data 压缩数据(须含压缩头部)
     * @param algorithm 解压算法(须与压缩时一致)
     * @return 原始数据; 失败返回空并发射 error 信号
     */
    QByteArray decompress(const QByteArray& data, Algorithm algorithm);

    // ── 辅助功能 ──

    /**
     * @brief 预估压缩率(不执行实际压缩)
     * @param data 待分析数据
     * @param algorithm 目标算法
     * @return 预估压缩比(0.0~1.0, 越低压缩效果越好)
     */
    double estimateRatio(const QByteArray& data, Algorithm algorithm) const;

    /**
     * @brief 检测数据是否已被压缩(基于熵值和头部标记)
     * @param data 待检测数据
     * @return true 表示数据可能已被压缩
     */
    bool isCompressed(const QByteArray& data) const;

    /**
     * @brief 设置压缩级别
     * @param level 压缩级别 1(最快)~9(最佳压缩), 默认6
     */
    void setCompressionLevel(int level);

    /** @brief 获取当前压缩级别 @return 1~9 */
    int compressionLevel() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 @return Stats结构体副本 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 压缩完成信号 @param result 压缩结果 @param algorithm 使用的算法 */
    void compressed(const QByteArray& result, Algorithm algorithm);
    /** @brief 解压完成信号 @param result 解压结果 @param algorithm 使用的算法 */
    void decompressed(const QByteArray& result, Algorithm algorithm);
    /** @brief 错误信号 @param errorMessage 错误描述 */
    void error(const QString& errorMessage);

private:
    // ── 内部算法实现 ──

    /** @brief RLE压缩 */
    QByteArray compressRle(const QByteArray& data);
    /** @brief RLE解压 */
    QByteArray decompressRle(const QByteArray& data);

    /** @brief Huffman压缩 */
    QByteArray compressHuffman(const QByteArray& data);
    /** @brief Huffman解压 */
    QByteArray decompressHuffman(const QByteArray& data);

    /** @brief LZ77压缩 */
    QByteArray compressLz77(const QByteArray& data);
    /** @brief LZ77解压 */
    QByteArray decompressLz77(const QByteArray& data);

    /** @brief Deflate压缩(LZ77+Huffman) */
    QByteArray compressDeflate(const QByteArray& data);
    /** @brief Deflate解压 */
    QByteArray decompressDeflate(const QByteArray& data);

    // ── Huffman辅助 ──

    /** @brief Huffman树节点 */
    struct HuffNode {
        int      symbol = -1;  ///< 字节值(-1=内部节点)
        quint32  freq = 0;     ///< 频率
        HuffNode* left = nullptr;  ///< 左子节点
        HuffNode* right = nullptr; ///< 右子节点
        QString  code;         ///< 编码位串
    };

    /** @brief 构建频率表 */
    QVector<quint32> buildFreqTable(const QByteArray& data) const;
    /** @brief 构建Huffman树, 返回根节点(调用者负责释放) */
    HuffNode* buildHuffmanTree(const QVector<quint32>& freq);
    /** @brief 递归生成编码表 */
    void generateCodes(HuffNode* node, const QString& prefix);
    /** @brief 递归释放Huffman树 */
    void freeHuffmanTree(HuffNode* node);
    /** @brief 位串转字节数组 */
    QByteArray bitsToBytes(const QString& bits) const;
    /** @brief 字节数组转位串 */
    QString bytesToBits(const QByteArray& bytes, int bitCount) const;

    // ── LZ77辅助 ──

    /** @brief LZ77匹配结果 */
    struct LzMatch {
        int offset = 0;  ///< 回溯偏移量
        int length = 0;  ///< 匹配长度
    };

    /** @brief 在滑动窗口中搜索最长匹配 */
    LzMatch findLzMatch(const QByteArray& data, int pos, int windowSize) const;

    // ── 通用辅助 ──

    /** @brief 计算数据的信息熵(0.0~8.0) */
    double calculateEntropy(const QByteArray& data) const;

    /** @brief 写入压缩头部: 魔数(2B) + 算法(1B) + 原始大小(4B) + 压缩级别(1B) */
    QByteArray writeHeader(Algorithm algo, quint32 originalSize) const;
    /** @brief 读取并验证压缩头部, 返回原始大小; 失败返回-1 */
    int readHeader(const QByteArray& data, Algorithm expectedAlgo) const;

    int  m_level = 6;  ///< 压缩级别 1~9
    Stats m_stats;     ///< 操作统计数据
};

#endif // DATA_COMPRESSOR_H
