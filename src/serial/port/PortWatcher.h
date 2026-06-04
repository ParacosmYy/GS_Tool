/**
 * @file PortWatcher.h
 * @brief 串口热插拔检测器 -- QTimer轮询QSerialPortInfo检测端口变化，携带设备详情
 *
 * 增强: 完整设备信息(PortDeviceInfo)/端口事件统计/设备变更日志/防抖机制
 * 协作: SerialConfigPanel(刷新端口) / MainWindow(状态栏提示) / SerialDetector(芯片识别)
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

/** @brief 端口设备详细信息 -- 热插拔事件携带的设备描述 */
struct PortDeviceInfo {
    QString portName;          ///< 端口名称(如"COM3")
    QString description;       ///< 设备描述
    QString manufacturer;      ///< 制造商名称
    QString serialNumber;      ///< 序列号
    quint16 vendorId = 0;      ///< USB厂商ID(VID)
    quint16 productId = 0;     ///< USB产品ID(PID)
    QString vidHex;            ///< VID十六进制字符串(如"1A86")
    QString pidHex;            ///< PID十六进制字符串(如"7523")

    /** @brief 检查设备信息是否为空 @return true=端口名为空 */
    bool isEmpty() const { return portName.isEmpty(); }

    /** @brief 生成单行摘要文本 @return 摘要字符串如"COM3 - CH340 [1A86:7523]" */
    QString toShortSummary() const;
};

/** @brief 端口事件记录 -- 记录一次热插拔事件 */
struct PortEventRecord {
    /** @brief 事件类型枚举 */
    enum class EventType { Arrival, Removal };
    EventType type;                      ///< 事件类型(接入/移除)
    QDateTime timestamp;                 ///< 事件发生时间
    PortDeviceInfo device;               ///< 涉及的设备信息
    int totalPortsAfterEvent = 0;        ///< 事件后的在线端口总数

    /** @brief 生成日志文本 @return 日志字符串如"[14:32:05] +COM3 CH340" */
    QString toLogString() const;
};

/** @brief 串口热插拔检测器(增强版) -- 定时轮询QSerialPortInfo检测变化，携带设备详情信号 */
class PortWatcher : public QObject {
    Q_OBJECT

public:
    /** @brief 构造串口热插拔检测器 @param parent 父对象 */
    explicit PortWatcher(QObject* parent = nullptr);

    /** @brief 析构函数，停止定时器，子对象自动销毁 */
    ~PortWatcher() override;

    PortWatcher(const PortWatcher&) = delete;                  ///< 禁止拷贝
    PortWatcher& operator=(const PortWatcher&) = delete;       ///< 禁止赋值

    /** @brief 启动检测(建立基准快照+定时轮询) */
    void start();

    /** @brief 停止检测 */
    void stop();

    /** @brief 检测是否运行中 @return true=正在运行 */
    bool isRunning() const;

    /** @brief 获取轮询间隔 @return 间隔(毫秒)，默认2000 */
    int interval() const;

    /** @brief 设置轮询间隔 @param msec 间隔(毫秒)，仅停止时生效 */
    void setInterval(int msec);

    /** @brief 获取当前已知端口名称列表 @return 端口名称列表 */
    QStringList currentPorts() const;

    /** @brief 获取当前所有设备详细信息 @return 设备信息向量 */
    QVector<PortDeviceInfo> currentDevices() const;

    /** @brief 按端口名查询设备详情 @param portName 端口名称 @return 设备信息，未找到返回空值 */
    PortDeviceInfo deviceInfo(const QString& portName) const;

    /** @brief 获取累计端口新增次数 @return 新增次数 */
    quint64 totalArrivals() const;

    /** @brief 获取累计端口移除次数 @return 移除次数 */
    quint64 totalRemovals() const;

    /** @brief 获取累计轮询次数 @return 轮询次数 */
    quint64 totalPolls() const;

    /** @brief 获取累计变化次数(新增+移除) @return 变化次数 */
    quint64 totalChanges() const;

    /** @brief 获取累计端口扫描次数 @return 扫描次数 */
    quint64 totalPortScans() const;

    /** @brief 获取累计热插拔事件次数 @return 事件次数 */
    quint64 totalHotplugEvents() const;

    /** @brief 获取累计端口扫描错误次数 @return 错误次数 */
    quint64 totalScanErrors() const;

    /** @brief 获取最近变更距现在毫秒数 @return 毫秒数，无事件返回-1 */
    qint64 msSinceLastChange() const;

    /** @brief 获取当前在线设备数 @return 在线设备数量 */
    int onlineDeviceCount() const;

    /** @brief 获取最近事件记录(时间倒序) @param maxCount 最大返回条数，0=全部 @return 事件记录向量 */
    QVector<PortEventRecord> recentEvents(int maxCount = 0) const;

    /** @brief 获取事件日志最大条数 @return 容量值 */
    int eventLogCapacity() const;

    /** @brief 设置日志容量 @param capacity 最大条数(最小10) */
    void setEventLogCapacity(int capacity);

    /** @brief 重置所有统计和日志 */
    void resetWatcherStatistics();

signals:
    /** @brief 新串口设备接入(携带设备详情) @param device 接入的设备信息 */
    void deviceAdded(const PortDeviceInfo& device);

    /** @brief 串口设备移除(携带设备详情) @param device 移除的设备信息 */
    void deviceRemoved(const PortDeviceInfo& device);

    /** @brief 端口列表变化 @param devs 当前所有设备信息 */
    void devicesChanged(const QVector<PortDeviceInfo>& devs);

    /** @brief 兼容接口: 新端口(仅端口名) @param portName 端口名称 */
    void portAdded(const QString& portName);

    /** @brief 兼容接口: 端口移除(仅端口名) @param portName 端口名称 */
    void portRemoved(const QString& portName);

    /** @brief 兼容接口: 端口列表变化 */
    void portsChanged();

private slots:
    /** @brief 定时轮询回调 */
    void onTimeout();

private:
    /** @brief 查询系统可用串口(按端口名排序) @return 端口名到设备信息的映射 */
    QMap<QString, PortDeviceInfo> queryAvailableDevices();

    /** @brief 将QSerialPortInfo转换为PortDeviceInfo @param info Qt端口信息 @return 内部设备信息 */
    static PortDeviceInfo fromQtInfo(const QSerialPortInfo& info);

    /** @brief 追加事件日志(超容量移除旧记录) @param event 事件记录 */
    void appendEventLog(const PortEventRecord& event);

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
