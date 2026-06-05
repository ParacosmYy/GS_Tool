/**
 * @file UsbConnection.h
 * @brief USB连接实现 — 通过libusb动态加载与USB设备通信
 *
 * 实现IConnection接口，提供USB设备的打开/关闭/读写操作，
 * 支持Bulk/Interrupt/Control三种USB传输模式。
 *
 * 依赖:
 *   - UsbLibraryLoader: 动态加载libusb共享库，无需编译时链接
 *   - IConnection: 统一连接接口
 *
 * 传输模式:
 *   - Bulk传输: 默认端点0x01，适合大批量数据
 *   - Interrupt传输: 适合小包实时数据(HID等)
 *   - Control传输: 控制请求(EP0)，requestType bit7决定方向
 *
 * 安全约束:
 *   - write(data): 空数据直接返回0，不发起USB传输
 *   - controlTransfer: 数据长度限制在65535字节内防止截断
 *   - 返回值通过qBound限制在合法范围内
 *
 * 统计: 传输次数/字节数/错误/分类型计数/内核分离/重置/打开次数
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

    /** @brief 设置目标USB设备(厂商ID/产品ID) @param vid 厂商ID @param pid 产品ID @return true=设置成功 */
    bool setDevice(quint16 vid, quint16 pid);
    /** @brief 声明USB接口 @param interface 接口编号 @return true=声明成功 */
    bool claimInterface(int interface);
    /** @brief 释放USB接口 @param interface 接口编号 */
    void releaseInterface(int interface);
    /** @brief Bulk传输，endpoint bit7决定方向 @param endpoint 端点地址 @param data 发送数据 @return 接收到的数据 */
    QByteArray bulkTransfer(int endpoint, const QByteArray& data);
    /** @brief Interrupt传输 @param endpoint 端点地址 @param data 发送数据 @return 接收到的数据 */
    QByteArray interruptTransfer(int endpoint, const QByteArray& data);
    /** @brief Control传输 @param requestType 请求类型 @param request 请求码 @param value 值字段 @param index 索引字段 @param data 数据缓冲 @return 接收到的数据 */
    QByteArray controlTransfer(quint8 requestType, quint8 request,
                               quint16 value, quint16 index, const QByteArray& data);
    /** @brief 读取USB设备描述符(需先打开连接) @return 设备描述符结构体 */
    UsbDeviceDescriptor readDeviceDescriptor();
    /** @brief 读取USB字符串描述符 @param descIndex 描述符索引(iManufacturer/iProduct/iSerialNumber) @return 字符串内容 */
    QString readStringDescriptor(quint8 descIndex);
    /** @brief 获取设备描述符摘要 @return 包含bcdUSB/class/vid/pid/manufacturer/product/serial的映射 */
    QVariantMap deviceDescriptorSummary();
    /** @brief 检查并分离内核驱动(Linux专用，Windows无操作) @param interfaceNum 接口编号 @return true=分离成功或无需分离 */
    bool detachKernelDriverIfNeeded(int interfaceNum);

    // ---- 统计信息接口 ----

    quint64 totalTransfers() const { return m_totalTransfers; }       ///< @return 总传输次数
    quint64 totalBytesSent() const { return m_totalBytesSent; }       ///< @return 总发送字节数
    quint64 totalBytesReceived() const { return m_totalBytesReceived; } ///< @return 总接收字节数
    quint64 errorCount() const { return m_errorCount; }               ///< @return 错误计数
    quint64 bulkTransferCount() const { return m_bulkTransferCount; } ///< @return Bulk传输次数
    quint64 interruptTransferCount() const { return m_interruptTransferCount; } ///< @return Interrupt传输次数
    quint64 controlTransferCount() const { return m_controlTransferCount; } ///< @return Control传输次数
    quint64 kernelDetachCount() const { return m_kernelDetachCount; } ///< @return 内核驱动分离次数(Linux)
    quint64 totalDeviceResets() const { return m_totalDeviceResets; } ///< @return USB设备重置次数
    quint64 totalOpenAttempts() const { return m_totalOpenAttempts; } ///< @return 累计open()调用次数
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
    quint64 m_totalDeviceResets = 0;    ///< USB设备重置次数
    quint64 m_totalOpenAttempts = 0;    ///< 累计open()调用次数
};

#endif // USB_CONNECTION_H
