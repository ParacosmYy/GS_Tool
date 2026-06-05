/**
 * @file SerialFrameDecoder.h
 * @brief 通用串口帧解码器 — 按用户自定义帧结构解析/组装/校验二进制帧
 * @author Serial Tool Team
 * @date 2026-06-05
 *
 * 支持 uint8~double/bool/bytes/string 共 10 种字段类型，
 * 大端/小端字节序选择，帧头/帧尾/校验和完整验证。
 */

#ifndef SERIALFRAMEDECODER_H
#define SERIALFRAMEDECODER_H

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QtGlobal>

/**
 * @class SerialFrameDecoder
 * @brief 通用串口帧解码器
 *
 * 提供按用户自定义帧结构（帧头、字段列表、校验和、帧尾）解析/组装/校验
 * 二进制帧的能力，支持 10 种字段类型和大端/小端字节序。
 */
class SerialFrameDecoder : public QObject
{
    Q_OBJECT

public:
    /** @brief 字段类型枚举 */
    enum class FieldType {
        UInt8,    ///< 无符号 8 位整数
        UInt16,   ///< 无符号 16 位整数
        UInt32,   ///< 无符号 32 位整数
        Int8,     ///< 有符号 8 位整数
        Int16,    ///< 有符号 16 位整数
        Int32,    ///< 有符号 32 位整数
        Float32,  ///< 32 位浮点 (IEEE 754)
        Float64,  ///< 64 位双精度浮点 (IEEE 754)
        Bool,     ///< 布尔值 (1 字节, 0/非0)
        Bytes     ///< 原始字节/字符串
    };
    Q_ENUM(FieldType)

    /** @brief 字节序枚举 */
    enum class Endianness {
        BigEndian,     ///< 大端序 (网络字节序)
        LittleEndian   ///< 小端序 (x86)
    };
    Q_ENUM(Endianness)

    /** @brief 校验和类型枚举 */
    enum class ChecksumType {
        None,   ///< 无校验
        Sum8,   ///< 8 位累加和
        Xor8,   ///< 8 位异或
        Crc8,   ///< CRC-8
        Crc16,  ///< CRC-16/MODBUS
        Crc32   ///< CRC-32
    };
    Q_ENUM(ChecksumType)

    /** @brief 字段定义结构体 */
    struct FieldDef {
        QString name;             ///< 字段名称
        FieldType type;           ///< 字段数据类型
        int offset;               ///< 字段在帧内的字节偏移
        int length;               ///< 字段长度(字节数)，仅 Bytes 类型可 > 固定长度
    };

    /** @brief 帧定义结构体 */
    struct FrameDef {
        QByteArray header;               ///< 帧头字节序列
        QByteArray footer;               ///< 帧尾字节序列 (可为空)
        QList<FieldDef> fields;          ///< 字段定义列表
        ChecksumType checksumType;       ///< 校验和类型
        int checksumOffset;              ///< 校验和在帧内的偏移 (-1 表示自动计算)
        int checksumLength;              ///< 校验和长度 (字节)
        Endianness endianness;           ///< 字节序
    };

    /** @brief 操作统计结构体 */
    struct Stats {
        quint64 totalFramesDecoded = 0;            ///< 解码帧总数
        quint64 totalFramesBuilt = 0;              ///< 组装帧总数
        quint64 totalDecodeErrors = 0;             ///< 解码错误总数
        quint64 totalChecksumErrors = 0;           ///< 校验和错误总数
        quint64 framesByFieldType[10] = {};        ///< 按字段类型统计解码次数，索引对应 FieldType 枚举
        double avgFrameSize = 0.0;                 ///< 平均帧大小 (字节)
    };

    /** @brief 构造通用帧解码器 @param parent 父对象 */
    explicit SerialFrameDecoder(QObject *parent = nullptr);

    // ---- 帧定义管理 ----

    /**
     * @brief 设置当前帧定义
     * @param def 帧定义结构体
     */
    void setFrameDefinition(const FrameDef &def);

    /**
     * @brief 从 QVariantMap 加载帧定义
     * @param map 包含 header/footer/fields/checksumType/checksumOffset/checksumLength/endianness 键
     * @return 解析后的 FrameDef；格式错误时字段使用默认值
     */
    FrameDef frameDefFromVariantMap(const QVariantMap &map) const;

    /** @brief 获取当前帧定义 @return 当前帧定义的只读引用 */
    const FrameDef &frameDefinition() const;

    // ---- 帧解析 (Decode) ----

    /**
     * @brief 解码一帧原始字节，提取所有字段
     * @param data 原始帧数据(至少包含帧头+字段+校验+帧尾)
     * @return QVariantMap，键为字段名，值为字段值；失败返回空 Map
     */
    QVariantMap decodeFrame(const QByteArray &data);

    // ---- 帧组装 (Build) ----

    /**
     * @brief 根据字段值和帧定义组装完整帧
     * @param fieldValues 字段名→字段值的映射
     * @return 组装后的原始字节帧；失败返回空 QByteArray
     */
    QByteArray buildFrame(const QVariantMap &fieldValues);

    // ---- 帧校验 (Validate) ----

    /**
     * @brief 校验帧头、帧尾、校验和
     * @param data 待校验的完整帧数据
     * @return true 表示帧合法
     */
    bool validateFrame(const QByteArray &data) const;

    // ---- 辅助接口 ----

    /** @brief 根据帧定义计算最小帧长度 (帧头+所有字段+校验+帧尾) @return 最小字节数 */
    int minimumFrameLength() const;

    /** @brief 获取字段类型的固定字节长度 @param type 字段类型 @return 字节数，Bytes 类型返回 0 */
    static int fixedFieldSize(FieldType type);

    // ---- 统计接口 ----

    /** @brief 获取操作统计快照 @return 当前统计数据的只读引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 帧解码完成信号 @param fields 解析出的字段键值对 */
    void frameDecoded(const QVariantMap &fields);

    /** @brief 帧组装完成信号 @param frameData 组装后的原始字节 */
    void frameBuilt(const QByteArray &frameData);

    /** @brief 解码错误信号 @param errorMessage 中文错误描述 */
    void decodeError(const QString &errorMessage);

private:
    // ---- 内部解析辅助 ----

    /** @brief 从字节数组提取单个字段值 @param data 完整帧数据 @param field 字段定义 @return 字段值 */
    QVariant extractFieldValue(const QByteArray &data, const FieldDef &field) const;

    /** @brief 将字段值写入字节数组的指定位置 @param buf 目标缓冲区 @param field 字段定义 @param value 字段值 */
    void injectFieldValue(QByteArray &buf, const FieldDef &field, const QVariant &value) const;

    /** @brief 计算指定数据的校验和 @param data 待校验数据 @param type 校验和类型 @param length 校验和字节长度 @return 校验和字节序列 */
    QByteArray computeChecksum(const QByteArray &data, ChecksumType type, int length) const;

    /** @brief 获取校验和覆盖的数据范围 @param frameSize 完整帧大小 @return 待校验的数据区间 [start, end) */
    QPair<int, int> checksumRange(int frameSize) const;

    FrameDef m_frameDef;   ///< 当前帧定义
    mutable Stats m_stats; ///< 操作统计(可变，允许const方法内更新)
    quint64 m_totalFrameSizeForAvg = 0;  ///< 用于计算平均帧大小的累计字节数
};

#endif // SERIALFRAMEDECODER_H
