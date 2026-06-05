/**
 * @file RunLengthEncoder.h
 * @brief 增强型游程编码器 — PackBits/CCITT Group 3/自定义RLE
 *
 * 提供多种游程编码算法: PackBits(Apple)、CCITT Group 3(传真)、
 * 以及可配置阈值的自定义RLE, 适用于嵌入式调试数据流的
 * 重复模式压缩和协议帧压缩。
 */
#ifndef RUN_LENGTH_ENCODER_H
#define RUN_LENGTH_ENCODER_H

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class RunLengthEncoder
 * @brief 增强型游程编码器
 *
 * 支持3种RLE模式:
 * - PackBits: Apple标准, 1字节计数+数据
 * - CCITT Group 3: 传真标准, EOL+行程编码
 * - Custom: 可配置最小重复阈值
 */
class RunLengthEncoder : public QObject {
    Q_OBJECT

public:
    /** @brief RLE编码模式 */
    enum Mode {
        PackBits = 0,       ///< Apple PackBits格式
        CcittGroup3 = 1,    ///< CCITT Group 3传真格式
        CustomRle = 2       ///< 自定义可配置RLE
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
    explicit RunLengthEncoder(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~RunLengthEncoder() override;

    // ── 配置 ──

    /**
     * @brief 设置编码模式
     * @param mode 编码模式枚举
     */
    void setMode(Mode mode);

    /** @brief 获取当前编码模式 */
    Mode mode() const;

    /**
     * @brief 设置自定义RLE最小重复阈值
     * @param threshold 最小重复次数(默认3)
     */
    void setMinRun(int threshold);

    // ── 编解码 ──

    /**
     * @brief 游程编码(压缩)
     * @param data 原始数据
     * @return 编码后数据
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief 游程解码(解压)
     * @param data 编码数据
     * @return 原始数据
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
    /** @brief PackBits编码 */
    QByteArray encodePackBits(const QByteArray& data);
    /** @brief PackBits解码 */
    QByteArray decodePackBits(const QByteArray& data);
    /** @brief CCITT Group 3编码 */
    QByteArray encodeCcitt(const QByteArray& data);
    /** @brief CCITT Group 3解码 */
    QByteArray decodeCcitt(const QByteArray& data);
    /** @brief 自定义RLE编码 */
    QByteArray encodeCustom(const QByteArray& data);
    /** @brief 自定义RLE解码 */
    QByteArray decodeCustom(const QByteArray& data);

    Mode m_mode = PackBits;      ///< 当前编码模式
    int  m_minRun = 3;           ///< 最小重复阈值
    mutable Stats m_stats;       ///< 操作统计
};

#endif // RUN_LENGTH_ENCODER_H
