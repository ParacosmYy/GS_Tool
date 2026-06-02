/**
 * @file BleScanner.h
 * @brief BLE设备扫描器 — 扫描周围蓝牙低功耗设备
 *
 * 职责: 启动/停止BLE设备扫描，维护已发现设备列表，
 * 通过信号通知上层新设备的发现和扫描完成事件。
 */
#ifndef BLESCANNER_H
#define BLESCANNER_H

#include <QObject>
#include <QTimer>
#include <QVariantList>

/**
 * @brief BLE设备扫描器
 *
 * 封装BLE设备发现流程，支持限时扫描。
 * 发现的设备以QVariantMap形式上报，包含name/address/rssi等字段。
 */
class BleScanner : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造BLE扫描器
     * @param parent 父对象
     */
    explicit BleScanner(QObject* parent = nullptr);

    /** @brief 析构函数，停止扫描并清理资源 */
    ~BleScanner() override;

    /** @brief 开始扫描BLE设备 */
    void startScan();

    /** @brief 停止正在进行的扫描 */
    void stopScan();

    /**
     * @brief 获取已发现的所有设备列表
     * @return QVariantList，每项为QVariantMap(name/address/rssi)
     */
    QVariantList discoveredDevices() const;

signals:
    /**
     * @brief 发现新设备时发出
     * @param device 设备信息(name/address/rssi)
     */
    void deviceFound(const QVariantMap& device);

    /** @brief 扫描结束时发出 */
    void scanFinished();

private slots:
    /** @brief 扫描超时处理 */
    void onScanTimeout();

private:
    /** @brief 扫描超时定时器 */
    QTimer* m_scanTimer;

    /** @brief 已发现的设备列表 */
    QVariantList m_devices;
};

#endif // BLESCANNER_H
