/**
 * @file RunLengthCodec.h
 * @brief 游程编码(RLE)编解码器 — 可配置转义字节的安全二进制数据处理
 *
 * 提供游程编码(RLE)的编码与解码, 支持可配置转义字节,
 * 安全处理任意二进制数据, 适用于嵌入式调试场景中的
 * 重复数据模式压缩(如波形数据、恒定信号段)。
 */
#ifndef RUN_LENGTH_CODEC_H
#define RUN_LENGTH_CODEC_H

#include <QObject>
#include <QByteArray>

/**
 * @class RunLengthCodec
 * @brief 游程编码(RLE)编解码器
 *
 * 编码格式: 转义字节 + 重复字节值 + 重复次数(1字节, 最大255)
 * 非重复数据原样输出; 转义字节本身也会被编码。
 *
 * 典型用法:
 * @code
 *   RunLengthCodec codec;
 *   codec.setEscapeByte(0xFF);
 *   QByteArray encoded = codec.encode(rawData);
 *   QByteArray decoded = codec.decode(encoded);
 * @endcode
 */
class RunLengthCodec : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalEncodes = 0;         ///< 编码操作总次数
        quint64 totalDecodes = 0;         ///< 解码操作总次数
        double  compressionRatio = 0.0;   ///< 平均压缩比(输出/输入)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit RunLengthCodec(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~RunLengthCodec() override;

    // ── 配置 ──

    /**
     * @brief 设置转义字节
     * @param escape 转义字节值, 默认0xFF
     */
    void setEscapeByte(quint8 escape);

    /** @brief 获取当前转义字节 */
    quint8 escapeByte() const;

    /**
     * @brief 设置最小重复阈值(低于此值不编码为RLE)
     * @param threshold 最小重复次数, 默认3
     */
    void setMinRunLength(int threshold);

    /** @brief 获取当前最小重复阈值 */
    int minRunLength() const;

    // ── 编解码 ──

    /**
     * @brief RLE编码
     * @param data 原始数据
     * @return 编码后数据; 失败返回空并发射error信号
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief RLE解码
     * @param data 编码数据
     * @return 解码后原始数据; 失败返回空并发射error信号
     */
    QByteArray decode(const QByteArray& data);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 @param result 编码结果 @param ratio 压缩比 */
    void encoded(const QByteArray& result, double ratio);
    /** @brief 解码完成信号 @param result 解码结果 */
    void decoded(const QByteArray& result);
    /** @brief 错误信号 @param errorMessage 错误描述 */
    void error(const QString& errorMessage);

private:
    /** @brief 更新平均压缩比 */
    void updateCompressionRatio(double ratio);

    /** @brief 转义字节 */
    quint8 m_escape = 0xFF;

    /** @brief 最小游程长度阈值 */
    int m_minRun = 3;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // RUN_LENGTH_CODEC_H
