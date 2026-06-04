/**
 * @file UsbConnection.h
 * @brief USB连接实现 — 通过libusb与USB设备通信
 *
 * 实现IConnection接口，提供USB设备的打开/关闭/读写操作，
 * 支持Bulk/Interrupt/Control三种传输模式。
 * 通过UsbLibraryLoader动态加载libusb，无需编译时链接。
 */
#ifndef USB_CONNECTION_H
#define USB_CONNECTION_H

#include "connection/interface/IConnection.h"
#include "connection/usb/UsbLibraryLoader.h"

/** @brief USB连接类，通过USB接口与设备通信，支持Bulk/Interrupt/Control传输 */
class UsbConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造USB连接 @param parent 父对象 */
    explicit UsbConnection(QObject* parent = nullptr);
    /** @brief 析构，关闭连接并释放资源 */
    ~UsbConnection() override;

    // ---- IConnection 接口实现 ----
    ConnectionType type() const override;              ///< 返回连接类型(USB)
    QString name() const override;                     ///< 返回连接显示名称
    ConnectionState state() const override;            ///< 返回当前连接状态
    bool open() override;                              ///< 打开USB设备连接
    void close() override;                             ///< 关闭USB设备连接
    qint64 write(const QByteArray& data) override;     ///< 发送数据，返回实际写入字节数
    void configure(const QVariantMap& params) override; ///< 配置USB参数(vid/pid/interface等)

    // ---- USB专用接口 ----

    bool setDevice(quint16 vid, quint16 pid);   ///< 设置目标USB设备(厂商ID/产品ID)
    bool claimInterface(int interface);          ///< 声明USB接口
    void releaseInterface(int interface);        ///< 释放USB接口
    QByteArray bulkTransfer(int endpoint, const QByteArray& data); ///< Bulk传输，endpoint bit7决定方向
    QByteArray interruptTransfer(int endpoint, const QByteArray& data); ///< Interrupt传输
    QByteArray controlTransfer(quint8 requestType, quint8 request,  ///< Control传输
                               quint16 value, quint16 index, const QByteArray& data);
    UsbDeviceDescriptor readDeviceDescriptor();  ///< 读取USB设备描述符(需先打开连接)
    QString readStringDescriptor(quint8 descIndex); ///< 读取USB字符串描述符(iManufacturer/iProduct/iSerialNumber)
    QVariantMap deviceDescriptorSummary();       ///< 获取设备描述符摘要(bcdUSB/class/vid/pid/manufacturer/product/serial)
    bool detachKernelDriverIfNeeded(int interfaceNum); ///< 检查并分离内核驱动(Linux专用，Windows无操作)

    // ---- 统计信息接口 ----

    quint64 totalTransfers() const { return m_totalTransfers; }       ///< 总传输次数
    quint64 totalBytesSent() const { return m_totalBytesSent; }       ///< 总发送字节数
    quint64 totalBytesReceived() const { return m_totalBytesReceived; } ///< 总接收字节数
    quint64 errorCount() const { return m_errorCount; }               ///< 错误计数
    quint64 bulkTransferCount() const { return m_bulkTransferCount; } ///< Bulk传输次数
    quint64 interruptTransferCount() const { return m_interruptTransferCount; } ///< Interrupt传输次数
    quint64 controlTransferCount() const { return m_controlTransferCount; } ///< Control传输次数
    quint64 kernelDetachCount() const { return m_kernelDetachCount; } ///< 内核驱动分离次数(Linux)
    void resetStats(); ///< 重置所有统计计数器

private:
    quint16 m_vid       = 0;    ///< 厂商ID
    quint16 m_pid       = 0;    ///< 产品ID
    int     m_interface = 0;    ///< 当前接口编号
    ConnectionState m_state = ConnectionState::Disconnected; ///< 连接状态
    void*   m_devHandle = nullptr;      ///< libusb设备句柄 (不透明指针)
    void*   m_usbContext = nullptr;     ///< libusb上下文 (不透明指针)
    bool    m_interfaceClaimed = false; ///< 接口是否已声明
    bool    m_kernelDriverDetached = false; ///< 内核驱动是否已分离
    unsigned int m_timeout = 5000;      ///< USB传输超时(ms)

    // ---- 统计计数器 ----
    quint64 m_totalTransfers = 0;       ///< 总传输次数
    quint64 m_totalBytesSent = 0;       ///< 总发送字节数
    quint64 m_totalBytesReceived = 0;   ///< 总接收字节数
    quint64 m_errorCount = 0;           ///< 错误计数
    quint64 m_bulkTransferCount = 0;    ///< Bulk传输次数
    quint64 m_interruptTransferCount = 0; ///< Interrupt传输次数
    quint64 m_controlTransferCount = 0; ///< Control传输次数
    quint64 m_kernelDetachCount = 0;    ///< 内核驱动分离次数
};

#endif // USB_CONNECTION_H
