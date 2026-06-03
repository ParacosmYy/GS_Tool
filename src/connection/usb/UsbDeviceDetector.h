/**
 * @file UsbDeviceDetector.h
 * @brief USB设备检测器 — 扫描和监控USB设备插拔事件
 *
 * 枚举系统中所有USB设备，并监听设备插拔事件。
 */
#ifndef USB_DEVICE_DETECTOR_H
#define USB_DEVICE_DETECTOR_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QTimer>

/**
 * @brief USB设备检测器
 * 扫描系统USB设备列表，监听设备插入和移除事件。
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
     * @return 设备信息列表 [{vid, pid, manufacturer, product, serial}]
     */
    QVariantList scanDevices();

    /**
     * @brief 获取指定设备的详细信息
     * @param vid 厂商ID
     * @param pid 产品ID
     * @return 设备详细信息映射
     */
    QVariantMap deviceDetails(quint16 vid, quint16 pid) const;

    /** @brief 开始监听USB设备变化 */
    void startMonitoring(int intervalMs = 1000);

    /** @brief 停止监听 */
    void stopMonitoring();

    /** @brief 获取累计检测周期次数 */
    quint64 totalDetectionCycles() const;

    /** @brief 获取累计检测到设备次数 */
    quint64 totalDevicesDetected() const;

    /** @brief 获取累计设备插入事件次数 */
    quint64 totalAttachEvents() const;

    /** @brief 获取累计设备拔出事件次数 */
    quint64 totalDetachEvents() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 检测到新设备插入 */
    void deviceInserted(const QVariantMap& deviceInfo);

    /** @brief 检测到设备移除 */
    void deviceRemoved(const QVariantMap& deviceInfo);

private slots:
    /** @brief 定时轮询检查设备变化 */
    void onPollTimeout();

private:
    /** @brief 比较设备列表差异 */
    void detectChanges(const QVariantList& newList);

    QVariantList m_devices;       ///< 当前设备列表
    QTimer*      m_pollTimer;     ///< 轮询定时器

    // ---- 统计计数器 ----
    quint64 m_totalDetectionCycles = 0;        ///< 累计检测周期次数
    quint64 m_totalDevicesDetected = 0;        ///< 累计检测到设备次数
    quint64 m_totalAttachEvents = 0;           ///< 累计设备插入事件次数
    quint64 m_totalDetachEvents = 0;           ///< 累计设备拔出事件次数
};

#endif // USB_DEVICE_DETECTOR_H
