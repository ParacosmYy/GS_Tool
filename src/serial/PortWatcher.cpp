/**
 * @file PortWatcher.cpp
 * @brief 串口热插拔检测器实现 - 基于定时轮询检测串口设备的插拔事件
 *
 * 实现原理:
 *   1. start() 时立即拍一次快照作为基准（m_currentPorts）
 *   2. 定时器每 2 秒触发 onTimeout()
 *   3. onTimeout() 查询当前系统端口列表，与快照做差集运算:
 *      - 新列表中有而快照中没有 → portAdded 信号
 *      - 快照中有而新列表中没有 → portRemoved 信号
 *   4. 有变化时更新快照并发射 portsChanged 信号
 */

#include "serial/PortWatcher.h"
#include "core/Constants.h"
#include <QSerialPortInfo>
#include <algorithm>

/**
 * @brief 构造函数
 *
 * 创建定时器并设置默认间隔 2000ms。
 * 定时器的 parent 设为 this，确保随 PortWatcher 一起销毁。
 */
PortWatcher::PortWatcher(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(Timers::kPortPollMs);

    // 连接定时器超时到轮询处理函数
    connect(m_timer, &QTimer::timeout, this, &PortWatcher::onTimeout);
}

/**
 * @brief 析构函数
 *
 * 停止定时器。m_timer 作为子对象会被 Qt 对象树自动删除。
 */
PortWatcher::~PortWatcher()
{
    m_timer->stop();
}

/**
 * @brief 启动热插拔检测
 *
 * 立即查询一次当前端口列表作为基准快照，然后启动定时轮询。
 * 如果已经在运行，直接返回，避免重复启动。
 */
void PortWatcher::start()
{
    if (m_timer->isActive()) {
        return;  // 已经在运行，不重复启动
    }

    // 建立基准快照
    m_currentPorts = queryAvailablePorts();

    // 启动定时轮询
    m_timer->start();
}

/**
 * @brief 停止热插拔检测
 *
 * 停止定时器并清空端口快照，下次 start() 时会重新建立基准。
 */
void PortWatcher::stop()
{
    m_timer->stop();
    m_currentPorts.clear();
}

/**
 * @brief 检测是否正在运行
 *
 * @return true 如果定时器处于活跃状态
 */
bool PortWatcher::isRunning() const
{
    return m_timer->isActive();
}

/**
 * @brief 获取轮询间隔
 *
 * @return 当前轮询间隔（毫秒）
 */
int PortWatcher::interval() const
{
    return m_timer->interval();
}

/**
 * @brief 设置轮询间隔
 *
 * @param msec 新的轮询间隔（毫秒），必须 > 0
 */
void PortWatcher::setInterval(int msec)
{
    if (msec > 0) {
        m_timer->setInterval(msec);
    }
}

/**
 * @brief 获取当前已知的端口名称列表
 *
 * @return 最近一次快照中的端口名称列表
 */
QStringList PortWatcher::currentPorts() const
{
    return m_currentPorts;
}

/**
 * @brief 定时轮询回调（带防抖）
 *
 * 查询当前系统可用端口，与上次快照比较:
 *   - 新增候选: 在 newPorts 中但不在 m_currentPorts 中 → 累计计数，
 *     达到 kDebounceThreshold 后发射 portAdded 并更新快照
 *   - 移除候选: 在 m_currentPorts 中但不在 newPorts 中 → 累计计数，
 *     达到 kDebounceThreshold 后发射 portRemoved 并更新快照
 *   - 候选端口恢复原状态 → 清零对应计数器（防抖消除闪烁）
 *   - 确认变化后发射 portsChanged 总信号
 */
void PortWatcher::onTimeout()
{
    const QStringList newPorts = queryAvailablePorts();
    bool changed = false;

    // ---- 处理新增候选: 在 newPorts 中但不在 m_currentPorts 中 ----
    for (const QString& port : newPorts) {
        if (!m_currentPorts.contains(port)) {
            // 端口是新出现的，累加新增确认计数
            ++m_addedCandidateCount[port];
            if (m_addedCandidateCount[port] >= kDebounceThreshold) {
                emit portAdded(port);
                m_addedCandidateCount.remove(port);
                changed = true;
            }
        } else {
            // 端口依然存在，清除残留的新增候选计数（状态未变，无需操作）
            m_addedCandidateCount.remove(port);
        }
    }

    // ---- 处理移除候选: 在 m_currentPorts 中但不在 newPorts 中 ----
    for (const QString& port : m_currentPorts) {
        if (!newPorts.contains(port)) {
            // 端口消失了，累加移除确认计数
            ++m_removedCandidateCount[port];
            if (m_removedCandidateCount[port] >= kDebounceThreshold) {
                emit portRemoved(port);
                m_removedCandidateCount.remove(port);
                changed = true;
            }
        } else {
            // 端口依然存在，清除残留的移除候选计数（端口回来了，闪烁消除）
            m_removedCandidateCount.remove(port);
        }
    }

    // ---- 清理过期候选: 已恢复原状态的候选计数器 ----
    // 新增候选中出现但实际已存在的端口 → 状态恢复，清零
    for (const QString& port : m_addedCandidateCount.keys()) {
        if (m_currentPorts.contains(port)) {
            m_addedCandidateCount.remove(port);
        }
    }
    // 移除候选中消失但实际已不存在的端口 → 无需清理（上面已处理）

    // ---- 更新快照: 将已确认的端口变化反映到 m_currentPorts ----
    if (changed) {
        m_currentPorts = newPorts;
        emit portsChanged();
    }
}

/**
 * @brief 查询系统中所有可用串口名称
 *
 * 使用 QSerialPortInfo 枚举所有串口，提取端口名称并排序。
 * 排序确保每次查询结果顺序一致，避免因顺序差异导致误检测。
 *
 * @return 排序后的端口名称列表
 */
QStringList PortWatcher::queryAvailablePorts()
{
    QStringList portNames;
    const auto ports = QSerialPortInfo::availablePorts();
    portNames.reserve(ports.size());

    for (const QSerialPortInfo& info : ports) {
        portNames.append(info.portName());
    }

    // 按字母排序，确保比较结果稳定
    portNames.sort();

    return portNames;
}
