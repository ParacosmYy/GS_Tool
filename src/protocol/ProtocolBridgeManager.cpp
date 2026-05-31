/**
 * @file ProtocolBridgeManager.cpp
 * @brief 协议桥管理器实现
 *
 * 实现:
 *   - 构造/析构: 持有三个数据源对象，默认使用 FrameParser 模式
 *   - 模式切换: 重置旧源 → 断开旧信号 → 连接新信号 → 通知 UI
 *   - 数据路由: feedData() 根据当前模式将数据转发到对应的数据源
 *   - 空数据保护: 空数据直接忽略，避免无意义的处理开销
 */

#include "protocol/ProtocolBridgeManager.h"

#include <QMetaObject>
#include <QMetaMethod>
#include <QDebug>

/**
 * @brief 构造协议桥管理器
 * @param frameParser 外部创建的帧解析器
 * @param parent 父对象
 *
 * frameParser 由 MainWindow 创建并传入。如果 frameParser 尚未设置 parent，
 * 则归本对象管理（setParent）。同时创建 JustFloatBridge 和 FireWaterBridge。
 * 初始连接 FrameParser 的信号。
 */
ProtocolBridgeManager::ProtocolBridgeManager(FrameParser* frameParser, QObject* parent)
    : QObject(parent)
    , m_frameParser(frameParser)
    , m_justFloat(new JustFloatBridge(this))
    , m_fireWater(new FireWaterBridge(this))
    , m_activeBridge(nullptr)
    , m_mode(ChartProtocolMode::FrameParser)
{
    // FrameParser是外部创建的对象，如果尚未设置parent则归我们管理
    if (m_frameParser && !m_frameParser->parent()) {
        m_frameParser->setParent(this);
    }

    // 初始连接: 默认FrameParser模式，连接FrameParser的信号
    switchSource();
}

/**
 * @brief 析构协议桥管理器
 *
 * QObject 父子树自动销毁 m_justFloat、m_fireWater。
 * m_frameParser 如果 parent 是本对象也会被自动销毁。
 */
ProtocolBridgeManager::~ProtocolBridgeManager()
{
}

/**
 * @brief 设置协议模式
 * @param mode 目标协议模式
 *
 * 切换流程:
 *   1. 如果 mode 与当前模式相同，直接返回
 *   2. 重置旧源的内部状态（清空缓冲区）
 *   3. 更新 m_mode
 *   4. 重新连接信号（switchSource）
 *   5. 发出 protocolModeChanged 信号通知 UI
 */
void ProtocolBridgeManager::setProtocolMode(ChartProtocolMode mode)
{
    if (m_mode == mode) {
        return; // 模式未变，无需切换
    }

    // 重置旧源状态
    if (m_activeBridge) {
        m_activeBridge->reset();
    } else if (m_mode == ChartProtocolMode::FrameParser) {
        m_frameParser->reset();
    }

    // 切换模式
    m_mode = mode;

    // 重新连接信号
    switchSource();

    // 通知UI更新通道配置
    emit protocolModeChanged(m_mode);
}

/**
 * @brief 获取当前协议模式
 * @return 当前活动协议模式
 */
ProtocolBridgeManager::ChartProtocolMode ProtocolBridgeManager::protocolMode() const
{
    return m_mode;
}

/**
 * @brief 接收原始串口数据，转发到当前活动的协议源
 * @param data 原始字节数据
 *
 * 空数据保护: 传入空 QByteArray 时直接返回，不触发任何处理。
 * 这避免了空数据导致的状态机无意义调用和潜在的边界问题。
 *
 * 无效模式保护: switch 的 default 分支会输出 qWarning 日志，
 * 理论上不会触发（枚举覆盖完整），但作为防御性编程的保底措施。
 */
void ProtocolBridgeManager::feedData(const QByteArray& data)
{
    // ---- 空数据保护 ----
    // 串口在特殊情况下（如连接刚建立/断开瞬间）可能产生空数据，
    // 直接忽略以避免无意义的处理开销和潜在的边界问题
    if (data.isEmpty()) {
        return;
    }

    switch (m_mode) {
    case ChartProtocolMode::FrameParser:
        if (m_frameParser) {
            m_frameParser->feed(data);
        } else {
            qWarning() << "ProtocolBridgeManager::feedData: FrameParser mode but m_frameParser is null";
        }
        break;

    case ChartProtocolMode::JustFloat:
        if (m_justFloat) {
            m_justFloat->feed(data);
        } else {
            qWarning() << "ProtocolBridgeManager::feedData: JustFloat mode but m_justFloat is null";
        }
        break;

    case ChartProtocolMode::FireWater:
        if (m_fireWater) {
            m_fireWater->feed(data);
        } else {
            qWarning() << "ProtocolBridgeManager::feedData: FireWater mode but m_fireWater is null";
        }
        break;

    default:
        // 防御性编程: 枚举覆盖完整时不应该到达这里
        qWarning() << "ProtocolBridgeManager::feedData: unknown protocol mode:" << static_cast<int>(m_mode);
        break;
    }
}

/**
 * @brief 获取当前活动的桥
 * @return 当前活动桥指针，FrameParser 模式下返回 nullptr
 */
IProtocolBridge* ProtocolBridgeManager::activeBridge() const
{
    return m_activeBridge;
}

/**
 * @brief 获取 FrameParser 指针
 * @return 帧解析器指针
 */
FrameParser* ProtocolBridgeManager::frameParser() const
{
    return m_frameParser;
}

/**
 * @brief 获取 JustFloatBridge 指针
 * @return JustFloat 协议桥指针
 */
JustFloatBridge* ProtocolBridgeManager::justFloatBridge() const
{
    return m_justFloat;
}

/**
 * @brief 获取 FireWaterBridge 指针
 * @return FireWater 协议桥指针
 */
FireWaterBridge* ProtocolBridgeManager::fireWaterBridge() const
{
    return m_fireWater;
}

/**
 * @brief 切换数据源连接
 *
 * 根据当前 m_mode，断开所有源到本 Manager 的信号连接，
 * 然后仅连接活动源的信号。
 *
 * FrameParser 模式: 连接 frameParsed + frameError
 * JustFloat 模式: 连接 frameParsed
 * FireWater 模式: 连接 frameParsed
 *
 * 断开操作使用 disconnect(sender, signal, this, slot) 精确匹配，
 * 不会影响其他对象的信号连接。
 */
void ProtocolBridgeManager::switchSource()
{
    // ---- 先断开所有源到本manager转发的连接 ----

    // 断开FrameParser的frameParsed和frameError
    disconnect(m_frameParser, &FrameParser::frameParsed,
               this, &ProtocolBridgeManager::frameParsed);
    disconnect(m_frameParser, &FrameParser::frameError,
               this, &ProtocolBridgeManager::frameError);

    // 断开JustFloatBridge的frameParsed
    disconnect(m_justFloat, &JustFloatBridge::frameParsed,
               this, &ProtocolBridgeManager::frameParsed);

    // 断开FireWaterBridge的frameParsed
    disconnect(m_fireWater, &FireWaterBridge::frameParsed,
               this, &ProtocolBridgeManager::frameParsed);

    // ---- 根据模式设置活动桥并连接信号 ----

    switch (m_mode) {
    case ChartProtocolMode::FrameParser:
        m_activeBridge = nullptr;
        // FrameParser有frameParsed和frameError两个信号，都需要转发
        connect(m_frameParser, &FrameParser::frameParsed,
                this, &ProtocolBridgeManager::frameParsed);
        connect(m_frameParser, &FrameParser::frameError,
                this, &ProtocolBridgeManager::frameError);
        break;

    case ChartProtocolMode::JustFloat:
        m_activeBridge = m_justFloat;
        // JustFloatBridge只有frameParsed（无错误信号，解析失败静默丢弃）
        connect(m_justFloat, &JustFloatBridge::frameParsed,
                this, &ProtocolBridgeManager::frameParsed);
        break;

    case ChartProtocolMode::FireWater:
        m_activeBridge = m_fireWater;
        // FireWaterBridge只有frameParsed（无错误信号，解析失败静默丢弃）
        connect(m_fireWater, &FireWaterBridge::frameParsed,
                this, &ProtocolBridgeManager::frameParsed);
        break;
    }
}
