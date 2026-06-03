/**
 * @file BleScanner.h
 * @brief BLE设备扫描器 — 扫描周围蓝牙低功耗设备
 *
 * 职责: 启动/停止BLE设备扫描，维护已发现设备列表，
 * 支持按名称/RSSI/地址过滤，提供RSSI跟踪和设备名解析。
 * 通过信号通知上层新设备的发现和扫描完成事件。
 */
#ifndef BLESCANNER_H
#define BLESCANNER_H

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QStringList>
#include <QMap>
#include <QElapsedTimer>

/**
 * @brief BLE设备扫描器
 * 封装BLE设备发现流程，支持限时扫描和过滤器。
 * 发现的设备以QVariantMap形式上报，包含name/address/rssi/type/manufacturerData等字段。
 */
class BleScanner : public QObject {
    Q_OBJECT

public:
    /// 扫描过滤器配置，用于筛选扫描结果
    struct ScanFilter {
        QString namePrefix;       ///< 设备名称前缀过滤(空=不过滤)
        QString addressFilter;    ///< 地址子串过滤(空=不过滤)
        int minRssi = -128;       ///< 最低RSSI阈值(dBm)
        bool hideUnnamed = false; ///< 是否隐藏无名设备
    };

    explicit BleScanner(QObject* parent = nullptr);
    ~BleScanner() override;

    // ---- 扫描控制 ----
    void startScan();                     ///< 开始扫描BLE设备
    void stopScan();                      ///< 停止正在进行的扫描
    void setFilter(const ScanFilter& f);  ///< 设置扫描过滤器
    ScanFilter filter() const;            ///< 获取当前过滤器

    // ---- 设备查询 ----
    QVariantList discoveredDevices() const;              ///< 已发现的所有设备列表
    QVariantMap deviceByAddress(const QString& addr) const; ///< 按地址查找设备
    bool isScanning() const;                             ///< 是否正在扫描
    qint64 scanElapsedTime() const;                      ///< 当前扫描已持续时间(ms)

    // ---- 统计接口 ----
    int scanCount() const;               ///< 已完成的扫描次数
    int totalDevicesFound() const;       ///< 累计发现设备总数(去重)
    quint64 filteredDeviceCount() const; ///< 被过滤器过滤掉的设备数
    quint64 totalScanStarts() const;     ///< 累计启动扫描次数
    quint64 totalDiscoveryEvents() const;///< 累计发现事件次数(含RSSI更新)
    quint64 totalScanDurationMs() const; ///< 累计扫描总时长(ms)
    int bestRssi() const;                ///< 最佳RSSI值
    int worstRssi() const;               ///< 最差RSSI值
    double averageRssi() const;          ///< 平均RSSI值
    void clearHistory();                 ///< 清空扫描历史
    void resetScannerStatistics();       ///< 重置所有统计计数器

signals:
    void deviceFound(const QVariantMap& device);          ///< 发现新设备
    void deviceRssiUpdated(const QString& addr, int rssi);///< 设备RSSI更新
    void scanFinished();                                   ///< 扫描结束
    void scanStateChanged(bool scanning);                 ///< 扫描状态变化
    void deviceFiltered(const QVariantMap& dev, const QString& reason); ///< 设备被过滤

private slots:
    void onScanTimeout();       ///< 扫描超时处理
    void onSimulateDiscovery(); ///< 模拟发现单个设备(含RSSI波动)

private:
    void generateSimulatedDevices();                          ///< 生成模拟BLE设备列表
    bool passesFilter(const QVariantMap& device) const;       ///< 检查设备是否通过过滤器
    bool updateDeviceList(const QVariantMap& device);         ///< 更新或添加设备(返回是否新设备)
    QString resolveDeviceName(const QByteArray& mfrData) const; ///< 解析厂商数据中的名称

    // ---- 成员变量 ----
    QTimer*        m_scanTimer;            ///< 扫描超时定时器
    QTimer*        m_discoveryTimer;       ///< 模拟发现间隔定时器
    QVariantList   m_devices;              ///< 已发现的设备列表
    QVariantList   m_simQueue;             ///< 模拟设备队列
    int            m_simIndex = 0;         ///< 当前模拟弹出索引
    ScanFilter     m_filter;               ///< 当前扫描过滤器
    int            m_scanCount = 0;        ///< 已完成扫描次数
    QStringList    m_seenAddresses;        ///< 已发现设备地址集合(去重)
    QMap<QString, QVariantMap> m_deviceByAddr; ///< 按地址索引的设备信息
    QMap<QString, QList<int>>  m_rssiHistory;  ///< RSSI历史记录
    QElapsedTimer  m_scanElapsed;          ///< 扫描经过时间计量器

    quint64 m_totalScanStarts = 0;         ///< 累计启动扫描次数
    quint64 m_totalDiscoveryEvents = 0;    ///< 累计发现事件次数
    quint64 m_filteredDeviceCount = 0;     ///< 被过滤设备数
    quint64 m_totalScanDurationMs = 0;     ///< 累计扫描总时长
    int     m_bestRssi = 0;                ///< 最佳RSSI值
    int     m_worstRssi = 0;               ///< 最差RSSI值
    qint64  m_rssiSum = 0;                 ///< RSSI累计总和
    int     m_rssiSampleCount = 0;         ///< RSSI采样点数
};

#endif // BLESCANNER_H
