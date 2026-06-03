/**
 * @file IDevice.h
 * @brief 设备抽象接口 - 外部设备的统一管理协议
 *
 * 定义了设备的连接、断开、查询接口，用于串口设备、BLE 外设、
 * USB 设备、CAN 节点等所有外部设备的统一管理。
 *
 * 零出站依赖: 仅依赖 Qt Core 类型，不 include 任何项目头文件
 *
 * 设计模式:
 *   - 策略模式: 不同设备类型作为可互换的策略
 *   - 外观模式: 简化底层设备操作的统一接口
 *
 * 协作关系:
 *   - DeviceRegistry: 持有所有 IDevice 实例，管理设备生命周期
 *   - DeviceProfilePanel: 显示 IDevice 的详细信息
 *   - OtaManager: 通过 IDevice 执行固件升级
 */
#ifndef INTERFACES_IDEVICE_H
#define INTERFACES_IDEVICE_H

#include <QString>
#include <QVariant>

/**
 * @brief 设备抽象接口 - 外部设备的统一管理协议
 *
 * 每种设备类型提供一个 IDevice 实现，DeviceRegistry
 * 统一管理所有设备的发现、连接和状态查询。
 */
class IDevice {
public:
    virtual ~IDevice() = default;

    /**
     * @brief 连接到设备
     * @param address 设备地址 (如 "COM3", "192.168.1.1", BLE MAC)
     * @return 是否连接成功
     */
    virtual bool connect(const QString& address) = 0;

    /** @brief 断开设备连接 */
    virtual void disconnect() = 0;

    /** @brief 查询设备是否已连接 */
    virtual bool isConnected() const = 0;

    /**
     * @brief 获取设备信息摘要
     * @return 设备描述 (如 "STM32F407 @ COM3 (115200bps)")
     */
    virtual QString deviceInfo() const = 0;

    /** @brief 获取设备唯一标识符 (如序列号、MAC 地址) */
    virtual QString deviceId() const = 0;

    /** @brief 获取设备类型名称 (如 "STM32", "ESP32", "J-Link") */
    virtual QString deviceType() const = 0;

    /**
     * @brief 获取设备详细属性
     * @return 属性键值对 (如 { "vendor": "STMicroelectronics", "chip": "STM32F407VG" })
     */
    virtual QVariantMap deviceProperties() const = 0;
};

#endif // INTERFACES_IDEVICE_H
