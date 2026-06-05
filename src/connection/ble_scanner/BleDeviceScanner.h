/**
 * @file BleDeviceScanner.h
 * @brief BLE设备扫描器 — 扫描周围蓝牙低功耗设备并维护发现列表
 *
 * 职责: 启动/停止BLE扫描，维护已发现设备缓存，跟踪RSSI历史，
 * 设备按地址去重，服务UUID分类，扫描间隔控制。
 */
#ifndef BLEDEVICESCANNER_H
#define BLEDEVICESCANNER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QMap>
#include <QList>

#include "connection/ble_scanner/BleScanTypes.h"

/**
 * @brief BLE设备扫描器
 *
 * 封装BLE设备发现流程。模拟模式下通过定时器生成演示设备；
 * 真实硬件接入后替换为QBluetoothDeviceDiscoveryAgent。
 * 发现结果以BleDeviceInfo信号上报，支持RSSI历史跟踪。
 */
class BleDeviceScanner : public QObject {
    Q_OBJECT

public:
    /** @brief 构造BLE设备扫描器 @param parent 父QObject指针 */
    explicit BleDeviceScanner(QObject* parent = nullptr);
    /** @brief 析构，停止扫描并释放定时器 */
    ~BleDeviceScanner() override;

    // ---- 扫描控制 ----
    /** @brief 开始扫描BLE设备 */
    void startScan();
    /** @brief 停止正在进行的扫描 */
    void stopScan();
    /** @brief 是否正在扫描 @return true=扫描进行中 */
    bool isScanning() const;
    /** @brief 设置扫描超时 @param ms 超时毫秒数 */
    void setScanTimeout(int ms);
    /** @brief 设置模拟发现间隔 @param ms 间隔毫秒数 */
    void setDiscoveryInterval(int ms);

    // ---- 设备查询 ----
    /** @brief 获取所有已发现设备列表 @return BleDeviceInfo列表 */
    QList<BleDeviceInfo> discoveredDevices() const;
    /** @brief 按地址查找设备 @param address BLE设备MAC地址 @return 设备信息 */
    BleDeviceInfo deviceByAddress(const QString& address) const;
    /** @brief 获取已发现设备数量 @return 设备数量 */
    int deviceCount() const;
    /** @brief 获取指定地址的RSSI历史 @param address 设备地址 @return RSSI采样列表 */
    QList<int> rssiHistory(const QString& address) const;

    // ---- 统计接口 ----
    /** @brief 获取累计启动扫描次数 */
    quint64 totalScanStarts() const;
    /** @brief 获取累计扫描完成次数 */
    quint64 totalScanCycles() const;
    /** @brief 获取累计发现事件次数(含RSSI更新) */
    quint64 totalDiscoveryEvents() const;
    /** @brief 获取累计去重设备地址总数 */
    quint64 uniqueDevicesSeen() const;
    /** @brief 获取累计扫描总时长(ms) */
    quint64 totalScanDurationMs() const;
    /** @brief 获取累计停止扫描次数 */
    quint64 totalScanStops() const;
    /** @brief 获取最佳RSSI值(dBm) */
    int bestRssi() const;
    /** @brief 获取最差RSSI值(dBm) */
    int worstRssi() const;
    /** @brief 获取平均RSSI值(dBm) */
    double averageRssi() const;
    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    void deviceFound(const BleDeviceInfo& info);   ///< 发现新设备
    void deviceUpdated(const BleDeviceInfo& info); ///< 设备信息更新(RSSI变化)
    void scanComplete();                            ///< 扫描结束

private slots:
    void onScanTimeout();        ///< 扫描超时处理
    void onSimulateDiscovery();  ///< 模拟发现单个设备

private:
    void generateSimulatedDevices();                    ///< 生成模拟BLE设备队列
    void addOrUpdateDevice(const BleDeviceInfo& info);  ///< 添加或更新设备(去重)
    BleDeviceType classifyByServices(const QStringList& uuids) const; ///< 根据服务UUID分类

    QTimer*       m_scanTimer;       ///< 扫描超时定时器
    QTimer*       m_discoveryTimer;  ///< 模拟发现间隔定时器
    QElapsedTimer m_elapsed;         ///< 扫描经过时间计量器

    QMap<QString, BleDeviceInfo> m_deviceCache;    ///< 按地址缓存的设备信息
    QMap<QString, QList<int>>    m_rssiHistoryMap;  ///< 按地址索引的RSSI历史
    QList<BleDeviceInfo>         m_simQueue;        ///< 模拟设备队列
    int                          m_simIndex = 0;    ///< 当前模拟弹出索引

    int m_scanTimeoutMs = 10000;     ///< 扫描超时(ms)
    int m_discoveryIntervalMs = 600; ///< 发现间隔(ms)

    // ---- 统计计数器 ----
    quint64 m_totalScanStarts = 0;
    quint64 m_totalScanCycles = 0;
    quint64 m_totalDiscoveryEvents = 0;
    quint64 m_totalScanDurationMs = 0;
    quint64 m_totalScanStops = 0;
    int     m_bestRssi = 0;
    int     m_worstRssi = 0;
    qint64  m_rssiSum = 0;
    int     m_rssiSampleCount = 0;
};

#endif // BLEDEVICESCANNER_H
