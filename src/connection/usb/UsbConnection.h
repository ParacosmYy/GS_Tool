/**
 * @file UsbConnection.h
 * @brief USB连接实现 — 通过libusb与USB设备通信
 *
 * 实现IConnection接口，提供USB设备的打开/关闭/读写操作，
 * 支持Bulk/Interrupt/Control三种传输模式。
 * 通过UsbLibraryLoader动态加载libusb，无需编译时链接。
 * 支持内核驱动自动分离(Linux)、设备描述符读取、字符串描述符查询。
 */
#ifndef USB_CONNECTION_H
#define USB_CONNECTION_H

#include "connection/interface/IConnection.h"
#include "connection/usb/UsbLibraryLoader.h"

/**
 * @brief USB连接类
 * 通过USB接口与设备通信，实现IConnection统一接口。
 * 支持Bulk传输、Interrupt传输和Control传输。
 * 内核驱动管理: Linux上自动检测并分离内核驱动，Windows无操作。
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
     * @param endpoint 端点地址(bit7决定方向: 0=OUT, 1=IN)
     * @param data 发送数据(OUT)或接收缓冲区大小(IN时data.size()为缓冲区长度)
     * @return 实际传输的数据，失败返回空QByteArray
     */
    QByteArray bulkTransfer(int endpoint, const QByteArray& data);

    /**
     * @brief Interrupt传输
     * @param endpoint 端点地址(bit7决定方向)
     * @param data 发送数据或接收缓冲区大小
     * @return 实际传输的数据
     */
    QByteArray interruptTransfer(int endpoint, const QByteArray& data);

    /**
     * @brief Control传输
     * @param requestType 请求类型(bit7决定方向)
     * @param request 请求代码
     * @param value 值字段
     * @param index 索引字段
     * @param data 数据负载
     * @return 实际传输的数据
     */
    QByteArray controlTransfer(quint8 requestType, quint8 request,
                               quint16 value, quint16 index,
                               const QByteArray& data);

    /**
     * @brief 读取USB设备描述符
     * 需要先打开连接，读取18字节标准设备描述符
     * @return 设备描述符结构体，未连接时返回空
     */
    UsbDeviceDescriptor readDeviceDescriptor();

    /**
     * @brief 读取USB字符串描述符
     * @param descIndex 字符串描述符索引(iManufacturer/iProduct/iSerialNumber)
     * @return 解码后的字符串，失败返回空
     */
    QString readStringDescriptor(quint8 descIndex);

    /**
     * @brief 获取设备描述符的摘要信息
     * 读取描述符并返回人类可读的QVariantMap
     * @return 描述符摘要 {bcdUSB, deviceClass, vid, pid, manufacturer, product, serial}
     */
    QVariantMap deviceDescriptorSummary();

    /**
     * @brief 检查并分离内核驱动(Linux专用)
     * Windows上无操作。Linux上检查指定接口是否被内核驱动占用，
     * 如果是则尝试分离以便用户态程序独占使用。
     * @param interfaceNum 接口编号
     * @return 成功分离或无需分离返回true，分离失败返回false
     */
    bool detachKernelDriverIfNeeded(int interfaceNum);

    // ---- 统计信息接口 ----

    /** @brief 获取总传输次数 */
    quint64 totalTransfers() const { return m_totalTransfers; }

    /** @brief 获取总发送字节数 */
    quint64 totalBytesSent() const { return m_totalBytesSent; }

    /** @brief 获取总接收字节数 */
    quint64 totalBytesReceived() const { return m_totalBytesReceived; }

    /** @brief 获取错误计数 */
    quint64 errorCount() const { return m_errorCount; }

    /** @brief 获取Bulk传输次数 */
    quint64 bulkTransferCount() const { return m_bulkTransferCount; }

    /** @brief 获取Interrupt传输次数 */
    quint64 interruptTransferCount() const { return m_interruptTransferCount; }

    /** @brief 获取Control传输次数 */
    quint64 controlTransferCount() const { return m_controlTransferCount; }

    /** @brief 获取内核驱动分离次数(Linux) */
    quint64 kernelDetachCount() const { return m_kernelDetachCount; }

    /** @brief 重置所有统计计数器 */
    void resetStats();

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
