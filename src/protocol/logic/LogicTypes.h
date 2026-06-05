/**
 * @file LogicTypes.h
 * @brief Logic Analyzer 核心类型定义 -- 采样数据、通道配置、触发条件、协议帧
 *
 * 提供 Logic Analyzer 模块使用的所有公共数据结构:
 *   - LogicLevel: 数字信号电平枚举
 *   - ProtocolType: 支持解码的协议类型
 *   - LogicSample: 单次采样点(时间戳 + 通道掩码)
 *   - LogicChannelConfig: 通道配置(名称/颜色/阈值)
 *   - LogicTrigger: 触发条件(通道/电平/位置)
 *   - ProtocolFrame: 解码后的协议帧
 */
#ifndef LOGIC_TYPES_H
#define LOGIC_TYPES_H

#include <QByteArray>
#include <QColor>
#include <QString>
#include <QtGlobal>

/**
 * @brief 数字信号电平枚举
 *
 * 表示单通道在某一时刻的逻辑电平状态。
 */
enum class LogicLevel : quint8 {
    Low       = 0,  ///< 低电平
    High      = 1,  ///< 高电平
    Undefined = 2   ///< 未定义/高阻态
};

/**
 * @brief 协议解码类型枚举
 *
 * 标识 ProtocolDecoder 支持的协议类型。
 */
enum class ProtocolType : quint8 {
    UART     = 0,  ///< 通用异步收发器
    SPI      = 1,  ///< 串行外设接口
    I2C      = 2,  ///< 内部集成电路总线
    CAN      = 3,  ///< 控制器局域网
    OneWire  = 4,  ///< 单总线协议
    Custom   = 5   ///< 自定义协议
};

/**
 * @brief 单次数字采样点
 *
 * 记录某一时刻所有通道的电平状态。timestamp 单位为纳秒(ns)，
 * channelMask 为位掩码，bit[i] 表示第 i 个通道的电平。
 */
struct LogicSample {
    quint64 timestamp   = 0;  ///< 采样时间戳(纳秒)
    quint8  channelMask = 0;  ///< 通道电平位掩码(bit0=CH0, bit1=CH1, ...)
};

/**
 * @brief 逻辑通道配置
 *
 * 定义单个数字通道的显示名称、索引、颜色、使能状态和比较器阈值。
 */
struct LogicChannelConfig {
    QString name         = QString();   ///< 通道显示名称
    quint8  channelIndex = 0;           ///< 通道物理索引
    QColor  color        = Qt::green;   ///< 波形显示颜色
    bool    enabled      = true;        ///< 是否启用采集
    double  threshold    = 1.5;         ///< 比较器阈值电压(V)，高于此值判为 High
};

/**
 * @brief 触发条件
 *
 * 定义采样的触发条件: 当指定通道达到指定电平时触发。
 * position 表示触发点在预触发缓冲区中的相对位置(0=缓冲区起始)。
 */
struct LogicTrigger {
    int         channelIndex = 0;            ///< 触发通道索引
    LogicLevel  level        = LogicLevel::High;  ///< 触发电平
    quint64     position     = 0;            ///< 触发位置(样本数偏移)
};

/**
 * @brief 解码后的协议帧
 *
 * 包含一个完整协议帧的时间范围、帧类型标识、原始数据和人类可读解码文本。
 */
struct ProtocolFrame {
    quint64    startTime    = 0;        ///< 帧起始时间戳(纳秒)
    quint64    endTime      = 0;        ///< 帧结束时间戳(纳秒)
    QString    type         = QString(); ///< 帧类型描述(如 "UART Data", "I2C Read")
    QByteArray data;                    ///< 原始数据负载
    QString    decodedText  = QString(); ///< 人类可读解码文本
};

#endif // LOGIC_TYPES_H
