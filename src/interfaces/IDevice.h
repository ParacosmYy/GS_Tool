/**
 * @file IDevice.h
 * @brief 设备抽象接口 — 统一描述外设设备的发现、连接、属性查询
 *
 * 所有外设(串口/BLE/CAN/USB/SPI-I2C)通过此接口向上层提供统一的设备描述。
 * ConnectionFactory/DeviceManager/ConnectionQuickDialog依赖此接口列举设备。
 * 层级: L0 纯虚接口层
 */
#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QStringList>

/** @brief 设备类型枚举 — 涵盖EmbedDebug支持的所有外设分类 */
enum class DeviceType {
    SerialPort,     ///< 串口设备(COM/ttyUSB/ttyACM)
    Bluetooth,      ///< BLE蓝牙设备
    Network,        ///< 网络设备(TCP/UDP/WS)
    CanBus,         ///< CAN总线适配器
    UsbRaw,         ///< USB原始设备(libusb)
    SpiI2c,         ///< SPI/I2C桥接设备
    Rtt,            ///< J-Link RTT调试接口
    Unknown         ///< 未知设备类型
};

/** @brief 设备状态枚举 — 描述设备在发现/连接生命周期中的状态 */
enum class DeviceState {
    Offline,        ///< 离线(未检测到/已断开)
    Available,      ///< 可用(已检测到，可连接)
    Connected,      ///< 已连接(正在通信)
    Error           ///< 错误状态(连接失败/设备异常)
};

/**
 * @brief 设备抽象接口 — 统一的设备描述契约
 *
 * 提供设备发现、属性查询、状态监控的抽象接口。
 * 协作: SerialDetector/UsbDeviceDetector(发现) / ConnectionQuickDialog(选择) / ConnectionFactory(创建)
 */
class IDevice : public QObject {
    Q_OBJECT
public:
    explicit IDevice(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IDevice() = default;
    virtual DeviceType deviceType() const = 0;       ///< 获取设备类型
    virtual QString deviceId() const = 0;             ///< 获取唯一标识(COM3/MAC地址)
    virtual QString displayName() const = 0;          ///< 获取显示名称
    virtual QString description() const = 0;          ///< 获取描述(制造商/型号)
    virtual DeviceState deviceState() const = 0;      ///< 获取当前状态
    virtual bool isAvailable() const = 0;             ///< 是否可用于连接
    virtual QVariantMap properties() const = 0;       ///< 获取属性映射
    virtual QVariant property(const QString& key, const QVariant& defaultValue = QVariant()) const = 0; ///< 获取指定属性
    virtual bool supportsCapability(const QString& cap) const = 0; ///< 是否支持指定功能
    virtual QStringList capabilities() const = 0;     ///< 获取所有能力标识
signals:
    void deviceStateChanged(DeviceState newState);    ///< 设备状态变化
    void propertyChanged(const QString& key);         ///< 属性变化
};

Q_DECLARE_METATYPE(DeviceType)
Q_DECLARE_METATYPE(DeviceState)
#endif // IDEVICE_H
