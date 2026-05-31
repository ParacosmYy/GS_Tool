#include "protocol/ProtocolBridgeManager.h"

#include <QMetaObject>
#include <QMetaMethod>

// 构造函数: 持有三个数据源对象，默认使用FrameParser模式
// frameParser由MainWindow创建并传入，this成为其parent
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

ProtocolBridgeManager::~ProtocolBridgeManager()
{
}

// 设置协议模式
// 切换时会:
//   1. 重置旧源的内部状态（清空缓冲区）
//   2. 断开旧源的信号连接
//   3. 连接新源的信号
//   4. 发出protocolModeChanged信号通知UI
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

// 获取当前协议模式
ProtocolBridgeManager::ChartProtocolMode ProtocolBridgeManager::protocolMode() const
{
    return m_mode;
}

// 接收原始串口数据，路由到当前活动的协议源
void ProtocolBridgeManager::feedData(const QByteArray& data)
{
    switch (m_mode) {
    case ChartProtocolMode::FrameParser:
        m_frameParser->feed(data);
        break;
    case ChartProtocolMode::JustFloat:
        m_justFloat->feed(data);
        break;
    case ChartProtocolMode::FireWater:
        m_fireWater->feed(data);
        break;
    }
}

// 获取当前活动的桥（FrameParser模式下返回nullptr）
IProtocolBridge* ProtocolBridgeManager::activeBridge() const
{
    return m_activeBridge;
}

// 获取FrameParser指针
FrameParser* ProtocolBridgeManager::frameParser() const
{
    return m_frameParser;
}

// 获取JustFloatBridge指针
JustFloatBridge* ProtocolBridgeManager::justFloatBridge() const
{
    return m_justFloat;
}

// 获取FireWaterBridge指针
FireWaterBridge* ProtocolBridgeManager::fireWaterBridge() const
{
    return m_fireWater;
}

// 切换数据源连接
// 根据当前m_mode，断开所有源到本manager的信号，然后仅连接活动源
// 这样Manager::frameParsed始终转发的是当前活动源的解析结果
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
