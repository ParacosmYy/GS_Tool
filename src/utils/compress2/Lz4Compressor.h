/**
 * @file Lz4Compressor.h
 * @brief LZ4风格快速压缩器 — 高速字节级压缩/解压引擎
 *
 * 实现类LZ4的极高速压缩算法，针对嵌入式调试数据流的实时压缩优化。
 * 使用滑动窗口字节匹配、可配置压缩级别和最小匹配长度。
 * 压缩数据携带自定义头部(魔数+原始大小+压缩级别)以支持自描述格式。
 */
#pragma once

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>

/**
 * @class Lz4Compressor
 * @brief LZ4风格快速压缩/解压引擎
 *
 * 典型用法:
 * @code
 *   Lz4Compressor lz4;
 *   lz4.setLevel(3);
 *   QByteArray compressed = lz4.compress(rawData);
 *   QByteArray recovered = lz4.decompress(compressed);
 * @endcode
 */
class Lz4Compressor : public QObject {
    Q_OBJECT

public:
    /** @brief 压缩/解压操作统计结构 */
    struct Stats {
        quint64 totalCompressed    = 0;    ///< 压缩操作总次数
        quint64 totalDecompressed  = 0;    ///< 解压操作总次数
        quint64 totalBytesIn       = 0;    ///< 输入字节数总计(压缩前)
        quint64 totalBytesOut      = 0;    ///< 输出字节数总计(压缩后)
        double  avgCompressionRatio = 0.0; ///< 平均压缩比(0.0~1.0)
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param compressionLevel 压缩级别1(最快)~9(最佳压缩), 默认1
     * @param parent           父对象
     */
    explicit Lz4Compressor(int compressionLevel = 1, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~Lz4Compressor() override;

    // ── 核心操作 ──

    /**
     * @brief 压缩数据
     *
     * 输出格式: 魔数(2B) + 原始大小(4B) + 级别(1B) + 压缩载荷
     * @param data 原始数据
     * @return 带头部的压缩数据; 空输入或失败返回空并发射error信号
     */
    QByteArray compress(const QByteArray& data);

    /**
     * @brief 解压数据
     *
     * 读取并校验压缩头部，根据头部信息执行解压。
     * @param data 压缩数据(须含头部)
     * @return 原始数据; 头部无效返回空并发射error信号
     */
    QByteArray decompress(const QByteArray& data);

    /**
     * @brief 设置压缩级别
     * @param level 级别1(最快)~9(最佳压缩)
     */
    void setLevel(int level);

    /** @brief 获取当前压缩级别 @return 1~9 */
    int level() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 @return Stats结构体副本 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 压缩完成信号 @param originalSize 原始大小 @param compressedSize 压缩后大小 */
    void compressionCompleted(int originalSize, int compressedSize);
    /** @brief 解压完成信号 @param compressedSize 压缩大小 @param decompressedSize 解压后大小 */
    void decompressionCompleted(int compressedSize, int decompressedSize);
    /** @brief 错误信号 @param errorMessage 错误描述 */
    void error(const QString& errorMessage);

private:
    /** @brief 压缩头部大小: 魔数(2) + 原始大小(4) + 级别(1) = 7字节 */
    static constexpr int HEADER_SIZE = 7;

    /** @brief 魔数标识 */
    static constexpr char MAGIC_0 = 'L';
    static constexpr char MAGIC_1 = '4';

    /** @brief 默认滑动窗口大小(字节) */
    static constexpr int DEFAULT_WINDOW_SIZE = 65536;

    /** @brief 最小匹配长度(低于此值直接输出字面量) */
    static constexpr int MIN_MATCH = 4;

    /** @brief 窗口搜索步进(级别越高搜索越精细) */
    int windowStep(int level) const;

    /** @brief 写入压缩头部 */
    QByteArray writeHeader(quint32 originalSize, quint8 level) const;

    /** @brief 读取并校验压缩头部, 返回原始大小; 失败返回-1 */
    int readHeader(const QByteArray& data) const;

    int  m_level;          ///< 压缩级别 1~9
    Stats m_stats;         ///< 操作统计
    QElapsedTimer m_timer; ///< 处理耗时计时器
};
