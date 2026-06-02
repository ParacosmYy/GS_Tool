/**
 * @file ProtocolBridgeManager.h
 * @brief 协议桥管理器 - 统一管理帧解析模式和VOFA+协议模式的数据路由
 *
 * 职责:
 *   1. 持有 FrameParser、JustFloatBridge、FireWaterBridge 三个数据源
 *   2. 根据 ChartProtocolMode 将原始串口数据路由到当前活动的数据源
 *   3. 转发当前活动源的 frameParsed 信号，下游无需知道数据来自哪个协议
 *   4. 提供桥接器状态查询（解析状态、累计帧数、错误统计）
 *   5. 支持运行时动态切换活跃桥接器（不中断数据流）
 *
 * 设计模式: 策略模式(Strategy) -- 不同协议源是可互换的策略，Manager 是 Context
 * 业务层: 不依赖任何表现层类
 */

#ifndef PROTOCOLBRIDGEMANAGER_H
#define PROTOCOLBRIDGEMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>

#include "protocol/parser/FrameParser.h"
#include "protocol/bridge/IProtocolBridge.h"
#include "protocol/bridge/JustFloatBridge.h"
#include "protocol/bridge/FireWaterBridge.h"

/**
 * @brief 协议桥管理器 - 统一管理帧解析模式和VOFA+协议模式的数据路由
 *
 * 持有三个数据源对象，根据当前模式将串口数据路由到对应的数据源。
 * 转发当前活动源的 frameParsed/frameError 信号，下游组件只需连接 Manager 的信号。
 *
 * 协作关系:
 *   - ConnectionController: 上游数据源，调用 feedData() 送入串口数据
 *   - ChartModel / ProtocolView: 下游消费者，连接 frameParsed/frameError 信号
 */
class ProtocolBridgeManager : public QObject {
    Q_OBJECT

public:
    /** @brief 协议模式枚举，不同模式对应不同的数据解析策略 */
    enum class ChartProtocolMode {
        FrameParser,    ///< 默认帧解析模式（用户自定义帧格式）
        JustFloat,      ///< VOFA+ JustFloat浮点字节流协议
        FireWater       ///< VOFA+ FireWater CSV尾标记协议
    };
    Q_ENUM(ChartProtocolMode)

    /** @brief 桥接器运行时统计信息，用于 UI 显示和诊断 */
    struct BridgeStats {
        bool isParsing = false;         ///< 是否正在解析中
        quint64 totalFramesParsed = 0;  ///< 累计成功解析帧数
        quint64 totalErrors = 0;        ///< 累计解析错误次数
        quint64 checksumErrors = 0;     ///< 累计校验错误次数
    };

    /** @brief 构造，默认使用 FrameParser 模式 */
    explicit ProtocolBridgeManager(FrameParser* frameParser, QObject* parent = nullptr);

    /** @brief 析构，QObject 父子树自动销毁持有的桥对象 */
    ~ProtocolBridgeManager() override = default;

    ProtocolBridgeManager(const ProtocolBridgeManager&) = delete;
    ProtocolBridgeManager& operator=(const ProtocolBridgeManager&) = delete;

    /** @brief 设置协议模式（切换时自动重置旧源、激活新源、通知 UI） */
    void setProtocolMode(ChartProtocolMode mode);

    /** @brief 获取当前协议模式 */
    ChartProtocolMode protocolMode() const;

    /** @brief 接收原始串口数据，转发到当前活动的协议源。空数据直接忽略 */
    void feedData(const QByteArray& data);

    /** @brief 获取当前活动的桥指针（FrameParser 模式下返回 nullptr） */
    IProtocolBridge* activeBridge() const;

    /** @brief 获取 FrameParser 指针 */
    FrameParser* frameParser() const;

    /** @brief 获取 JustFloatBridge 指针 */
    JustFloatBridge* justFloatBridge() const;

    /** @brief 获取 FireWaterBridge 指针 */
    FireWaterBridge* fireWaterBridge() const;

    // ---- 桥接器状态查询接口 ----

    /** @brief 获取当前活动桥接器的运行时统计信息 */
    BridgeStats bridgeStats() const;

    /** @brief 查询当前活动桥接器是否正在解析 */
    bool isParsing() const;

    /** @brief 获取当前活动桥接器的累计成功解析帧数 */
    quint64 totalFramesParsed() const;

    /** @brief 获取当前活动桥接器的累计解析错误次数 */
    quint64 totalErrors() const;

    /** @brief 获取当前活动桥接器的累计校验错误次数（仅 FrameParser 模式） */
    quint64 checksumErrors() const;

    // ---- 运行时动态切换 ----

    /**
     * @brief 动态切换活跃桥接器（不中断数据流、不重置旧源状态）
     *
     * 与 setProtocolMode 的区别: 不重置旧源状态，保留中间数据，
     * 适合快速切换场景（如自动检测协议类型）。
     */
    void switchActiveBridge(ChartProtocolMode mode);

    /** @brief 重置所有桥接器的错误和帧计数统计，不影响运行状态 */
    void resetStats();

signals:
    /** @brief 转发当前活动源的帧解析成功信号 */
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    /** @brief 帧解析错误（仅 FrameParser 模式） */
    void frameError(const QString& reason, const QByteArray& rawFrame);

    /** @brief 协议模式切换时发出，通知 UI 更新 */
    void protocolModeChanged(ChartProtocolMode mode);

private:
    /** @brief 切换数据源信号连接 */
    void switchSource();

    /** @brief FrameParser 帧解析成功槽（累加统计后转发） */
    void onFrameParserParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    /** @brief FrameParser 帧解析错误槽（累加校验错误后转发） */
    void onFrameParserError(const QString& reason, const QByteArray& rawFrame);

    /** @brief 桥接器帧解析成功槽（累加帧计数后转发） */
    void onBridgeParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    FrameParser* m_frameParser;         ///< 帧解析器（非桥，直接持有）
    JustFloatBridge* m_justFloat;       ///< JustFloat协议桥
    FireWaterBridge* m_fireWater;       ///< FireWater协议桥
    IProtocolBridge* m_activeBridge;    ///< 当前活动的桥（FrameParser模式下为nullptr）
    ChartProtocolMode m_mode;           ///< 当前协议模式

    quint64 m_totalFramesParsed = 0;    ///< 累计成功解析帧数
    quint64 m_totalErrors = 0;          ///< 累计解析错误次数
    quint64 m_checksumErrors = 0;       ///< 累计校验错误次数
};

#endif // PROTOCOLBRIDGEMANAGER_H
