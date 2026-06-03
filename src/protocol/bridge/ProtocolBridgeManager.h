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
 *   6. 统计总桥接数、解析帧数、解析错误数、处理字节数
 *   7. 自动检测协议类型（从数据模式推断 JustFloat / FireWater / Raw）
 *   8. 每协议统计（各协议独立帧数/字节数/错误数）+ 吞吐量计算
 *
 * 设计模式: 策略模式(Strategy) -- 不同协议源是可互换的策略，Manager 是 Context
 * 业务层: 不依赖任何表现层类
 */

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

/**
 * @brief 协议桥管理器 - 统一管理帧解析模式和VOFA+协议模式的数据路由
 *
 * 持有三个数据源对象，根据当前模式将串口数据路由到对应的数据源。
 * 转发当前活动源的 frameParsed/frameError 信号，下游组件只需连接 Manager 的信号。
 * 支持从数据流模式自动推断协议类型，以及每协议独立统计和吞吐量计算。
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

    /** @brief 单个协议的累计统计，用于每协议统计报表 */
    struct ProtocolStats {
        quint64 frames = 0;     ///< 该协议累计成功解析帧数
        quint64 bytes = 0;      ///< 该协议累计处理字节数
        quint64 errors = 0;     ///< 该协议累计解析错误次数
    };

    /** @brief 吞吐量快照，用于实时速率显示 */
    struct ThroughputSnapshot {
        double framesPerSec = 0.0;   ///< 当前帧率(帧/秒)
        double bytesPerSec = 0.0;    ///< 当前字节吞吐率(字节/秒)
    };

    /** @brief 自动检测结果，包含推断的模式和置信度 */
    struct AutoDetectResult {
        ChartProtocolMode detectedMode = ChartProtocolMode::FrameParser;///< 检测到的模式
        double confidence = 0.0;       ///< 置信度 [0.0, 1.0]
        bool detected = false;         ///< 是否已完成检测
    };

    /** @brief 构造，默认使用 FrameParser 模式 */
    explicit ProtocolBridgeManager(FrameParser* frameParser, QObject* parent = nullptr);

    /** @brief 析构，QObject 父子树自动销毁持有的桥对象 */
    ~ProtocolBridgeManager() override = default;

    /** @brief 禁止拷贝构造 */
    ProtocolBridgeManager(const ProtocolBridgeManager&) = delete;
    /** @brief 禁止拷贝赋值 */
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

    // ---- 管理器级统计接口 ----

    /** @brief 获取累计创建/移除的桥接器切换总次数 */
    quint64 totalBridges() const;

    /** @brief 获取所有协议源累计解析的总帧数（含所有模式） */
    quint64 totalFramesParsedAll() const;

    /** @brief 获取所有协议源累计解析错误总数（含所有模式） */
    quint64 totalParseErrors() const;

    /** @brief 获取累计处理的字节总数 */
    quint64 totalBytesProcessed() const;

    // ---- 每协议统计接口 ----

    /** @brief 获取指定协议的累计统计 @param mode 协议模式 @return 该协议的帧数/字节/错误 */
    ProtocolStats protocolStats(ChartProtocolMode mode) const;

    /** @brief 获取所有协议的统计汇总 @return 模式→统计的Map */
    QMap<ChartProtocolMode, ProtocolStats> allProtocolStats() const;

    // ---- 吞吐量接口 ----

    /** @brief 获取当前吞吐量快照(基于最近1秒的滑动窗口) @return 帧率和字节率 */
    ThroughputSnapshot throughput() const;

    // ---- 自动检测接口 ----

    /**
     * @brief 从数据流自动检测协议类型
     *
     * 检测逻辑:
     *   - JustFloat: 尾部标记 00 00 80 7F 出现在 4 字节对齐位置
     *   - FireWater: 以 \n 结尾的 ASCII 可打印文本，含逗号/制表符分隔
     *   - FrameParser: 无法匹配上述特征时回退
     *
     * @param data 采样数据（建议至少 64 字节）
     * @return 检测结果（模式+置信度）
     */
    AutoDetectResult detectProtocol(const QByteArray& data) const;

    /**
     * @brief 启用/禁用自动检测模式
     *
     * 启用后，feedData() 在未确定模式时会自动尝试检测并切换。
     * 检测完成或用户手动切换后停止自动检测。
     *
     * @param enable true 启用自动检测
     */
    void setAutoDetectEnabled(bool enable);

    /** @brief 查询自动检测是否启用 @return true 自动检测已启用 */
    bool isAutoDetectEnabled() const;

    /** @brief 获取最后一次自动检测结果 @return 检测结果快照 */
    AutoDetectResult lastAutoDetectResult() const;

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

    /** @brief 自动检测完成信号 @param result 检测结果 */
    void autoDetectCompleted(const AutoDetectResult& result);

private:
    /** @brief 切换数据源信号连接 */
    void switchSource();

    /** @brief FrameParser 帧解析成功槽（累加统计后转发） */
    void onFrameParserParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    /** @brief FrameParser 帧解析错误槽（累加校验错误后转发） */
    void onFrameParserError(const QString& reason, const QByteArray& rawFrame);

    /** @brief 桥接器帧解析成功槽（累加帧计数后转发） */
    void onBridgeParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    /** @brief 自动检测内部实现: 分析 JustFloat 尾部标记匹配度 @return 匹配分值 [0, 1.0] */
    double scoreJustFloat(const QByteArray& data) const;

    /** @brief 自动检测内部实现: 分析 FireWater CSV 文本特征匹配度 @return 匹配分值 [0, 1.0] */
    double scoreFireWater(const QByteArray& data) const;

    /** @brief 更新吞吐量滑动窗口 @param frameBytes 本次帧字节数 */
    void updateThroughput(quint64 frameBytes);

    FrameParser* m_frameParser;         ///< 帧解析器（非桥，直接持有）
    JustFloatBridge* m_justFloat;       ///< JustFloat协议桥
    FireWaterBridge* m_fireWater;       ///< FireWater协议桥
    IProtocolBridge* m_activeBridge;    ///< 当前活动的桥（FrameParser模式下为nullptr）
    ChartProtocolMode m_mode;           ///< 当前协议模式

    quint64 m_totalFramesParsed = 0;    ///< 累计成功解析帧数
    quint64 m_totalErrors = 0;          ///< 累计解析错误次数
    quint64 m_checksumErrors = 0;       ///< 累计校验错误次数

    quint64 m_totalBridges = 0;         ///< 累计桥接器切换次数
    quint64 m_totalFramesParsedAll = 0; ///< 所有模式累计解析帧数
    quint64 m_totalParseErrors = 0;     ///< 所有模式累计解析错误数
    quint64 m_totalBytesProcessed = 0;  ///< 累计处理的字节总数

    // ---- 每协议统计 ----
    QMap<ChartProtocolMode, ProtocolStats> m_protocolStats;  ///< 各协议独立统计

    // ---- 吞吐量计算 ----
    QElapsedTimer m_throughputTimer;        ///< 吞吐量计算基准计时器
    quint64 m_throughputFrameCount = 0;     ///< 滑动窗口内帧计数
    quint64 m_throughputByteCount = 0;      ///< 滑动窗口内字节计数
    mutable ThroughputSnapshot m_lastThroughput; ///< 最近一次吞吐量快照缓存

    // ---- 自动检测 ----
    bool m_autoDetectEnabled = false;       ///< 自动检测开关
    AutoDetectResult m_lastDetectResult;    ///< 最近一次自动检测结果
    QByteArray m_autoDetectBuffer;          ///< 自动检测采样缓冲区
    static constexpr int kAutoDetectMinBytes = 16;  ///< 自动检测最少采样字节数
    static constexpr int kAutoDetectMaxBytes = 512; ///< 自动检测最大采样字节数
};

#endif // PROTOCOLBRIDGEMANAGER_H
