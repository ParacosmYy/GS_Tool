/**
 * @file UsbDeviceDetector.h
 * @brief USB设备检测器 — 扫描和监控USB设备插拔事件
 *
 * 通过双通道枚举USB设备:
 * 1. WMIC/SetupAPI方式(Windows) — 不依赖libusb，始终可用
 * 2. libusb方式 — 需要libusb动态库，可获取制造商/产品/序列号等详细信息
 * 监听设备插拔事件，支持定时轮询检测。
 */
#ifndef USB_DEVICE_DETECTOR_H
#define USB_DEVICE_DETECTOR_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QTimer>
#include <QElapsedTimer>
#include "connection/usb/UsbLibraryLoader.h"

/**
 * @brief USB设备检测器
 * 扫描系统USB设备列表，监听设备插入和移除事件。
 * 优先使用libusb获取详细设备信息，不可用时回退到WMIC。
 */
class UsbDeviceDetector : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造USB设备检测器
     * @param parent 父对象
     */
    explicit UsbDeviceDetector(QObject* parent = nullptr);

    /**
     * @brief 扫描当前所有USB设备
     * 优先使用libusb扫描(可获取字符串描述符)，libusb不可用时回退到WMIC
     * @return 设备信息列表 [{vid, pid, vidHex, pidHex, name, manufacturer, serial, bcdUSB, deviceClass}]
     */
    QVariantList scanDevices();

    /**
     * @brief 使用libusb扫描设备并获取完整描述符信息
     * 读取每个设备的设备描述符和字符串描述符(制造商/产品/序列号)
     * @return 设备信息列表
     */
    QVariantList scanDevicesViaLibusb();

    /**
     * @brief 使用WMIC命令扫描设备(不依赖libusb)
     * @return 设备信息列表
     */
    QVariantList scanDevicesViaWmic();

    /**
     * @brief 获取指定设备的详细信息
     * @param vid 厂商ID
     * @param pid 产品ID
     * @return 设备详细信息映射
     */
    QVariantMap deviceDetails(quint16 vid, quint16 pid) const;

    /**
     * @brief 通过libusb获取设备的字符串描述符
     * 需要先打开设备，读取制造商/产品/序列号字符串
     * @param vid 厂商ID
     * @param pid 产品ID
     * @return 包含manufacturer/product/serial字段的映射
     */
    QVariantMap fetchStringDescriptors(quint16 vid, quint16 pid);

    /** @brief 开始监听USB设备变化 */
    void startMonitoring(int intervalMs = 1000);

    /** @brief 停止监听 */
    void stopMonitoring();

    /** @brief 检查libusb是否可用 */
    bool isLibusbAvailable() const;

    /** @brief 获取累计检测周期次数 @return 检测周期总次数 */
    quint64 totalDetectionCycles() const;

    /** @brief 获取累计检测到设备次数 @return 设备检测总次数 */
    quint64 totalDevicesDetected() const;

    /** @brief 获取累计设备插入事件次数 @return 插入事件总次数 */
    quint64 totalAttachEvents() const;

    /** @brief 获取累计设备拔出事件次数 @return 拔出事件总次数 */
    quint64 totalDetachEvents() const;

    /** @brief 获取libusb扫描调用次数 @return libusb扫描总次数 */
    quint64 totalLibusbScans() const;

    /** @brief 获取WMIC扫描调用次数 @return WMIC扫描总次数 */
    quint64 totalWmicScans() const;

    /** @brief 获取平均扫描耗时(ms) @return 平均每次扫描耗时 */
    double avgScanTimeMs() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 检测到新设备插入 */
    void deviceInserted(const QVariantMap& deviceInfo);

    /** @brief 检测到设备移除 */
    void deviceRemoved(const QVariantMap& deviceInfo);

    /** @brief 扫描完成，携带设备列表和使用的扫描方式 */
    void scanCompleted(const QVariantList& devices, const QString& method);

private slots:
    /** @brief 定时轮询检查设备变化 */
    void onPollTimeout();

private:
    /** @brief 比较设备列表差异 */
    void detectChanges(const QVariantList& newList);

    /**
     * @brief 将libusb设备描述符转换为QVariantMap
     * @param vid 厂商ID
     * @param pid 产品ID
     * @param desc 设备描述符结构体
     * @param manufacturer 制造商字符串(可能为空)
     * @param product 产品字符串(可能为空)
     * @param serial 序列号字符串(可能为空)
     * @return 设备信息映射
     */
    QVariantMap descriptorToMap(quint16 vid, quint16 pid,
                                const UsbDeviceDescriptor& desc,
                                const QString& manufacturer,
                                const QString& product,
                                const QString& serial);

    QVariantList m_devices;       ///< 当前设备列表
    QTimer*      m_pollTimer;     ///< 轮询定时器
    bool         m_libusbAvailable = false; ///< libusb是否可用

    // ---- 统计计数器 ----
    quint64 m_totalDetectionCycles = 0;        ///< 累计检测周期次数
    quint64 m_totalDevicesDetected = 0;        ///< 累计检测到设备次数
    quint64 m_totalAttachEvents = 0;           ///< 累计设备插入事件次数
    quint64 m_totalDetachEvents = 0;           ///< 累计设备拔出事件次数
    quint64 m_totalLibusbScans = 0;            ///< libusb扫描调用次数
    quint64 m_totalWmicScans = 0;              ///< WMIC扫描调用次数
    qint64  m_totalScanTimeMs = 0;             ///< 累计扫描总耗时(ms)
    quint64 m_totalScanCount = 0;              ///< 累计扫描完成次数
};

#endif // USB_DEVICE_DETECTOR_H
