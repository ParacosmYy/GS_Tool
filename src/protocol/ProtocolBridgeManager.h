/**
 * @file ProtocolBridgeManager.h
 * @brief 协议桥管理器 - 统一管理帧解析模式和VOFA+协议模式的数据路由
 *
 * 职责:
 *   1. 持有 FrameParser、JustFloatBridge、FireWaterBridge 三个数据源
 *   2. 根据 ChartProtocolMode 将原始串口数据路由到当前活动的数据源
 *   3. 转发当前活动源的 frameParsed 信号，下游 ChartModel/ProtocolView 无需知道数据来自哪个协议
 *   4. 模式切换时重置旧源、激活新源，发出 protocolModeChanged 信号通知 UI
 *
 * 数据流:
 *   ConnectionController::dataReceived
 *     -> ProtocolBridgeManager::feedData
 *       -> 当前活动源(FrameParser / JustFloatBridge / FireWaterBridge).feed()
 *         -> 源内部解析完成后 emit frameParsed(fields, rawFrame)
 *           -> ProtocolBridgeManager::frameParsed 转发
 *             -> ChartModel::onFrameParsed + ProtocolView::onFrameParsed
 *
 * 设计模式: 策略模式(Strategy) -- 不同协议源是可互换的策略，Manager 是 Context
 * 业务层: 不依赖任何表现层类
 */

#ifndef PROTOCOLBRIDGEMANAGER_H
#define PROTOCOLBRIDGEMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>

#include "protocol/FrameParser.h"
#include "protocol/IProtocolBridge.h"
#include "protocol/JustFloatBridge.h"
#include "protocol/FireWaterBridge.h"

/**
 * @brief 协议桥管理器 - 统一管理帧解析模式和VOFA+协议模式的数据路由
 *
 * 持有三个数据源对象，根据当前模式将串口数据路由到对应的数据源。
 * 转发当前活动源的 frameParsed/frameError 信号，下游组件只需连接 Manager 的信号。
 *
 * 协作关系:
 *   - ConnectionController: 上游数据源，调用 feedData() 送入串口数据
 *   - ChartModel / ProtocolView: 下游消费者，连接 frameParsed/frameError 信号
 *   - FrameParser / JustFloatBridge / FireWaterBridge: 被管理的协议解析策略
 */
class ProtocolBridgeManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 协议模式枚举
     *
     * 不同模式对应不同的数据解析策略:
     *   - FrameParser: 用户自定义帧格式（帧头/帧尾/长度/校验/字段）
     *   - JustFloat: VOFA+ 小端浮点字节流协议（固定通道数，4字节float）
     *   - FireWater: VOFA+ CSV 尾标记协议（以换行符分隔的数据行）
     */
    enum class ChartProtocolMode {
        FrameParser,    ///< 默认帧解析模式（用户自定义帧格式）
        JustFloat,      ///< VOFA+ JustFloat浮点字节流协议
        FireWater       ///< VOFA+ FireWater CSV尾标记协议
    };
    Q_ENUM(ChartProtocolMode)

    /**
     * @brief 构造协议桥管理器
     * @param frameParser 外部创建的帧解析器（如果尚未设置 parent 则归本对象管理）
     * @param parent 父对象（通常为 MainWindow）
     *
     * 默认使用 FrameParser 模式。初始连接 FrameParser 的信号。
     */
    explicit ProtocolBridgeManager(FrameParser* frameParser, QObject* parent = nullptr);

    /**
     * @brief 析构协议桥管理器
     *
     * QObject 父子树自动销毁持有的桥对象。
     */
    ~ProtocolBridgeManager() override;

    // 禁止拷贝和赋值（QObject派生类）
    ProtocolBridgeManager(const ProtocolBridgeManager&) = delete;
    ProtocolBridgeManager& operator=(const ProtocolBridgeManager&) = delete;

    /**
     * @brief 设置协议模式（切换时自动重置旧源、激活新源）
     * @param mode 目标协议模式
     *
     * 切换流程:
     *   1. 重置旧源的内部状态（清空缓冲区）
     *   2. 断开旧源的信号连接
     *   3. 连接新源的信号
     *   4. 发出 protocolModeChanged 信号通知 UI
     *
     * 如果 mode 与当前模式相同，不做任何操作。
     */
    void setProtocolMode(ChartProtocolMode mode);

    /**
     * @brief 获取当前协议模式
     * @return 当前活动协议模式
     */
    ChartProtocolMode protocolMode() const;

    /**
     * @brief 接收原始串口数据，转发到当前活动的协议源
     * @param data 原始字节数据
     *
     * 空数据保护: 传入空 QByteArray 时直接返回，不触发任何处理。
     * 无效模式保护: 理论上不会触发（枚举覆盖完整），但 default 分支会输出错误日志。
     */
    void feedData(const QByteArray& data);

    /**
     * @brief 获取当前活动的桥
     * @return 当前活动桥指针，FrameParser 模式下返回 nullptr
     *
     * 用于外部配置（如设置通道数、分隔符等）。
     */
    IProtocolBridge* activeBridge() const;

    /**
     * @brief 获取 FrameParser 指针
     * @return 帧解析器指针（用于帧编辑器设置定义等场景）
     */
    FrameParser* frameParser() const;

    /**
     * @brief 获取 JustFloatBridge 指针
     * @return JustFloat 协议桥指针（用于设置固定通道数等配置）
     */
    JustFloatBridge* justFloatBridge() const;

    /**
     * @brief 获取 FireWaterBridge 指针
     * @return FireWater 协议桥指针（用于设置分隔符等配置）
     */
    FireWaterBridge* fireWaterBridge() const;

signals:
    /**
     * @brief 转发当前活动源的 frameParsed 信号
     * @param fields 各通道名→值的映射
     * @param rawFrame 本帧的原始字节数据
     *
     * 无论哪种模式，下游只需连接此信号即可接收解析结果。
     */
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    /**
     * @brief 帧解析错误（仅 FrameParser 模式会产生此信号）
     * @param reason 错误原因描述
     * @param rawFrame 出错时的原始数据
     */
    void frameError(const QString& reason, const QByteArray& rawFrame);

    /**
     * @brief 协议模式切换时发出
     * @param mode 切换后的新协议模式
     *
     * 通知 UI 更新通道配置面板等。
     */
    void protocolModeChanged(ChartProtocolMode mode);

private:
    /**
     * @brief 切换数据源连接
     *
     * 断开所有源到本 Manager 转发的信号连接，然后仅连接当前活动源的信号。
     * 这样 Manager::frameParsed 始终转发的是当前活动源的解析结果。
     */
    void switchSource();

    FrameParser* m_frameParser;         ///< 帧解析器（非桥，直接持有）
    JustFloatBridge* m_justFloat;       ///< JustFloat协议桥
    FireWaterBridge* m_fireWater;       ///< FireWater协议桥
    IProtocolBridge* m_activeBridge;    ///< 当前活动的桥（FrameParser模式下为nullptr）
    ChartProtocolMode m_mode;           ///< 当前协议模式
};

#endif // PROTOCOLBRIDGEMANAGER_H
