/** @file ProtocolBridgeManager.h @brief 协议桥管理器 - 统一管理帧解析模式和VOFA+协议模式的数据路由
 *
 * 职责: 持有三个数据源(FrameParser/JustFloat/FireWater), 根据ChartProtocolMode路由串口数据,
 * 转发活动源的frameParsed信号, 提供状态查询/运行时切换/自动检测/每协议统计/吞吐量计算
 * 设计模式: 策略模式(Strategy) — 不同协议源是可互换的策略，Manager是Context */

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
    /** @brief 协议模式枚举 */
    enum class ChartProtocolMode { FrameParser, JustFloat, FireWater };
    Q_ENUM(ChartProtocolMode)

    /** @brief 桥接器运行时统计 */
    struct BridgeStats {
        bool isParsing = false; quint64 totalFramesParsed = 0; quint64 totalErrors = 0; quint64 checksumErrors = 0;
    };

    /** @brief 单个协议的累计统计 */
    struct ProtocolStats { quint64 frames = 0; quint64 bytes = 0; quint64 errors = 0; };

    /** @brief 吞吐量快照 */
    struct ThroughputSnapshot { double framesPerSec = 0.0; double bytesPerSec = 0.0; };

    /** @brief 自动检测结果 */
    struct AutoDetectResult {
        ChartProtocolMode detectedMode = ChartProtocolMode::FrameParser; double confidence = 0.0; bool detected = false;
    };

    /** @brief 构造，默认使用 FrameParser 模式 */
    explicit ProtocolBridgeManager(FrameParser* frameParser, QObject* parent = nullptr);

    /** @brief 析构，QObject 父子树自动销毁持有的桥对象 */
    ~ProtocolBridgeManager() override = default;

    /** @brief 禁止拷贝构造 */
    ProtocolBridgeManager(const ProtocolBridgeManager&) = delete;
    /** @brief 禁止拷贝赋值 */
    ProtocolBridgeManager& operator=(const ProtocolBridgeManager&) = delete;

    /** @brief 设置协议模式(切换时重置旧源、激活新源) @param mode 协议模式枚举 */
    void setProtocolMode(ChartProtocolMode mode);
    /** @brief 获取当前协议模式 @return 当前ChartProtocolMode枚举值 */
    ChartProtocolMode protocolMode() const;
    /** @brief 接收原始串口数据，路由到活动协议源 @param data 原始串口字节数据 */
    void feedData(const QByteArray& data);
    /** @brief 获取当前活动桥指针 @return 活动IProtocolBridge指针，FrameParser模式返回nullptr */
    IProtocolBridge* activeBridge() const;
    /** @brief 获取FrameParser指针 @return FrameParser指针 */
    FrameParser* frameParser() const;
    /** @brief 获取JustFloatBridge指针 @return JustFloatBridge指针 */
    JustFloatBridge* justFloatBridge() const;
    /** @brief 获取FireWaterBridge指针 @return FireWaterBridge指针 */
    FireWaterBridge* fireWaterBridge() const;

    // ---- 桥接器状态查询 ----
    /** @brief 获取活动桥接器运行时统计 @return BridgeStats结构体 */
    BridgeStats bridgeStats() const;
    /** @brief 查询活动桥接器是否在解析 @return true=正在解析 */
    bool isParsing() const;
    /** @brief 获取活动桥接器累计成功解析帧数 @return 解析帧计数 */
    quint64 totalFramesParsed() const;
    /** @brief 获取活动桥接器累计解析错误次数 @return 错误计数 */
    quint64 totalErrors() const;
    /** @brief 获取活动桥接器累计校验错误(仅FrameParser) @return 校验错误计数 */
    quint64 checksumErrors() const;

    // ---- 管理器级统计 ----
    /** @brief 获取累计桥接器切换次数 @return 切换计数 */
    quint64 totalBridges() const;
    /** @brief 获取所有协议源累计解析帧数 @return 总解析帧计数 */
    quint64 totalFramesParsedAll() const;
    /** @brief 获取所有协议源累计解析错误数 @return 总错误计数 */
    quint64 totalParseErrors() const;
    /** @brief 获取累计处理字节总数 @return 总字节数 */
    quint64 totalBytesProcessed() const;

    // ---- 每协议统计 ----
    /** @brief 获取指定协议累计统计 @param mode 协议模式 @return 该协议的ProtocolStats */
    ProtocolStats protocolStats(ChartProtocolMode mode) const;
    /** @brief 获取所有协议统计汇总 @return 模式到统计的QMap */
    QMap<ChartProtocolMode, ProtocolStats> allProtocolStats() const;

    // ---- 吞吐量 ----
    /** @brief 获取当前吞吐量快照(1秒滑动窗口) @return ThroughputSnapshot结构体 */
    ThroughputSnapshot throughput() const;

    // ---- 自动检测 ----
    /** @brief 从数据流自动检测协议类型(JustFloat/FireWater/FrameParser) @param data 采样数据(≥64字节) @return 检测结果 */
    AutoDetectResult detectProtocol(const QByteArray& data) const;
    /** @brief 启用/禁用自动检测(feedData时自动推断) @param enable true=启用 */
    void setAutoDetectEnabled(bool enable);
    /** @brief 查询自动检测开关 @return true=自动检测已启用 */
    bool isAutoDetectEnabled() const;
    /** @brief 获取最后一次自动检测结果 @return AutoDetectResult结构体 */
    AutoDetectResult lastAutoDetectResult() const;

    // ---- 运行时动态切换 ----
    /** @brief 动态切换活跃桥接器(不中断数据流、不重置旧源状态，适合快速切换) */
    void switchActiveBridge(ChartProtocolMode mode);

    /** @brief 重置所有统计，不影响运行状态 */
    void resetStats();

signals:
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame); ///< 转发活动源帧解析成功
    void frameError(const QString& reason, const QByteArray& rawFrame); ///< 帧解析错误(仅FrameParser)
    void protocolModeChanged(ChartProtocolMode mode);          ///< 协议模式切换通知
    void autoDetectCompleted(const AutoDetectResult& result);  ///< 自动检测完成信号

private:
    void switchSource();                                       ///< 切换数据源信号连接
    void onFrameParserParsed(const QVariantMap& f, const QByteArray& r); ///< FrameParser解析成功槽
    void onFrameParserError(const QString& r, const QByteArray& d); ///< FrameParser解析错误槽
    void onBridgeParsed(const QVariantMap& f, const QByteArray& r); ///< 桥接器解析成功槽
    double scoreJustFloat(const QByteArray& data) const;       ///< JustFloat尾部标记匹配度评分
    double scoreFireWater(const QByteArray& data) const;       ///< FireWater CSV特征匹配度评分
    void updateThroughput(quint64 frameBytes);                 ///< 更新吞吐量滑动窗口

    // ---- 核心资源 ----
    FrameParser* m_frameParser;                                 ///< 帧解析器(直接持有)
    JustFloatBridge* m_justFloat;                               ///< JustFloat协议桥
    FireWaterBridge* m_fireWater;                               ///< FireWater协议桥
    IProtocolBridge* m_activeBridge;                            ///< 当前活动桥(FrameParser模式为nullptr)
    ChartProtocolMode m_mode;                                   ///< 当前协议模式

    // ---- 桥接器统计 ----
    quint64 m_totalFramesParsed = 0; quint64 m_totalErrors = 0; quint64 m_checksumErrors = 0;
    quint64 m_totalBridges = 0; quint64 m_totalFramesParsedAll = 0;
    quint64 m_totalParseErrors = 0; quint64 m_totalBytesProcessed = 0;

    // ---- 每协议统计/吞吐量/自动检测 ----
    QMap<ChartProtocolMode, ProtocolStats> m_protocolStats;     ///< 各协议独立统计
    QElapsedTimer m_throughputTimer;                            ///< 吞吐量计时器
    quint64 m_throughputFrameCount = 0; quint64 m_throughputByteCount = 0;
    mutable ThroughputSnapshot m_lastThroughput;                ///< 吞吐量快照缓存
    bool m_autoDetectEnabled = false; AutoDetectResult m_lastDetectResult; QByteArray m_autoDetectBuffer;
    static constexpr int kAutoDetectMinBytes = 16;
    static constexpr int kAutoDetectMaxBytes = 512;
};

#endif // PROTOCOLBRIDGEMANAGER_H
