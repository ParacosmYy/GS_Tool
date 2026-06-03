/**
 * @file UsbConnection.h
 * @brief USB连接实现 — 通过libusb与USB设备通信
 *
 * 实现IConnection接口，提供USB设备的打开/关闭/读写操作，
 * 支持Bulk/Interrupt/Control三种传输模式。
 */
#ifndef USB_CONNECTION_H
#define USB_CONNECTION_H

#include "connection/interface/IConnection.h"

/**
 * @brief USB连接类
 * 通过USB接口与设备通信，实现IConnection统一接口。
 * 支持Bulk传输、Interrupt传输和Control传输。
 */
class UsbConnection : public IConnection {
    Q_OBJECT

public:
    explicit UsbConnection(QObject* parent = nullptr);
    ~UsbConnection() override;

    // ---- IConnection 接口实现 ----
    ConnectionType type() const override;
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    void configure(const QVariantMap& params) override;

    // ---- USB专用接口 ----

    /**
     * @brief 设置目标USB设备
     * @param vid 厂商ID
     * @param pid 产品ID
     * @return 是否找到并设置设备
     */
    bool setDevice(quint16 vid, quint16 pid);

    /**
     * @brief 声明USB接口
     * @param interface 接口编号
     * @return 是否成功声明
     */
    bool claimInterface(int interface);

    /**
     * @brief 释放USB接口
     * @param interface 接口编号
     */
    void releaseInterface(int interface);

    /**
     * @brief Bulk传输
     * @param endpoint 端点地址
     * @param data 发送数据
     * @return 接收到的数据
     */
    QByteArray bulkTransfer(int endpoint, const QByteArray& data);

    /**
     * @brief Interrupt传输
     * @param endpoint 端点地址
     * @param data 发送数据
     * @return 接收到的数据
     */
    QByteArray interruptTransfer(int endpoint, const QByteArray& data);

    /**
     * @brief Control传输
     * @param requestType 请求类型
     * @param request 请求代码
     * @param value 值字段
     * @param index 索引字段
     * @param data 数据负载
     * @return 接收到的数据
     */
    QByteArray controlTransfer(quint8 requestType, quint8 request,
                               quint16 value, quint16 index,
                               const QByteArray& data);

private:
    quint16 m_vid       = 0;    ///< 厂商ID
    quint16 m_pid       = 0;    ///< 产品ID
    int     m_interface = 0;    ///< 当前接口编号
    ConnectionState m_state = ConnectionState::Disconnected; ///< 连接状态
    void*   m_devHandle = nullptr;      ///< libusb设备句柄 (不透明指针)
    void*   m_usbContext = nullptr;     ///< libusb上下文 (不透明指针)
    bool    m_interfaceClaimed = false; ///< 接口是否已声明
    unsigned int m_timeout = 5000;      ///< USB传输超时(ms)
};

#endif // USB_CONNECTION_H
