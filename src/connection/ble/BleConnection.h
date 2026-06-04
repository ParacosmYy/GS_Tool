/** @file BleConnection.h @brief BLE连接实现 -- 适配器模式封装蓝牙BLE到IConnection接口。当前为模拟实现，真实BLE需链接Qt Bluetooth模块 */
#ifndef BLECONNECTION_H
#define BLECONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTimer>

/** @brief BLE连接实现。封装BLE底层通信，模拟连接状态转换和数据回环 */
class BleConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造BLE连接 @param parent 父对象 */
    explicit BleConnection(QObject* parent = nullptr);
    /** @brief 析构(自动断开连接) */
    ~BleConnection() override;
    // ---- IConnection接口 ----
    ConnectionType type() const override;    ///< @return 连接类型
    QString name() const override;           ///< @return BLE设备地址
    ConnectionState state() const override;  ///< @return 当前连接状态
    bool open() override;                    ///< @return true=成功发起连接
    void close() override;                   ///< 关闭BLE连接
    qint64 write(const QByteArray& data) override; ///< @return 实际写入字节数，-1=失败
    void configure(const QVariantMap& params) override; ///< @param params 配置(address/deviceName)
    // ---- BLE专用 ----
    /** @brief 连接到指定地址的BLE设备 @param address BLE设备地址 */
    void connectToDevice(const QString& address);
    /** @brief 断开当前BLE设备 */
    void disconnectDevice();
    /** @brief 发现GATT服务 @return 已发现的服务UUID列表 */
    QStringList discoverServices();
    /** @brief 获取设备名称 @return 当前连接设备的名称 */
    QString deviceName() const;

    // ---- 统计信息 ----
    quint64 totalScans() const { return m_totalScans; } ///< 总扫描次数
    quint64 totalConnections() const { return m_totalConnections; } ///< 总连接次数
    quint64 totalDisconnections() const { return m_totalDisconnections; } ///< 总断开次数
    quint64 totalServicesDiscovered() const { return m_totalServicesDiscovered; } ///< 总发现GATT服务次数
    quint64 totalCharacteristicsRead() const { return m_totalCharacteristicsRead; } ///< 总特征读取次数
    quint64 totalWrites() const { return m_totalWrites; } ///< 总写入次数
    quint64 totalReads() const { return m_totalReads; } ///< 总读取次数
    quint64 totalCharacteristicWrites() const { return m_totalCharacteristicWrites; } ///< 总特征值写入次数
    quint64 totalCharacteristicReads() const { return m_totalCharacteristicReads; } ///< 总特征值读取次数
    quint64 totalNotifications() const { return m_totalNotifications; } ///< 总BLE通知接收次数
    quint64 totalErrors() const { return m_errorCount; } ///< 总错误次数(别名)
    quint64 totalBytesSent() const { return m_totalBytesWritten; } ///< 总发送字节数
    quint64 totalBytesReceived() const { return m_totalBytesRead; } ///< 总接收字节数
    quint64 totalBytesWritten() const { return m_totalBytesWritten; } ///< 总写入字节数(旧接口)
    quint64 totalBytesRead() const { return m_totalBytesRead; } ///< 总读取字节数(旧接口)
    quint64 errorCount() const { return m_errorCount; } ///< 错误计数
    void resetStats();                       ///< 重置所有统计计数器

signals:
    /** @brief GATT服务发现完成 @param services 已发现的服务UUID列表 */
    void servicesDiscovered(const QStringList& services);
    /** @brief 特征值读取完成 @param characteristicUuid 特征UUID @param value 读取到的数据 */
    void characteristicRead(const QString& characteristicUuid, const QByteArray& value);

private slots:
    /** @brief 模拟连接建立完成回调 */
    void onConnectTimeout();

private:
    /** @brief 初始化模拟GATT服务列表 */
    void initMockServices();
    QString m_deviceAddress;                 ///< 目标BLE设备地址
    QString m_deviceName;                    ///< 目标BLE设备名称
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前连接状态
    QStringList m_services;                  ///< 已发现的GATT服务列表
    QTimer* m_connectTimer;                  ///< 模拟连接延迟定时器
    qint64 m_bytesWritten = 0;               ///< 写入累计字节(旧接口)

    // ---- 统计计数器 ----
    quint64 m_totalScans = 0;                   ///< 总扫描次数
    quint64 m_totalConnections = 0;             ///< 总连接次数
    quint64 m_totalDisconnections = 0;          ///< 总断开次数
    quint64 m_totalServicesDiscovered = 0;      ///< 总发现GATT服务次数
    quint64 m_totalCharacteristicsRead = 0;     ///< 总特征读取次数
    quint64 m_totalWrites = 0;                  ///< 总写入次数
    quint64 m_totalReads = 0;                   ///< 总读取次数
    quint64 m_totalCharacteristicWrites = 0;    ///< 总特征值写入次数
    quint64 m_totalCharacteristicReads = 0;     ///< 总特征值读取次数
    quint64 m_totalNotifications = 0;           ///< 总BLE通知接收次数
    quint64 m_totalBytesWritten = 0;            ///< 总写入字节数
    quint64 m_totalBytesRead = 0;               ///< 总读取字节数
    quint64 m_errorCount = 0;                   ///< 错误计数
};

#endif // BLECONNECTION_H
