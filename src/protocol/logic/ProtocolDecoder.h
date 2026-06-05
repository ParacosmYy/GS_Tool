/**
 * @file ProtocolDecoder.h
 * @brief 协议解码引擎 -- 将数字采样信号解码为协议帧
 *
 * ProtocolDecoder 负责:
 *   - 协议类型选择(UART/SPI/I2C/CAN/OneWire/Custom)
 *   - 每协议独立配置(波特率/数据位/通道映射等)
 *   - 从LogicSample序列中提取协议帧
 *   - 解码错误检测与统计
 *
 * 使用方式:
 *   1. setProtocol() 选择协议类型
 *   2. setXxxConfig() 配置协议参数
 *   3. decode() 传入采样数据，返回解码帧列表
 *   4. frameDecoded 信号实时推送每一帧
 */
#ifndef PROTOCOL_DECODER_H
#define PROTOCOL_DECODER_H

#include <QObject>
#include <QVariantMap>
#include "protocol/logic/LogicTypes.h"

/**
 * @brief 协议解码引擎
 *
 * 支持 UART/SPI/I2C 三种常见协议的实时解码。
 * 每次调用 decode() 处理一批采样数据，返回解码后的 ProtocolFrame 列表。
 */
class ProtocolDecoder : public QObject {
    Q_OBJECT

public:
    /** @brief 构造协议解码引擎 @param parent 父QObject指针 */
    explicit ProtocolDecoder(QObject* parent = nullptr);

    /** @brief 设置目标协议类型 @param type 协议枚举值 */
    void setProtocol(ProtocolType type);

    /** @brief 获取当前协议类型 @return 协议枚举值 */
    ProtocolType protocol() const;

    /**
     * @brief 通过通用Map配置协议参数
     * @param config 键值对配置(协议相关字段)
     *
     * UART: baudRate, dataBits, stopBits, parity
     * SPI:  clockChannel, mosiChannel, misoChannel, csChannel, cpol, cpha
     * I2C:  sdaChannel, sclChannel
     */
    void configureProtocol(const QVariantMap& config);

    /**
     * @brief 解码一批采样数据
     * @param samples 输入采样序列(来自LogicSampler)
     * @return 解码后的协议帧列表
     */
    QVector<ProtocolFrame> decode(const QVector<LogicSample>& samples);

    /** @brief 重置解码器内部状态(不清除统计) */
    void reset();

    // ── 协议专用配置 ──

    /**
     * @brief 配置UART解码参数
     * @param baudRate 波特率(bps)
     * @param dataBits 数据位(5~9)
     * @param stopBits 停止位(1或2)
     * @param parity 校验("None"/"Even"/"Odd")
     */
    void setUartConfig(int baudRate, int dataBits, int stopBits,
                       const QString& parity);

    /**
     * @brief 配置SPI解码参数
     * @param clockChannel 时钟通道索引
     * @param mosiChannel  主出从入数据通道
     * @param misoChannel  主入从出数据通道
     * @param csChannel    片选通道
     * @param cpol         时钟极性(0/1)
     * @param cpha         时钟相位(0/1)
     */
    void setSpiConfig(int clockChannel, int mosiChannel, int misoChannel,
                      int csChannel, bool cpol, bool cpha);

    /**
     * @brief 配置I2C解码参数
     * @param sdaChannel 数据线通道索引
     * @param sclChannel 时钟线通道索引
     */
    void setI2cConfig(int sdaChannel, int sclChannel);

    // ── 统计信息 ──
    /** @brief 获取累计解码帧数 */
    quint64 totalFramesDecoded() const;
    /** @brief 获取累计解码字节数 */
    quint64 totalBytesDecoded() const;
    /** @brief 获取累计解码错误数 */
    quint64 totalErrors() const;
    /** @brief 获取累计协议切换次数 */
    quint64 totalProtocolsSwitched() const;
    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 单帧解码完成 @param frame 解码后的协议帧 */
    void frameDecoded(const ProtocolFrame& frame);

    /** @brief 解码错误 @param error 错误描述 */
    void decodeError(const QString& error);

private:
    /** @brief UART协议解码 @param samples 采样数据 @return 解码帧列表 */
    QVector<ProtocolFrame> decodeUart(const QVector<LogicSample>& samples);

    /** @brief SPI协议解码 @param samples 采样数据 @return 解码帧列表 */
    QVector<ProtocolFrame> decodeSpi(const QVector<LogicSample>& samples);

    /** @brief I2C协议解码 @param samples 采样数据 @return 解码帧列表 */
    QVector<ProtocolFrame> decodeI2c(const QVector<LogicSample>& samples);

    /** @brief 从采样序列中提取指定通道的电平序列 @param samples 采样数据 @param channel 通道索引 @return 电平序列(true=High) */
    QVector<bool> extractChannel(const QVector<LogicSample>& samples,
                                  int channel) const;

    /** @brief 二分查找时间戳最接近targetNs的样本索引 @param samples 采样数据 @param targetNs 目标时间戳(ns) @return 样本索引，未找到返回-1 */
    static int findSampleAtTime(const QVector<LogicSample>& samples,
                                 quint64 targetNs);

    // ── 协议配置 ──
    ProtocolType m_protocol = ProtocolType::UART;  ///< 当前协议类型
    // UART参数
    int     m_uartBaudRate = 9600;   ///< 波特率
    int     m_uartDataBits = 8;      ///< 数据位
    int     m_uartStopBits = 1;      ///< 停止位
    QString m_uartParity   = QStringLiteral("None"); ///< 校验
    int     m_uartChannel  = 0;      ///< UART数据通道索引
    // SPI参数
    int  m_spiClockChannel = 0;      ///< SPI时钟通道
    int  m_spiMosiChannel  = 1;      ///< MOSI通道
    int  m_spiMisoChannel  = 2;      ///< MISO通道
    int  m_spiCsChannel    = 3;      ///< CS通道
    bool m_spiCpol         = false;  ///< 时钟极性
    bool m_spiCpha         = false;  ///< 时钟相位
    // I2C参数
    int m_i2cSdaChannel = 0;         ///< SDA通道
    int m_i2cSclChannel = 1;         ///< SCL通道
    // ── 统计计数器 ──
    quint64 m_totalFramesDecoded     = 0;  ///< 累计解码帧数
    quint64 m_totalBytesDecoded      = 0;  ///< 累计解码字节数
    quint64 m_totalErrors            = 0;  ///< 累计解码错误数
    quint64 m_totalProtocolsSwitched = 0;  ///< 累计协议切换次数
};

#endif // PROTOCOL_DECODER_H
