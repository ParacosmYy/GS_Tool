/**
 * @file SpiI2cBridgeTypes.h
 * @brief SPI/I2C桥接类型定义 — 总线类型、配置结构体和传输结构体
 *
 * 职责:
 *   1. 定义BusType枚举(SPI/I2C)
 *   2. 定义SpiConfig/I2cConfig配置参数结构体
 *   3. 定义BusTransfer统一传输请求结构体
 *
 * 协作: SpiI2cBridge(桥接引擎) / SpiI2cBridgeWidget(UI面板)
 */
#ifndef SPII2CBRIDGETYPES_H
#define SPII2CBRIDGETYPES_H

#include <QByteArray>
#include <QtGlobal>

/** @brief 总线类型枚举 */
enum class BusType { SPI, I2C };

/** @brief SPI总线配置参数 */
struct SpiConfig {
    quint32 clockRate = 1000000;     ///< 时钟频率(Hz)，默认1MHz
    quint8  mode      = 0;           ///< SPI模式(0-3: CPOL/CPHA组合)
    bool    csActiveLow = true;      ///< CS低电平有效
    quint8  bitsPerWord = 8;         ///< 每字比特数(8/16/32)
    bool    lsbFirst  = false;       ///< true=LSB优先，false=MSB优先
};

/** @brief I2C总线配置参数 */
struct I2cConfig {
    quint32 clockRate     = 100000;  ///< 时钟频率(Hz)，默认100kHz(标准模式)
    quint8  deviceAddress = 0x00;    ///< 设备7位地址(0x00-0x7F)
    bool    tenBitAddress = false;   ///< true=10位地址模式
};

/** @brief 统一总线传输请求结构体 */
struct BusTransfer {
    BusType   type       = BusType::SPI;  ///< 总线类型
    QByteArray txData;                     ///< 发送数据缓冲区
    QByteArray rxData;                     ///< 接收数据缓冲区(传输完成后填充)
    quint32    timeoutMs = 100;            ///< 传输超时时间(ms)
};

#endif // SPII2CBRIDGETYPES_H
