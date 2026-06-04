/**
 * @file BleConnection.h
 * @brief BLE连接实现 — 适配器模式，封装蓝牙BLE到IConnection接口
 *
 * 职责: BLE设备连接/断开、GATT服务发现、特征值读写，
 * 通过IConnection统一接口供上层使用。
 * 当前为模拟实现，真实BLE集成需链接Qt Bluetooth模块。
 */
#ifndef BLECONNECTION_H
#define BLECONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTimer>

/**
 * @brief BLE连接实现
 *
 * 封装BLE底层通信，实现IConnection统一接口。
 * 模拟连接状态转换和数据回环，GATT服务返回预设列表。
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

    /** @brief 返回连接类型（当前返回Serial，待枚举扩展） */
    ConnectionType type() const override;

    /** @brief 返回BLE设备地址作为连接名称 */
    QString name() const override;

    /** @brief 返回当前连接状态 */
    ConnectionState state() const override;

    /** @brief 打开BLE连接（模拟异步连接过程） */
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
     * @param params 支持的key: address, deviceName
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

    /**
     * @brief 获取设备名称
     * @return 设备名称，未设置返回空串
     */
    QString deviceName() const;

    // ---- 统计信息接口 ----

    /** @brief 获取总扫描次数 */
    quint64 totalScans() const { return m_totalScans; }

    /** @brief 获取总连接次数 */
    quint64 totalConnections() const { return m_totalConnections; }

    /** @brief 获取总断开次数 */
    quint64 totalDisconnections() const { return m_totalDisconnections; }

    /** @brief 获取总发现GATT服务次数 */
    quint64 totalServicesDiscovered() const { return m_totalServicesDiscovered; }

    /** @brief 获取总特征读取次数 */
    quint64 totalCharacteristicsRead() const { return m_totalCharacteristicsRead; }

    /** @brief 获取总写入次数 */
    quint64 totalWrites() const { return m_totalWrites; }

    /** @brief 获取总读取次数 */
    quint64 totalReads() const { return m_totalReads; }

    /** @brief 获取总发送字节数 */
    quint64 totalBytesSent() const { return m_totalBytesWritten; }

    /** @brief 获取总接收字节数 */
    quint64 totalBytesReceived() const { return m_totalBytesRead; }

    /** @brief 获取总写入字节数(旧接口兼容) */
    quint64 totalBytesWritten() const { return m_totalBytesWritten; }

    /** @brief 获取总读取字节数(旧接口兼容) */
    quint64 totalBytesRead() const { return m_totalBytesRead; }

    /** @brief 获取错误计数 */
    quint64 errorCount() const { return m_errorCount; }

    /** @brief 重置所有统计计数器 */
    void resetStats();

signals:
    /** @brief GATT服务发现完成 */
    void servicesDiscovered(const QStringList& services);

    /**
     * @brief 特征值读取完成
     * @param characteristicUuid 特征UUID
     * @param value 读取到的数据
     */
    void characteristicRead(const QString& characteristicUuid,
                            const QByteArray& value);

private slots:
    /** @brief 模拟连接建立完成 */
    void onConnectTimeout();

private:
    /** @brief 初始化模拟GATT服务列表 */
    void initMockServices();

    /** @brief 目标BLE设备地址 */
    QString m_deviceAddress;

    /** @brief 目标BLE设备名称 */
    QString m_deviceName;

    /** @brief 当前连接状态 */
    ConnectionState m_state = ConnectionState::Disconnected;

    /** @brief 已发现的GATT服务列表 */
    QStringList m_services;

    /** @brief 模拟连接延迟定时器 */
    QTimer* m_connectTimer;

    /** @brief 写入数据累计字节计数(旧接口保留) */
    qint64 m_bytesWritten = 0;

    // ---- 统计计数器 ----
    quint64 m_totalScans = 0;                   ///< 总扫描次数
    quint64 m_totalConnections = 0;             ///< 总连接次数
    quint64 m_totalDisconnections = 0;          ///< 总断开次数
    quint64 m_totalServicesDiscovered = 0;      ///< 总发现GATT服务次数
    quint64 m_totalCharacteristicsRead = 0;     ///< 总特征读取次数
    quint64 m_totalWrites = 0;                  ///< 总写入次数
    quint64 m_totalReads = 0;                   ///< 总读取次数
    quint64 m_totalBytesWritten = 0;            ///< 总写入字节数
    quint64 m_totalBytesRead = 0;               ///< 总读取字节数
    quint64 m_errorCount = 0;                   ///< 错误计数
};

#endif // BLECONNECTION_H
