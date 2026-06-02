/**
 * @file BleConnection.h
 * @brief BLE连接实现 — 适配器模式，封装蓝牙BLE到IConnection接口
 *
 * 职责: BLE设备连接/断开、GATT服务发现、特征值读写，
 * 通过IConnection统一接口供上层使用。
 */
#ifndef BLECONNECTION_H
#define BLECONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTimer>

/**
 * @brief BLE连接实现
 *
 * 封装BLE底层通信，实现IConnection统一接口。
 * 额外提供GATT服务发现和特征值操作接口。
 */
class BleConnection : public IConnection {
    Q_OBJECT

public:
    /**
     * @brief 构造BLE连接
     * @param parent 父对象
     */
    explicit BleConnection(QObject* parent = nullptr);

    /** @brief 析构函数，自动断开连接 */
    ~BleConnection() override;

    // ---- IConnection 接口实现 ----

    /** @brief 返回连接类型 */
    ConnectionType type() const override;

    /** @brief 返回BLE设备地址作为连接名称 */
    QString name() const override;

    /** @brief 返回当前连接状态 */
    ConnectionState state() const override;

    /** @brief 打开BLE连接 */
    bool open() override;

    /** @brief 关闭BLE连接 */
    void close() override;

    /**
     * @brief 写入数据到BLE特征值
     * @param data 待发送数据
     * @return 实际写入字节数，-1表示失败
     */
    qint64 write(const QByteArray& data) override;

    /**
     * @brief 通过参数映射配置BLE连接
     * @param params 支持的key: address, serviceUuid
     */
    void configure(const QVariantMap& params) override;

    // ---- BLE专用接口 ----

    /**
     * @brief 连接到指定地址的BLE设备
     * @param address BLE设备地址(如 "00:11:22:33:44:55")
     */
    void connectToDevice(const QString& address);

    /** @brief 断开当前BLE设备连接 */
    void disconnectDevice();

    /**
     * @brief 发现已连接设备的GATT服务
     * @return 服务UUID列表
     */
    QStringList discoverServices();

signals:
    /** @brief GATT服务发现完成 */
    void servicesDiscovered(const QStringList& services);

    /**
     * @brief 特征值读取完成
     * @param characteristicUuid 特征UUID
     * @param value 读取到的数据
     */
    void characteristicRead(const QString& characteristicUuid, const QByteArray& value);

private:
    /** @brief 目标BLE设备地址 */
    QString m_deviceAddress;

    /** @brief 当前连接状态 */
    ConnectionState m_state = ConnectionState::Disconnected;

    /** @brief 已发现的GATT服务列表 */
    QStringList m_services;
};

#endif // BLECONNECTION_H
