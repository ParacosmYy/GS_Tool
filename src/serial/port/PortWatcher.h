/** @file PortWatcher.h @brief 串口热插拔检测器 — QTimer轮询QSerialPortInfo检测端口变化，携带设备详情
 * 增强: 完整设备信息(PortDeviceInfo)/端口事件统计/设备变更日志/防抖机制
 * 协作: SerialConfigPanel(刷新端口) / MainWindow(状态栏提示) / SerialDetector(芯片识别) */

#ifndef PORTWATCHER_H
#define PORTWATCHER_H

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <QMap>
#include <QVector>
#include <QDateTime>
#include <QElapsedTimer>
#include <QSerialPortInfo>

/** @brief 端口设备详细信息 — 热插拔事件携带的设备描述 */
struct PortDeviceInfo {
    QString portName; QString description; QString manufacturer; QString serialNumber;
    quint16 vendorId = 0; quint16 productId = 0; QString vidHex; QString pidHex;
    bool isEmpty() const { return portName.isEmpty(); }          ///< 端口名为空则为true
    QString toShortSummary() const;                              ///< 单行摘要如"COM3 - CH340 [1A86:7523]"
};

/** @brief 端口事件记录 — 记录一次热插拔事件 */
struct PortEventRecord {
    enum class EventType { Arrival, Removal };
    EventType type; QDateTime timestamp; PortDeviceInfo device; int totalPortsAfterEvent = 0;
    QString toLogString() const;                                 ///< 日志文本如"[14:32:05] +COM3 CH340"
};

/** @brief 串口热插拔检测器(增强版) — 定时轮询QSerialPortInfo检测变化，携带设备详情信号 */
class PortWatcher : public QObject {
    Q_OBJECT

public:
    explicit PortWatcher(QObject* parent = nullptr);            ///< 构造串口热插拔检测器
    ~PortWatcher() override;                                   ///< 停止定时器，子对象自动销毁
    PortWatcher(const PortWatcher&) = delete;                  ///< 禁止拷贝
    PortWatcher& operator=(const PortWatcher&) = delete;       ///< 禁止赋值

    void start();                                               ///< 启动检测(建立基准快照+定时轮询)
    void stop();                                                ///< 停止检测
    bool isRunning() const;                                     ///< 检测是否运行中
    int interval() const;                                       ///< 获取轮询间隔(ms)，默认2000
    void setInterval(int msec);                                 ///< 设置轮询间隔(仅停止时生效)

    QStringList currentPorts() const;                           ///< 当前已知端口名称列表
    QVector<PortDeviceInfo> currentDevices() const;             ///< 当前所有设备详细信息
    PortDeviceInfo deviceInfo(const QString& portName) const;   ///< 按端口名查询设备详情

    quint64 totalArrivals() const;                              ///< 累计端口新增次数
    quint64 totalRemovals() const;                              ///< 累计端口移除次数
    quint64 totalPolls() const;                                 ///< 累计轮询次数
    quint64 totalChanges() const;                               ///< 累计变化次数(新增+移除)
    quint64 totalPortScans() const;                             ///< 累计端口扫描次数
    quint64 totalHotplugEvents() const;                         ///< 累计热插拔事件次数
    quint64 totalScanErrors() const;                            ///< 累计端口扫描错误次数
    qint64 msSinceLastChange() const;                           ///< 最近变更距现在毫秒数，无事件返回-1
    int onlineDeviceCount() const;                              ///< 当前在线设备数

    QVector<PortEventRecord> recentEvents(int maxCount = 0) const; ///< 最近事件记录(时间倒序)
    int eventLogCapacity() const;                               ///< 事件日志最大条数
    void setEventLogCapacity(int capacity);                     ///< 设置日志容量(最小10)
    void resetWatcherStatistics();                              ///< 重置所有统计和日志

signals:
    void deviceAdded(const PortDeviceInfo& device);            ///< 新串口设备接入(携带设备详情)
    void deviceRemoved(const PortDeviceInfo& device);          ///< 串口设备移除(携带设备详情)
    void devicesChanged(const QVector<PortDeviceInfo>& devs);  ///< 端口列表变化
    void portAdded(const QString& portName);                   ///< 兼容: 新端口(仅端口名)
    void portRemoved(const QString& portName);                 ///< 兼容: 端口移除(仅端口名)
    void portsChanged();                                        ///< 兼容: 端口列表变化

private slots:
    void onTimeout();                                           ///< 定时轮询回调

private:
    QMap<QString, PortDeviceInfo> queryAvailableDevices();     ///< 查询系统可用串口(按端口名排序)
    static PortDeviceInfo fromQtInfo(const QSerialPortInfo& info); ///< QSerialPortInfo→PortDeviceInfo
    void appendEventLog(const PortEventRecord& event);         ///< 追加事件日志(超容量移除旧记录)

    QTimer* m_timer;
    QMap<QString, PortDeviceInfo> m_currentDevices;            ///< 当前已知设备(端口名→信息)

    // ---- 统计 ----
    quint64 m_totalArrivals = 0; quint64 m_totalRemovals = 0; quint64 m_totalPolls = 0;
    quint64 m_totalChanges = 0; quint64 m_totalPortScans = 0; quint64 m_totalHotplugEvents = 0;
    quint64 m_totalScanErrors = 0;

    // ---- 日志 ----
    QVector<PortEventRecord> m_eventLog; int m_eventLogCapacity = 100;
    QElapsedTimer m_lastChangeTimer; bool m_hasLastChange = false;

    // ---- 防抖(连续kDebounceThreshold次确认才发射信号) ----
    QMap<QString, int> m_addedCandidateCount;
    QMap<QString, int> m_removedCandidateCount;
    QMap<QString, PortDeviceInfo> m_pendingAddDevices;
    static constexpr int kDebounceThreshold = 2;               ///< 连续2次确认(约4秒)
};

#endif // PORTWATCHER_H
