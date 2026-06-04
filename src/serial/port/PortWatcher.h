/**
 * @file PortWatcher.h
 * @brief 串口热插拔检测器 - 增强版，携带设备详细信息的热插拔通知
 *
 * 设计思路:
 *   利用 QTimer 每 2 秒轮询一次 QSerialPortInfo::availablePorts()，
 *   将当前端口列表与上次快照做差异比较，检测出新增或移除的端口，
 *   并通过信号通知上层模块，携带设备详情(端口名/制造商/描述/VID/PID)。
 *
 * 增强功能:
 *   1. 热插拔信号携带完整设备信息(PortDeviceInfo)，不再仅是端口名
 *   2. 端口事件统计(插入/移除历史、最近事件时间戳)
 *   3. 设备变更日志(最近N条事件记录)
 *
 * 协作关系:
 *   - SerialConfigPanel: 监听 deviceAdded/deviceRemoved 自动刷新端口列表
 *   - MainWindow: 监听 devicesChanged 在状态栏提示用户
 *   - SerialDetector: 可组合使用，提供更丰富的芯片识别
 *
 * 所属层级: 数据层（检测系统硬件状态，不涉及 UI）
 */

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

/**
 * @brief 端口设备详细信息 - 热插拔事件携带的设备描述
 */
struct PortDeviceInfo {
    QString portName;       ///< 端口名称(如"COM3")
    QString description;    ///< 设备描述(如"USB-SERIAL CH340")
    QString manufacturer;   ///< 制造商名称(如"wch.cn")
    QString serialNumber;   ///< 设备序列号
    quint16 vendorId = 0;   ///< USB厂商ID(VID)
    quint16 productId = 0;  ///< USB产品ID(PID)
    QString vidHex;         ///< VID十六进制(如"1A86")
    QString pidHex;         ///< PID十六进制(如"7523")

    /** @brief 判断设备信息是否为空(仅端口名为空时为true) @return true=空 */
    bool isEmpty() const { return portName.isEmpty(); }

    /**
     * @brief 生成单行摘要(用于状态栏/日志)
     * @return 如"COM3 - USB-SERIAL CH340 [1A86:7523]"
     */
    QString toShortSummary() const;
};

/**
 * @brief 端口事件记录 - 记录一次热插拔事件的关键信息
 */
struct PortEventRecord {
    enum class EventType { Arrival, Removal };
    EventType type;           ///< 事件类型(插入/移除)
    QDateTime timestamp;      ///< 事件时间戳
    PortDeviceInfo device;    ///< 事件对应的设备信息
    int totalPortsAfterEvent = 0; ///< 事件发生后系统总端口数

    /** @brief 生成事件日志文本 @return 如"[14:32:05] +COM3 CH340 (1A86:7523)" */
    QString toLogString() const;
};

/**
 * @brief 串口热插拔检测器(增强版)
 *
 * 通过定时轮询 QSerialPortInfo 检测系统中串口设备的变化。
 * 当检测到端口新增或移除时，发射携带设备详细信息的信号。
 * 维护最近事件日志，支持端口事件统计查询。
 *
 * 使用方式:
 * @code
 *   auto* watcher = new PortWatcher(this);
 *   connect(watcher, &PortWatcher::deviceAdded,
 *           [](const PortDeviceInfo& dev) {
 *       qDebug() << "新设备:" << dev.toShortSummary();
 *   });
 *   watcher->start();
 * @endcode
 */
class PortWatcher : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造串口热插拔检测器
     * @param parent 父对象，用于 Qt 对象树管理生命周期
     */
    explicit PortWatcher(QObject* parent = nullptr);

    /** @brief 析构函数 — 停止定时器，子对象自动销毁 */
    ~PortWatcher() override;

    // 禁止拷贝和赋值
    PortWatcher(const PortWatcher&) = delete;
    PortWatcher& operator=(const PortWatcher&) = delete;

    // ---- 监控控制 ----

    /** @brief 启动热插拔检测 — 立即建立基准快照并开始定时轮询 */
    void start();

    /** @brief 停止热插拔检测 — 停止定时器并清空快照 */
    void stop();

    /** @brief 检测是否正在运行 */
    bool isRunning() const;

    /** @brief 获取轮询间隔（毫秒），默认 2000ms */
    int interval() const;

    /** @brief 设置轮询间隔（毫秒），仅在停止状态下生效 */
    void setInterval(int msec);

    // ---- 当前状态查询 ----

    /** @brief 获取当前已知的端口名称列表 */
    QStringList currentPorts() const;

    /** @brief 获取当前所有已知设备的详细信息 @return PortDeviceInfo列表 */
    QVector<PortDeviceInfo> currentDevices() const;

    /** @brief 按端口名查询当前设备详情 @param portName 端口名 @return 设备信息，不存在返回空 */
    PortDeviceInfo deviceInfo(const QString& portName) const;

    // ---- 端口事件统计 ----

    /** @brief 获取累计检测到的端口新增次数 */
    quint64 totalArrivals() const;
    /** @brief 获取累计检测到的端口移除次数 */
    quint64 totalRemovals() const;
    /** @brief 获取累计轮询次数 */
    quint64 totalPolls() const;
    /** @brief 获取累计检测到变化的次数（新增+移除事件合计） */
    quint64 totalChanges() const;
    /** @brief 获取累计端口扫描次数 */
    quint64 totalPortScans() const;
    /** @brief 获取累计热插拔事件次数 */
    quint64 totalHotplugEvents() const;
    /** @brief 获取累计端口扫描错误次数 @return 扫描错误总次数 */
    quint64 totalScanErrors() const;
    /** @brief 获取最近一次端口变更距现在的毫秒数，无事件返回-1 */
    qint64 msSinceLastChange() const;
    /** @brief 获取当前在线设备数 */
    int onlineDeviceCount() const;

    // ---- 事件日志 ----

    /** @brief 获取最近的事件记录列表(最多保留maxCount条) @param maxCount 最大返回条数，默认0=返回全部 @return 事件记录列表(按时间倒序) */
    QVector<PortEventRecord> recentEvents(int maxCount = 0) const;

    /** @brief 获取事件日志最大保留条数 */
    int eventLogCapacity() const;

    /** @brief 设置事件日志最大保留条数 @param capacity 容量，最小10 */
    void setEventLogCapacity(int capacity);

    /** @brief 重置所有统计计数器和事件日志 */
    void resetWatcherStatistics();

signals:
    /** @brief 检测到新串口设备接入(携带设备详情) @param device 新增设备的详细信息 */
    void deviceAdded(const PortDeviceInfo& device);
    /** @brief 检测到串口设备移除(携带设备详情) @param device 被移除设备的详细信息 */
    void deviceRemoved(const PortDeviceInfo& device);
    /** @brief 端口列表发生了变化（任何新增或移除后都会发射） @param currentDevices 变更后的设备列表 */
    void devicesChanged(const QVector<PortDeviceInfo>& currentDevices);

    // 向后兼容信号(仅端口名)
    /** @brief 兼容旧接口: 检测到新端口 @param portName 端口名称 */
    void portAdded(const QString& portName);
    /** @brief 兼容旧接口: 检测到端口移除 @param portName 端口名称 */
    void portRemoved(const QString& portName);
    /** @brief 兼容旧接口: 端口列表变化 */
    void portsChanged();

private slots:
    /** @brief 定时轮询回调 — 比较 currentPorts 与系统实际端口列表的差异 */
    void onTimeout();

private:
    /** @brief 查询系统中所有可用串口设备信息（按端口名排序） @return 设备信息映射 */
    QMap<QString, PortDeviceInfo> queryAvailableDevices();

    /** @brief 将QSerialPortInfo转换为PortDeviceInfo @param info Qt串口信息 @return 设备信息 */
    static PortDeviceInfo fromQtInfo(const QSerialPortInfo& info);

    /** @brief 将事件添加到日志(超出容量时移除最旧记录) @param event 事件记录 */
    void appendEventLog(const PortEventRecord& event);

    QTimer* m_timer;             ///< 轮询定时器
    QMap<QString, PortDeviceInfo> m_currentDevices; ///< 当前已知设备映射(端口名→信息)

    // ---- 统计计数器 ----
    quint64 m_totalArrivals = 0;    ///< 累计端口新增次数
    quint64 m_totalRemovals = 0;    ///< 累计端口移除次数
    quint64 m_totalPolls = 0;       ///< 累计轮询次数
    quint64 m_totalChanges = 0;     ///< 累计检测到变化的次数
    quint64 m_totalPortScans = 0;   ///< 累计端口扫描次数
    quint64 m_totalHotplugEvents = 0; ///< 累计热插拔事件次数
    quint64 m_totalScanErrors = 0;   ///< 累计端口扫描错误次数

    // ---- 事件日志 ----
    QVector<PortEventRecord> m_eventLog; ///< 事件日志(按时间顺序，最新在末尾)
    int m_eventLogCapacity = 100;        ///< 事件日志最大保留条数
    QElapsedTimer m_lastChangeTimer;     ///< 最近一次变更的计时器
    bool m_hasLastChange = false;        ///< 是否有过变更事件

    /**
     * @name 防抖机制成员
     * USB 热插拔时可能出现短暂的端口闪烁，防抖策略要求连续
     * kDebounceThreshold 次轮询都检测到同一变化，才确认并发射信号。
     * @{
     */
    QMap<QString, int> m_addedCandidateCount;    ///< 新增候选确认计数: 端口名 → 连续出现次数
    QMap<QString, int> m_removedCandidateCount;  ///< 移除候选确认计数: 端口名 → 连续消失次数
    QMap<QString, PortDeviceInfo> m_pendingAddDevices;  ///< 待确认新增设备的缓存信息
    static constexpr int kDebounceThreshold = 2; ///< 连续2次确认才发射信号（约4秒）
    /** @} */
};

#endif // PORTWATCHER_H
