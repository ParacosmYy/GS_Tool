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
    m_timer->setInterval(2000);

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
 * @brief 定时轮询回调
 *
 * 查询当前系统可用端口，与上次快照比较:
 *   - 新增的端口 → 逐个发射 portAdded
 *   - 移除的端口 → 逐个发射 portRemoved
 *   - 有任何变化 → 发射 portsChanged
 */
void PortWatcher::onTimeout()
{
    const QStringList newPorts = queryAvailablePorts();

    // 检测新增端口: 在 newPorts 中但不在 m_currentPorts 中
    for (const QString& port : newPorts) {
        if (!m_currentPorts.contains(port)) {
            emit portAdded(port);
        }
    }

    // 检测移除端口: 在 m_currentPorts 中但不在 newPorts 中
    for (const QString& port : m_currentPorts) {
        if (!newPorts.contains(port)) {
            emit portRemoved(port);
        }
    }

    // 有变化时更新快照并发射总信号
    if (m_currentPorts != newPorts) {
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
