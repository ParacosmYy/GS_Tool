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
     *
     * 初始化内部定时器，但不启动轮询。需调用 start() 开始检测。
     *
     * @param parent 父对象，用于 Qt 对象树管理生命周期
     */
    explicit PortWatcher(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     *
     * 停止定时器，清理资源。定时器作为子对象会被 Qt 对象树自动销毁。
     */
    ~PortWatcher() override;

    // 禁止拷贝和赋值
    PortWatcher(const PortWatcher&) = delete;
    PortWatcher& operator=(const PortWatcher&) = delete;

    /**
     * @brief 启动热插拔检测
     *
     * 立即执行一次端口快照作为基准，然后启动定时轮询。
     * 如果已在运行状态，调用此方法无效（不会重置定时器）。
     */
    void start();

    /**
     * @brief 停止热插拔检测
     *
     * 停止定时器，清空内部端口快照。下次 start() 时会重新建立基准。
     */
    void stop();

    /**
     * @brief 检测是否正在运行
     *
     * @return true 如果定时器正在运行（正在轮询）
     */
    bool isRunning() const;

    /**
     * @brief 获取轮询间隔（毫秒）
     *
     * @return 当前轮询间隔，默认 2000ms
     */
    int interval() const;

    /**
     * @brief 设置轮询间隔
     *
     * 仅在停止状态下生效。如果正在运行，需要先 stop() 再 start()。
     *
     * @param msec 轮询间隔（毫秒），必须 > 0
     */
    void setInterval(int msec);

    /**
     * @brief 获取当前已知的端口名称列表
     *
     * 返回最近一次轮询快照中的端口列表。
     *
     * @return 端口名称列表（如 ["COM3", "COM5"]）
     */
    QStringList currentPorts() const;

signals:
    /**
     * @brief 检测到新串口设备接入
     *
     * 当轮询发现系统端口列表中有之前不存在的端口时发射。
     *
     * @param portName 新增端口的系统名称（如 "COM3"）
     */
    void portAdded(const QString& portName);

    /**
     * @brief 检测到串口设备移除
     *
     * 当轮询发现之前存在的端口从系统中消失时发射。
     *
     * @param portName 被移除端口的系统名称（如 "COM3"）
     */
    void portRemoved(const QString& portName);

    /**
     * @brief 端口列表发生了变化
     *
     * 任何端口新增或移除后都会发射此信号。
     * 适用于只需要知道"列表变了"而不关心具体变化的场景。
     */
    void portsChanged();

private slots:
    /**
     * @brief 定时轮询回调
     *
     * 比较 currentPorts 与系统实际端口列表的差异，
     * 对新增和移除分别发射信号。
     */
    void onTimeout();

private:
    /**
     * @brief 获取系统中所有可用串口名称列表
     *
     * @return 端口名称列表（按字母排序，确保比较结果稳定）
     */
    static QStringList queryAvailablePorts();

    QTimer* m_timer;             ///< 轮询定时器
    QStringList m_currentPorts;  ///< 上次快照的端口名称列表

    /**
     * @name 防抖机制成员
     *
     * USB 热插拔时可能出现短暂的端口闪烁（一瞬间出现又消失），
     * 导致误报 portAdded/portRemoved 信号。防抖策略要求连续
     * kDebounceThreshold 次轮询都检测到同一变化，才确认并发射信号。
     * 如果候选端口在确认前恢复原状态，计数器被清零。
     * @{
     */
    QMap<QString, int> m_addedCandidateCount;    ///< 新增候选确认计数: 端口名 → 连续出现次数
    QMap<QString, int> m_removedCandidateCount;  ///< 移除候选确认计数: 端口名 → 连续消失次数
    static constexpr int kDebounceThreshold = 2; ///< 连续2次确认才发射信号（约2秒）
    /** @} */
};

#endif // PORTWATCHER_H
