/**
 * @file LzssCompressor.h
 * @brief LZSS压缩/解压引擎 — 滑动窗口字典压缩
 *
 * 实现LZSS(Lempel-Ziv-Storer-Szymanski)压缩算法,
 * 使用固定长度编码的滑动窗口字典匹配, 适用于嵌入式固件
 * 数据压缩、日志压缩等场景。
 */
#ifndef LZSSCOMPRESSOR_H
#define LZSSCOMPRESSOR_H

#include <QObject>
#include <QByteArray>

/**
 * @class LzssCompressor
 * @brief LZSS压缩引擎 — 滑动窗口字典压缩/解压
 *
 * 典型用法:
 * @code
 *   LzssCompressor lzss;
 *   QByteArray compressed = lzss.compress(data);
 *   QByteArray recovered = lzss.decompress(compressed);
 * @endcode
 */
class LzssCompressor : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalCompressions = 0;   ///< 总压缩次数
        quint64 totalDecompressions = 0;  ///< 总解压次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit LzssCompressor(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~LzssCompressor() override;

    // ── 核心接口 ──

    /**
     * @brief 压缩数据
     * @param data 输入数据
     * @return 压缩后数据
     */
    QByteArray compress(const QByteArray& data);

    /**
     * @brief 解压数据
     * @param data 压缩数据(由compress产生)
     * @return 解压后原始数据
     */
    QByteArray decompress(const QByteArray& data);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 压缩完成信号 @param originalSize 原始大小 @param compressedSize 压缩后大小 */
    void compressionCompleted(int originalSize, int compressedSize);
    /** @brief 解压完成信号 @param compressedSize 压缩大小 @param decompressedSize 解压后大小 */
    void decompressionCompleted(int compressedSize, int decompressedSize);

private:
    /** @brief 滑动窗口大小 */
    static constexpr int WINDOW_SIZE = 4096;

    /** @brief 最小匹配长度 */
    static constexpr int MIN_MATCH = 3;

    /** @brief 最大匹配长度 */
    static constexpr int MAX_MATCH = 18;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // LZSSCOMPRESSOR_H
