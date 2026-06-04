/** @file ProtocolBridgeManager.h @brief 协议桥管理器 - 统一管理帧解析模式和VOFA+协议模式的数据路由
 *
 * 职责: 持有三个数据源(FrameParser/JustFloat/FireWater), 根据ChartProtocolMode路由串口数据,
 * 转发活动源的frameParsed信号, 提供状态查询/运行时切换/自动检测/每协议统计/吞吐量
 * 设计模式: 策略模式(Strategy) — 不同协议源是可互换策略，Manager是Context */

#ifndef PROTOCOLBRIDGEMANAGER_H
#define PROTOCOLBRIDGEMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>
#include <QElapsedTimer>
#include <QMap>

#include "protocol/parser/FrameParser.h"
#include "protocol/bridge/IProtocolBridge.h"
#include "protocol/bridge/JustFloatBridge.h"
#include "protocol/bridge/FireWaterBridge.h"

/** @brief 协议桥管理器 - 统一管理帧解析模式和VOFA+协议模式的数据路由
 * 协作: ConnectionController(上游feedData) / ChartModel/ProtocolView(下游连接frameParsed信号) */
class ProtocolBridgeManager : public QObject {
    Q_OBJECT

public:
    enum class ChartProtocolMode { FrameParser, JustFloat, FireWater }; ///< 协议模式枚举
    Q_ENUM(ChartProtocolMode)
    struct BridgeStats { bool isParsing = false; quint64 totalFramesParsed = 0; quint64 totalErrors = 0; quint64 checksumErrors = 0; };
    struct ProtocolStats { quint64 frames = 0; quint64 bytes = 0; quint64 errors = 0; };
    struct ThroughputSnapshot { double framesPerSec = 0.0; double bytesPerSec = 0.0; };
    struct AutoDetectResult { ChartProtocolMode detectedMode = ChartProtocolMode::FrameParser; double confidence = 0.0; bool detected = false; };

    explicit ProtocolBridgeManager(FrameParser* frameParser, QObject* parent = nullptr);
    ~ProtocolBridgeManager() override = default;
    ProtocolBridgeManager(const ProtocolBridgeManager&) = delete;
    ProtocolBridgeManager& operator=(const ProtocolBridgeManager&) = delete;

    void setProtocolMode(ChartProtocolMode mode);     ///< 设置协议模式(切换时重置旧源、激活新源)
    ChartProtocolMode protocolMode() const;           ///< 获取当前协议模式
    void feedData(const QByteArray& data);             ///< 接收原始串口数据，路由到活动协议源
    IProtocolBridge* activeBridge() const;             ///< 获取当前活动桥指针
    FrameParser* frameParser() const;                  ///< 获取FrameParser指针
    JustFloatBridge* justFloatBridge() const;          ///< 获取JustFloatBridge指针
    FireWaterBridge* fireWaterBridge() const;          ///< 获取FireWaterBridge指针

    // ---- 桥接器状态查询 ----
    BridgeStats bridgeStats() const;
    bool isParsing() const;
    quint64 totalFramesParsed() const;
    quint64 totalErrors() const;
    quint64 checksumErrors() const;

    // ---- 管理器级统计 ----
    quint64 totalBridges() const;
    quint64 totalFramesParsedAll() const;
    quint64 totalParseErrors() const;
    quint64 totalBytesProcessed() const;
    quint64 totalFeedDataCalls() const;
    quint64 totalEmptyDataSkips() const;
    quint64 totalAutoDetectAttempts() const;
    quint64 totalAutoDetectSuccesses() const;

    // ---- 每协议统计/吞吐量 ----
    ProtocolStats protocolStats(ChartProtocolMode mode) const;
    QMap<ChartProtocolMode, ProtocolStats> allProtocolStats() const;
    ThroughputSnapshot throughput() const;

    // ---- 自动检测/运行时切换 ----
    AutoDetectResult detectProtocol(const QByteArray& data) const;
    void setAutoDetectEnabled(bool enable);
    bool isAutoDetectEnabled() const;
    AutoDetectResult lastAutoDetectResult() const;
    void switchActiveBridge(ChartProtocolMode mode);   ///< 动态切换活跃桥(不中断数据流)
    void resetStats();

signals:
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame); ///< 转发活动源帧解析成功
    void frameError(const QString& reason, const QByteArray& rawFrame);      ///< 帧解析错误
    void protocolModeChanged(ChartProtocolMode mode);
    void autoDetectCompleted(const AutoDetectResult& result);

private:
    void switchSource();
    void onFrameParserParsed(const QVariantMap& f, const QByteArray& r);
    void onFrameParserError(const QString& r, const QByteArray& d);
    void onBridgeParsed(const QVariantMap& f, const QByteArray& r);
    double scoreJustFloat(const QByteArray& data) const;
    double scoreFireWater(const QByteArray& data) const;
    void updateThroughput(quint64 frameBytes);

    FrameParser* m_frameParser;          ///< 帧解析器(直接持有)
    JustFloatBridge* m_justFloat;        ///< JustFloat协议桥
    FireWaterBridge* m_fireWater;        ///< FireWater协议桥
    IProtocolBridge* m_activeBridge;     ///< 当前活动桥(FrameParser模式为nullptr)
    ChartProtocolMode m_mode;            ///< 当前协议模式

    quint64 m_totalFramesParsed = 0; quint64 m_totalErrors = 0; quint64 m_checksumErrors = 0;
    quint64 m_totalBridges = 0; quint64 m_totalFramesParsedAll = 0;
    quint64 m_totalParseErrors = 0; quint64 m_totalBytesProcessed = 0;
    quint64 m_totalFeedDataCalls = 0; quint64 m_totalEmptyDataSkips = 0;
    quint64 m_totalAutoDetectAttempts = 0; quint64 m_totalAutoDetectSuccesses = 0;

    QMap<ChartProtocolMode, ProtocolStats> m_protocolStats;
    QElapsedTimer m_throughputTimer;
    quint64 m_throughputFrameCount = 0; quint64 m_throughputByteCount = 0;
    mutable ThroughputSnapshot m_lastThroughput;
    bool m_autoDetectEnabled = false; AutoDetectResult m_lastDetectResult; QByteArray m_autoDetectBuffer;
    static constexpr int kAutoDetectMinBytes = 16;
    static constexpr int kAutoDetectMaxBytes = 512;
};

#endif // PROTOCOLBRIDGEMANAGER_H
