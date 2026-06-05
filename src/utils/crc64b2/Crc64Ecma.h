/**
 * @file Crc64Ecma.h
 * @brief CRC-64/ECMA-182校验引擎 — 查找表加速的流式校验
 *
 * 提供CRC-64/ECMA-182多项式的校验计算, 支持流式单字节更新,
 * 适用于嵌入式通信数据完整性验证和协议帧校验。
 */
#ifndef CRC64ECMA_H
#define CRC64ECMA_H

#include <QObject>
#include <QByteArray>

/**
 * @class Crc64Ecma
 * @brief CRC-64/ECMA-182校验 — 查找表加速
 *
 * 典型用法:
 * @code
 *   Crc64Ecma crc;
 *   quint64 val = crc.compute(data);
 *   crc.update(0x42);
 *   crc.reset();
 * @endcode
 */
class Crc64Ecma : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalComputations = 0;  ///< 总计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit Crc64Ecma(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~Crc64Ecma() override;

    // ── 核心接口 ──

    /**
     * @brief 一次性计算CRC-64/ECMA-182
     * @param data 输入数据
     * @return 64位CRC校验值
     */
    quint64 compute(const QByteArray& data);

    /**
     * @brief 流式追加单字节到CRC计算
     * @param byte 输入字节
     */
    void update(quint8 byte);

    /**
     * @brief 重置内部CRC状态
     */
    void reset();

    /**
     * @brief 获取当前CRC值(不改变内部状态)
     * @return 当前64位CRC值
     */
    quint64 currentValue() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 @param crc CRC-64结果值 */
    void computationCompleted(quint64 crc);

private:
    /** @brief ECMA-182多项式 */
    static constexpr quint64 POLY = 0x42F0E1EBA9EA3693ULL;

    /** @brief 查找表[256] */
    quint64 m_table[256] = {};

    /** @brief 当前CRC状态 */
    quint64 m_crc = 0;

    /** @brief 生成查找表 */
    void buildTable();

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // CRC64ECMA_H
