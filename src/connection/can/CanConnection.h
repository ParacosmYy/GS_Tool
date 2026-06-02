/**
 * @file CanConnection.h
 * @brief CAN/CAN-FD连接实现 — 适配器模式，封装CAN适配器串口协议到IConnection接口
 *
 * 职责: 通过串口CAN适配器(CANable/CANtact/LAWICEL兼容)收发CAN帧，
 * 实现IConnection统一接口供上层使用。
 */
#ifndef CANCONNECTION_H
#define CANCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QByteArray>
#include <QString>

/**
 * @brief CAN/CAN-FD总线连接实现
 *
 * 封装CAN总线底层通信，通过串口CAN适配器(LAWICEL/SLCAN协议)收发帧。
 * 支持经典CAN和CAN-FD模式，可配置波特率。
 */
class CanConnection : public IConnection {
    Q_OBJECT

public:
    /**
     * @brief 构造CAN连接
     * @param parent 父对象
     */
    explicit CanConnection(QObject* parent = nullptr);

    /** @brief 析构函数，自动关闭连接 */
    ~CanConnection() override;

    // ---- IConnection 接口实现 ----

    /** @brief 返回连接类型 */
    ConnectionType type() const override;

    /** @brief 返回CAN适配器名称 */
    QString name() const override;

    /** @brief 返回当前连接状态 */
    ConnectionState state() const override;

    /** @brief 打开CAN连接(通过底层串口) */
    bool open() override;

    /** @brief 关闭CAN连接 */
    void close() override;

    /**
     * @brief 写入(发送)CAN帧数据
     * @param data 待发送数据(封装为CAN帧)
     * @return 实际写入字节数，-1表示失败
     */
    qint64 write(const QByteArray& data) override;

    /**
     * @brief 通过参数映射配置CAN连接
     * @param params 支持的key: bitrate, canFd, adapter, serialPort
     */
    void configure(const QVariantMap& params) override;

    // ---- CAN专用接口 ----

    /**
     * @brief 设置CAN总线波特率
     * @param bitrate 波特率(如 500000)
     */
    void setBitrate(int bitrate);

    /**
     * @brief 启用/禁用CAN-FD模式
     * @param enabled true=启用CAN-FD
     */
    void setCanFdEnabled(bool enabled);

    /**
     * @brief 发送CAN帧
     * @param id 帧ID
     * @param data 帧数据
     * @param extended true=扩展帧(29位ID)
     * @return true=发送成功
     */
    bool sendFrame(int id, const QByteArray& data, bool extended = false);

    /**
     * @brief 设置底层串口连接(不转移所有权)
     * @param serialPort 已配置的IConnection串口实例
     */
    void setSerialPort(IConnection* serialPort);

    /**
     * @brief 获取底层串口连接
     * @return 串口连接指针(可能为nullptr)
     */
    IConnection* serialPort() const;

signals:
    /**
     * @brief 收到CAN帧时发出
     * @param id 帧ID
     * @param data 帧数据
     * @param extended true=扩展帧
     * @param rtr true=远程帧
     */
    void frameReceived(int id, const QByteArray& data, bool extended, bool rtr);

private slots:
    /** @brief 处理底层串口收到的原始数据 */
    void onSerialDataReceived(const QByteArray& data);

private:
    /**
     * @brief 向适配器发送LAWICEL命令并等待回车
     * @param cmd LAWICEL协议命令字符串(不含\\r)
     * @return 写入字节数
     */
    qint64 sendCommand(const QString& cmd);

    /** @brief 解析串口缓冲区中的LAWICEL帧行 */
    void parseBuffer();

    /** @brief CAN总线波特率，默认500kbps */
    int m_bitrate = 500000;

    /** @brief CAN-FD模式开关 */
    bool m_canFdEnabled = false;

    /** @brief 适配器名称 */
    QString m_adapterName;

    /** @brief 当前连接状态 */
    ConnectionState m_state = ConnectionState::Disconnected;

    /** @brief 底层串口连接(不拥有所有权) */
    IConnection* m_serialPort = nullptr;

    /** @brief 串口接收缓冲区，用于按行解析LAWICEL帧 */
    QByteArray m_rxBuffer;
};

#endif // CANCONNECTION_H
