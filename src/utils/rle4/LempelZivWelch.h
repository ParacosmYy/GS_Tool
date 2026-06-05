/**
 * @file LempelZivWelch.h
 * @brief LZW自适应字典压缩编解码器
 *
 * 提供LZW(Lempel-Ziv-Welch)自适应字典压缩的完整实现,
 * 支持可配置字典大小和编码位宽, 适用于嵌入式调试场景中的
 * 数据传输压缩和协议载荷优化。
 */
#ifndef LEMPEL_ZIV_WELCH_H
#define LEMPEL_ZIV_WELCH_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QHash>

/**
 * @class LempelZivWelch
 * @brief LZW自适应字典压缩编解码器
 *
 * 典型用法:
 * @code
 *   LempelZivWelch lzw;
 *   lzw.setDictSize(4096);
 *   QByteArray compressed = lzw.compress(rawData);
 *   QByteArray recovered = lzw.decompress(compressed);
 * @endcode
 */
class LempelZivWelch : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalCompressions = 0;     ///< 压缩操作总次数
        quint64 totalDecompressions = 0;   ///< 解压操作总次数
        quint64 totalBytesIn = 0;          ///< 输入字节总数
        quint64 totalBytesOut = 0;         ///< 输出字节总数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit LempelZivWelch(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~LempelZivWelch() override;

    // ── 配置 ──

    /**
     * @brief 设置最大字典大小
     * @param size 最大字典条目数, 默认4096
     */
    void setDictSize(int size);

    /** @brief 获取当前最大字典大小 */
    int dictSize() const;

    // ── 压缩/解压 ──

    /**
     * @brief LZW压缩
     * @param data 原始数据
     * @return 压缩后的编码数据
     */
    QByteArray compress(const QByteArray& data);

    /**
     * @brief LZW解压
     * @param data 压缩数据
     * @return 解压后的原始数据
     */
    QByteArray decompress(const QByteArray& data);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 压缩完成信号 @param ratio 压缩比 */
    void compressed(double ratio);
    /** @brief 解压完成信号 @param originalSize 原始数据大小 */
    void decompressed(int originalSize);
    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /**
     * @brief 将编码序列打包为字节数组
     * @param codes 编码序列
     * @param codeSize 每个编码位数
     * @return 打包后的字节数组
     */
    QByteArray packCodes(const QVector<quint32>& codes, int codeSize) const;

    /**
     * @brief 从字节数组解包编码序列
     * @param data 打包数据
     * @param codeSize 每个编码位数
     * @param count 编码数量
     * @return 编码序列
     */
    QVector<quint32> unpackCodes(const QByteArray& data, int codeSize,
                                 int count) const;

    /**
     * @brief 计算所需编码位数
     * @param maxCode 最大编码值
     * @return 所需位数
     */
    int calcCodeBits(quint32 maxCode) const;

    /**
     * @brief 更新平均处理时间
     * @param elapsedMs 本次耗时(ms)
     */
    void updateAvgTime(double elapsedMs) const;

    /** @brief 最大字典大小 */
    int m_dictSize = 4096;

    /** @brief 操作统计(mutable支持const方法更新) */
    mutable Stats m_stats;
};

#endif // LEMPEL_ZIV_WELCH_H
