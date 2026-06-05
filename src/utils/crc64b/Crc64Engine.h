/**
 * @file Crc64Engine.h
 * @brief CRC-64校验引擎 — 支持ECMA-182与WE多项式的流式校验
 *
 * 提供CRC-64/ECMA-182和CRC-64/WE两种多项式的校验计算,
 * 支持流式接口和查找表加速, 适用于嵌入式通信数据完整性验证。
 */
#ifndef CRC64ENGINE_H
#define CRC64ENGINE_H

#include <QObject>
#include <QByteArray>

/**
 * @class Crc64Engine
 * @brief CRC-64校验引擎 — ECMA-182 / WE 双多项式
 *
 * 典型用法:
 * @code
 *   Crc64Engine engine(Crc64Engine::Polynomial::ECMA182);
 *   engine.update(data1);
 *   engine.update(data2);
 *   quint64 crc = engine.finalValue();
 * @endcode
 */
class Crc64Engine : public QObject {
    Q_OBJECT

public:
    /** @brief 支持的CRC-64多项式类型 */
    enum class Polynomial {
        ECMA182 = 0,  ///< CRC-64/ECMA-182 (0x42F0E1EBA9EA3693)
        WE            ///< CRC-64/WE (不同初始值/输出异或)
    };
    Q_ENUM(Polynomial)

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalChecksums = 0;       ///< 校验计算总次数
        quint64 totalBytes = 0;           ///< 处理字节总数
        double  avgTime = 0.0;            ///< 平均耗时(us)
    };

    /** @brief 构造函数 @param poly 多项式类型 @param parent 父对象 */
    explicit Crc64Engine(Polynomial poly = Polynomial::ECMA182,
                         QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~Crc64Engine() override;

    // ── 流式接口 ──

    /** @brief 重置内部CRC状态, 开始新的校验计算 */
    void reset();

    /**
     * @brief 追加数据到CRC计算
     * @param data 输入数据
     */
    void update(const QByteArray& data);

    /**
     * @brief 追加单字节到CRC计算
     * @param byte 输入字节
     */
    void update(quint8 byte);

    /**
     * @brief 获取当前CRC-64值
     * @return 64位CRC校验值
     */
    quint64 finalValue();

    // ── 便捷接口 ──

    /**
     * @brief 一次性计算CRC-64(不改变内部状态)
     * @param data 输入数据
     * @return 64位CRC校验值
     */
    quint64 checksum(const QByteArray& data);

    // ── 查询 ──

    /** @brief 获取当前多项式类型 */
    Polynomial polynomial() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 校验完成信号 @param crc CRC-64结果值 */
    void checksumReady(quint64 crc);
    /** @brief 错误信号 @param errorMessage 错误描述 */
    void error(const QString& errorMessage);

private:
    /** @brief 生成查找表 */
    void buildLookupTable();

    /** @brief CRC-64多项式 */
    Polynomial m_poly;

    /** @brief 多项式系数 */
    quint64 m_polynomial = 0;

    /** @brief 查找表[256] */
    quint64 m_table[256] = {};

    /** @brief 当前CRC状态 */
    quint64 m_crc = 0;

    /** @brief 是否已调用finalValue(需要reset才能继续) */
    bool m_finalized = false;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // CRC64ENGINE_H
