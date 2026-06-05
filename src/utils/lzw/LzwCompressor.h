/**
 * @file LzwCompressor.h
 * @brief LZW压缩引擎 — 基于字典的自适应压缩/解压
 *
 * 提供LZW(Lempel-Ziv-Welch)字典压缩算法的完整实现,
 * 支持可配置最大字典大小, 适用于嵌入式调试场景中的
 * 数据传输优化和协议载荷压缩。
 */
#ifndef LZW_COMPRESSOR_H
#define LZW_COMPRESSOR_H

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class LzwCompressor
 * @brief LZW字典压缩引擎
 *
 * 典型用法:
 * @code
 *   LzwCompressor lzw;
 *   lzw.setMaxDictSize(4096);
 *   QByteArray compressed = lzw.compress(rawData);
 *   QByteArray recovered = lzw.decompress(compressed);
 * @endcode
 */
class LzwCompressor : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalCompressions = 0;     ///< 压缩操作总次数
        quint64 totalDecompressions = 0;   ///< 解压操作总次数
        double  avgRatio = 0.0;            ///< 平均压缩比(输出/输入)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit LzwCompressor(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~LzwCompressor() override;

    // ── 配置 ──

    /**
     * @brief 设置最大字典大小
     * @param maxSize 最大字典条目数, 默认4096
     */
    void setMaxDictSize(int maxSize);

    /** @brief 获取当前最大字典大小 */
    int maxDictSize() const;

    // ── 压缩/解压 ──

    /**
     * @brief LZW压缩
     * @param data 原始数据
     * @return 压缩后的编码数据; 失败返回空并发射error信号
     */
    QByteArray compress(const QByteArray& data);

    /**
     * @brief LZW解压
     * @param data 压缩数据
     * @return 解压后的原始数据; 失败返回空并发射error信号
     */
    QByteArray decompress(const QByteArray& data);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 压缩完成信号 @param result 压缩结果 @param ratio 压缩比 */
    void compressed(const QByteArray& result, double ratio);
    /** @brief 解压完成信号 @param result 解压结果 */
    void decompressed(const QByteArray& result);
    /** @brief 错误信号 @param errorMessage 错误描述 */
    void error(const QString& errorMessage);

private:
    /**
     * @brief 将编码索引序列打包为字节数组
     * @param codes 编码索引列表
     * @param codeSize 每个编码的位数
     * @return 打包后的字节数组
     */
    QByteArray packCodes(const QVector<quint32>& codes, int codeSize) const;

    /**
     * @brief 从字节数组解包编码索引序列
     * @param data 打包数据
     * @param codeSize 每个编码的位数
     * @param codeCount 编码数量
     * @return 编码索引列表
     */
    QVector<quint32> unpackCodes(const QByteArray& data, int codeSize,
                                 int codeCount) const;

    /**
     * @brief 计算所需编码位数
     * @param maxCode 最大编码值
     * @return 所需位数
     */
    int calcCodeBits(quint32 maxCode) const;

    /** @brief 更新平均压缩比 */
    void updateAvgRatio(double ratio);

    /** @brief 最大字典大小 */
    int m_maxDictSize = 4096;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // LZW_COMPRESSOR_H
