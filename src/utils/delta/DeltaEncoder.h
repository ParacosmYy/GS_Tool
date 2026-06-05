/**
 * @file DeltaEncoder.h
 * @brief Delta编码器 — 增量编码与ZigZag编码
 *
 * 提供Delta(增量)编码/解码和ZigZag编码, 适用于时序数据压缩、
 * 传感器数据差分传输和嵌入式调试中的数值序列压缩。
 * ZigZag编码将有符号整数映射为无符号整数, 使小绝对值
 * 的负数也能用较少比特表示。
 */
#ifndef DELTA_ENCODER_H
#define DELTA_ENCODER_H

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class DeltaEncoder
 * @brief Delta增量编码器(含ZigZag编码)
 *
 * 支持: 基础Delta编码、ZigZag+Delta联合编码、
 * 可配置初始参考值的增量编码。
 */
class DeltaEncoder : public QObject {
    Q_OBJECT

public:
    /** @brief 编码模式 */
    enum Mode {
        RawDelta = 0,       ///< 原始差分(有符号)
        ZigZagDelta = 1,    ///< ZigZag编码后的差分
        XorDelta = 2        ///< 异或差分(适用于浮点模式)
    };
    Q_ENUM(Mode)

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalEncodes = 0;       ///< 编码操作总次数
        quint64 totalDecodes = 0;       ///< 解码操作总次数
        quint64 totalBytesIn = 0;       ///< 输入字节总数
        quint64 totalBytesOut = 0;      ///< 输出字节总数
        double  avgRatio = 0.0;         ///< 平均压缩比
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit DeltaEncoder(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~DeltaEncoder() override;

    // ── 配置 ──

    /** @brief 设置编码模式 */
    void setMode(Mode mode);

    /** @brief 获取当前编码模式 */
    Mode mode() const;

    // ── 整数序列编解码 ──

    /**
     * @brief 对qint64序列进行Delta编码
     * @param values 输入整数序列
     * @return 编码后的差分序列
     */
    QVector<qint64> encodeInt64(const QVector<qint64>& values);

    /**
     * @brief 对Delta编码的qint64序列进行解码
     * @param deltas 编码后的差分序列
     * @return 解码后的原始整数序列
     */
    QVector<qint64> decodeInt64(const QVector<qint64>& deltas);

    // ── 字节流编解码 ──

    /**
     * @brief 对字节数组进行Delta编码
     * @param data 原始字节数据
     * @return 编码后数据
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief 对Delta编码的字节数组进行解码
     * @param data 编码数据
     * @return 解码后原始数据
     */
    QByteArray decode(const QByteArray& data);

    // ── ZigZag编解码 ──

    /**
     * @brief ZigZag编码: 有符号→无符号映射
     * @param n 有符号整数
     * @return 无符号整数
     */
    static quint64 zigzagEncode(qint64 n);

    /**
     * @brief ZigZag解码: 无符号→有符号映射
     * @param n 无符号整数
     * @return 有符号整数
     */
    static qint64 zigzagDecode(quint64 n);

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
    Mode m_mode = ZigZagDelta;    ///< 当前编码模式
    mutable Stats m_stats;        ///< 操作统计
};

#endif // DELTA_ENCODER_H
