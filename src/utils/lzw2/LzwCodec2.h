/**
 * @file LzwCodec2.h
 * @brief LZW编解码器(增强版) — 支持可变位宽和重置字典的高性能LZW实现
 *
 * 提供LZW压缩与解压的完整管线, 支持可变编码位宽、字典重置策略、
 * 多种打包模式, 适用于嵌入式调试中的协议载荷压缩和实时数据流处理。
 */
#ifndef LZW_CODEC2_H
#define LZW_CODEC2_H

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class LzwCodec2
 * @brief 增强版LZW编解码器
 *
 * 支持可变位宽(9~16bit)和字典满时自动重置。
 * 典型用法:
 * @code
 *   LzwCodec2 codec;
 *   QByteArray enc = codec.encode(rawData);
 *   QByteArray dec = codec.decode(enc);
 * @endcode
 */
class LzwCodec2 : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalEncodes = 0;       ///< 编码操作总次数
        quint64 totalDecodes = 0;       ///< 解码操作总次数
        quint64 totalBytesIn = 0;       ///< 输入字节总数
        quint64 totalBytesOut = 0;      ///< 输出字节总数
        double  avgRatio = 0.0;         ///< 平均压缩比(输出/输入)
        int     dictResets = 0;         ///< 字典重置次数
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit LzwCodec2(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~LzwCodec2() override;

    // ── 配置 ──

    /**
     * @brief 设置最大编码位宽
     * @param bits 位宽(9~16), 默认12
     */
    void setMaxCodeBits(int bits);

    /** @brief 获取当前最大编码位宽 */
    int maxCodeBits() const;

    /**
     * @brief 设置字典满时是否自动重置
     * @param enable true则字典满时重置到初始状态
     */
    void setAutoReset(bool enable);

    // ── 编解码 ──

    /**
     * @brief LZW编码(压缩)
     * @param data 原始数据
     * @return 编码后数据; 失败返回空并发射error信号
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief LZW解码(解压)
     * @param data 编码数据
     * @return 原始数据; 失败返回空并发射error信号
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
    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /** @brief 位打包: 编码列表转字节数组 */
    QByteArray packCodes(const QVector<quint32>& codes, int codeSize) const;

    /** @brief 位解包: 字节数组转编码列表 */
    QVector<quint32> unpackCodes(const QByteArray& data,
                                 int codeSize, int count) const;

    /** @brief 计算当前最大编码值所需位数 */
    int calcBits(quint32 maxCode) const;

    int  m_maxCodeBits = 12;   ///< 最大编码位宽
    bool m_autoReset = true;   ///< 字典满时自动重置
    mutable Stats m_stats;     ///< 操作统计(mutable允许const方法修改)
};

#endif // LZW_CODEC2_H
