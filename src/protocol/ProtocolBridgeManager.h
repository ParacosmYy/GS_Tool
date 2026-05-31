#ifndef PROTOCOLBRIDGEMANAGER_H
#define PROTOCOLBRIDGEMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>

#include "protocol/FrameParser.h"
#include "protocol/IProtocolBridge.h"
#include "protocol/JustFloatBridge.h"
#include "protocol/FireWaterBridge.h"

// 协议桥管理器 -- 统一管理帧解析模式和VOFA+协议模式的数据路由
//
// 职责:
//   1. 持有FrameParser、JustFloatBridge、FireWaterBridge三个数据源
//   2. 根据ChartProtocolMode将原始串口数据路由到当前活动的数据源
//   3. 转发当前活动源的frameParsed信号，下游ChartModel/ProtocolView无需知道数据来自哪个协议
//   4. 模式切换时重置旧源、激活新源，发出protocolModeChanged信号通知UI
//
// 数据流:
//   ConnectionController::dataReceived
//     → ProtocolBridgeManager::feedData
//       → 当前活动源(FrameParser / JustFloatBridge / FireWaterBridge).feed()
//         → 源内部解析完成后 emit frameParsed(fields, rawFrame)
//           → ProtocolBridgeManager::frameParsed 转发
//             → ChartModel::onFrameParsed + ProtocolView::onFrameParsed
//
// 设计模式: 策略模式(Strategy) -- 不同协议源是可互换的策略，Manager是Context
// 业务层: 不依赖任何表现层类
class ProtocolBridgeManager : public QObject {
    Q_OBJECT

public:
    // 协议模式枚举
    enum class ChartProtocolMode {
        FrameParser,    // 默认帧解析模式（用户自定义帧格式）
        JustFloat,      // VOFA+ JustFloat浮点字节流协议
        FireWater       // VOFA+ FireWater CSV尾标记协议
    };
    Q_ENUM(ChartProtocolMode)

    explicit ProtocolBridgeManager(FrameParser* frameParser, QObject* parent = nullptr);

    ~ProtocolBridgeManager() override;

    // 禁止拷贝和赋值（QObject派生类）
    ProtocolBridgeManager(const ProtocolBridgeManager&) = delete;
    ProtocolBridgeManager& operator=(const ProtocolBridgeManager&) = delete;

    // 设置协议模式（切换时自动重置旧源、激活新源）
    void setProtocolMode(ChartProtocolMode mode);

    // 获取当前协议模式
    ChartProtocolMode protocolMode() const;

    // 接收原始串口数据，转发到当前活动的协议源
    void feedData(const QByteArray& data);

    // 获取当前活动的桥（用于外部配置，如设置通道数、分隔符等）
    // FrameParser模式下返回nullptr（FrameParser不是IProtocolBridge）
    IProtocolBridge* activeBridge() const;

    // 获取FrameParser指针（用于帧编辑器设置定义等场景）
    FrameParser* frameParser() const;

    // 获取JustFloatBridge指针（用于设置固定通道数等配置）
    JustFloatBridge* justFloatBridge() const;

    // 获取FireWaterBridge指针（用于设置分隔符等配置）
    FireWaterBridge* fireWaterBridge() const;

signals:
    // 转发当前活动源的frameParsed信号
    // 无论哪种模式，下游只需连接此信号即可接收解析结果
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    // 帧解析错误（仅FrameParser模式会产生此信号）
    void frameError(const QString& reason, const QByteArray& rawFrame);

    // 协议模式切换时发出，通知UI更新通道配置
    void protocolModeChanged(ChartProtocolMode mode);

private:
    // 切换数据源连接（断开旧源信号、连接新源信号）
    void switchSource();

    FrameParser* m_frameParser;         // 帧解析器（非桥，直接持有）
    JustFloatBridge* m_justFloat;       // JustFloat协议桥
    FireWaterBridge* m_fireWater;       // FireWater协议桥
    IProtocolBridge* m_activeBridge;    // 当前活动的桥（FrameParser模式下为nullptr）
    ChartProtocolMode m_mode;           // 当前协议模式
};

#endif // PROTOCOLBRIDGEMANAGER_H
