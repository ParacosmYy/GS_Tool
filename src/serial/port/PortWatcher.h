/**
 * @file PortWatcher.h
 * @brief 串口热插拔检测器 - 基于定时轮询检测串口设备的插拔事件
 *
 * 设计思路:
 *   利用 QTimer 每 2 秒轮询一次 QSerialPortInfo::availablePorts()，
 *   将当前端口列表与上次快照做差异比较，检测出新增或移除的端口，
 *   并通过信号通知上层模块。
 *
 * 为什么用轮询而不是 Windows 设备事件:
 *   - 跨平台兼容（Windows/Linux/macOS 均可用）
 *   - 实现简单，不需要处理底层设备通知的复杂性
 *   - 2 秒间隔对串口热插拔场景足够灵敏，不会造成 CPU 负担
 *
 * 协作关系:
 *   - SerialConfigPanel: 可监听 portAdded/portRemoved 自动刷新端口列表
 *   - MainWindow: 可监听 portsChanged 在状态栏提示用户
 *
 * 所属层级: 数据层（检测系统硬件状态，不涉及 UI）
 */

#ifndef PORTWATCHER_H
#define PORTWATCHER_H

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <QMap>

/**
 * @brief 串口热插拔检测器
 *
 * 通过定时轮询 QSerialPortInfo 检测系统中串口设备的变化。
 * 当检测到端口新增或移除时，发射对应信号通知上层。
 *
 * 使用方式:
 * @code
 *   auto* watcher = new PortWatcher(this);
 *   connect(watcher, &PortWatcher::portAdded, this, &MyClass::onPortAdded);
 *   connect(watcher, &PortWatcher::portRemoved, this, &MyClass::onPortRemoved);
 *   watcher->start();  // 开始轮询
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

    /** @brief 获取当前已知的端口名称列表 */
    QStringList currentPorts() const;

    /** @brief 获取累计检测到的端口新增次数 */
    quint64 totalArrivals() const;
    /** @brief 获取累计检测到的端口移除次数 */
    quint64 totalRemovals() const;
    /** @brief 获取累计轮询次数 */
    quint64 totalPolls() const;
    /** @brief 获取累计检测到变化的次数（新增+移除事件合计） */
    quint64 totalChanges() const;
    /** @brief 获取累计端口扫描次数（与轮询次数一致） */
    quint64 totalPortScans() const;
    /** @brief 获取累计热插拔事件次数（新增+移除事件合计，与totalChanges一致） */
    quint64 totalHotplugEvents() const;
    /** @brief 重置所有统计计数器 */
    void resetWatcherStatistics();

signals:
    /** @brief 检测到新串口设备接入 @param portName 新增端口的系统名称 */
    void portAdded(const QString& portName);
    /** @brief 检测到串口设备移除 @param portName 被移除端口的系统名称 */
    void portRemoved(const QString& portName);
    /** @brief 端口列表发生了变化（任何新增或移除后都会发射） */
    void portsChanged();

private slots:
    /** @brief 定时轮询回调 — 比较 currentPorts 与系统实际端口列表的差异 */
    void onTimeout();

private:
    /** @brief 查询系统中所有可用串口名称列表（按字母排序） */
    static QStringList queryAvailablePorts();

    QTimer* m_timer;             ///< 轮询定时器
    QStringList m_currentPorts;  ///< 上次快照的端口名称列表

    quint64 m_totalArrivals = 0;    ///< 累计端口新增次数
    quint64 m_totalRemovals = 0;    ///< 累计端口移除次数
    quint64 m_totalPolls = 0;       ///< 累计轮询次数
    quint64 m_totalChanges = 0;     ///< 累计检测到变化的次数
    quint64 m_totalPortScans = 0;   ///< 累计端口扫描次数
    quint64 m_totalHotplugEvents = 0; ///< 累计热插拔事件次数

    /**
     * @name 防抖机制成员
     * USB 热插拔时可能出现短暂的端口闪烁，防抖策略要求连续
     * kDebounceThreshold 次轮询都检测到同一变化，才确认并发射信号。
     * @{
     */
    QMap<QString, int> m_addedCandidateCount;    ///< 新增候选确认计数: 端口名 → 连续出现次数
    QMap<QString, int> m_removedCandidateCount;  ///< 移除候选确认计数: 端口名 → 连续消失次数
    static constexpr int kDebounceThreshold = 2; ///< 连续2次确认才发射信号（约2秒）
    /** @} */
};

#endif // PORTWATCHER_H
