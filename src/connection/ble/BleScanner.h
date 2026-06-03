/**
 * @file BleScanner.h
 * @brief BLE设备扫描器 — 扫描周围蓝牙低功耗设备
 *
 * 职责: 启动/停止BLE设备扫描，维护已发现设备列表，
 * 通过信号通知上层新设备的发现和扫描完成事件。
 * 当前使用模拟扫描模式（无Qt Bluetooth模块依赖）。
 */
#ifndef BLESCANNER_H
#define BLESCANNER_H

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QStringList>

/**
 * @brief BLE设备扫描器
 *
 * 封装BLE设备发现流程，支持限时扫描。
 * 发现的设备以QVariantMap形式上报，包含name/address/rssi等字段。
 * 模拟模式下会生成演示设备用于开发调试。
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

    /**
     * @brief 查询扫描是否正在进行
     * @return true=正在扫描
     */
    bool isScanning() const;

    /** @brief 获取已完成的扫描次数 */
    int scanCount() const;

    /** @brief 获取累计发现的设备总数（去重后） */
    int totalDevicesFound() const;

    /** @brief 清空扫描历史记录 */
    void clearHistory();

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

    /** @brief 模拟发现单个设备（渐进式） */
    void onSimulateDiscovery();

private:
    /** @brief 生成模拟BLE设备列表 */
    void generateSimulatedDevices();

    /** @brief 扫描超时定时器 */
    QTimer* m_scanTimer;

    /** @brief 模拟发现间隔定时器 */
    QTimer* m_discoveryTimer;

    /** @brief 已发现的设备列表 */
    QVariantList m_devices;

    /** @brief 模拟设备队列（渐进式弹出） */
    QVariantList m_simQueue;

    /** @brief 当前扫描索引 */
    int m_simIndex = 0;

    /** @brief 已完成扫描次数 */
    int m_scanCount = 0;

    /** @brief 累计发现的设备地址集合（去重） */
    QStringList m_seenAddresses;
};

#endif // BLESCANNER_H
