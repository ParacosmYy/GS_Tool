/**
 * @file PortBridge.h
 * @brief 多端口桥接引擎 — 在不同连接之间转发数据包
 *
 * PortBridge 维护一组 BridgeRule，根据规则定义的方向和过滤器
 * 将源连接的数据转发到目标连接。
 *
 * 协作关系:
 *   - BridgeTypes: 规则/过滤器/统计结构体
 *   - BridgeConfigPanel: UI 编辑桥接规则
 *   - ConnectionController: 提供 feedData 的数据来源
 */
#ifndef PORTBRIDGE_H
#define PORTBRIDGE_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QString>
#include <QByteArray>
#include "connection/bridge/BridgeTypes.h"

/**
 * @brief 多端口桥接引擎
 *
 * 管理桥接规则的增删改查，执行数据转发与过滤，维护统计计数器。
 * 外部通过 feedData() 注入数据，PortBridge 根据匹配的规则执行转发。
 */
class PortBridge : public QObject {
    Q_OBJECT

public:
    /** @brief 构造桥接引擎 @param parent 父对象 */
    explicit PortBridge(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~PortBridge() override = default;

    // ---- 规则管理 ----

    /**
     * @brief 添加一条桥接规则
     * @param rule 规则配置（name 必须唯一）
     * @return true=添加成功，false=名称重复或参数无效
     */
    bool addBridge(const BridgeRule& rule);

    /**
     * @brief 按规则名称移除桥接规则
     * @param ruleName 规则名称
     * @return true=移除成功，false=规则不存在
     */
    bool removeBridge(const QString& ruleName);

    /** @brief 清空所有桥接规则并重置统计 */
    void clearBridges();

    /** @brief 获取所有桥接规则 @return 规则列表 */
    QVector<BridgeRule> bridges() const;

    /**
     * @brief 启用指定名称的桥接规则
     * @param ruleName 规则名称
     * @return true=成功，false=规则不存在
     */
    bool enableBridge(const QString& ruleName);

    /**
     * @brief 禁用指定名称的桥接规则
     * @param ruleName 规则名称
     * @return true=成功，false=规则不存在
     */
    bool disableBridge(const QString& ruleName);

    // ---- 数据转发 ----

    /**
     * @brief 向桥接引擎注入数据
     * @param sourceId 数据来源连接的唯一标识
     * @param data 接收到的原始字节数据
     *
     * 遍历所有已启用的规则，匹配 sourceId 后根据方向和过滤器
     * 决定是否转发。转发通过 dataForwarded 信号通知外部写入目标连接。
     */
    void feedData(const QString& sourceId, const QByteArray& data);

    // ---- 过滤评估 ----

    /**
     * @brief 评估数据是否通过过滤器链
     * @param data 待检查的数据
     * @param filters 过滤器列表（AND 逻辑）
     * @return true=通过所有过滤器，false=被拦截
     */
    bool passesFilter(const QByteArray& data, const QVector<BridgeFilter>& filters) const;

    // ---- 统计（单规则查询委托给 PortBridgeStats.cpp） ----

    /** @brief 获取所有规则的合并统计 @return 累计统计快照 */
    BridgeStats totalStatistics() const;

    /**
     * @brief 获取指定规则的统计
     * @param ruleName 规则名称
     * @return 该规则的统计快照，规则不存在则返回全零
     */
    BridgeStats bridgeStatistics(const QString& ruleName) const;

    /** @brief 获取当前活跃（已启用）桥接规则数 @return 活跃规则数 */
    quint64 totalBridgesActive() const;

    /** @brief 获取全局累计转发字节数 @return 字节数 */
    quint64 totalBytesForwarded() const;
    /** @brief 获取全局累计过滤字节数 @return 字节数 */
    quint64 totalBytesFiltered() const;
    /** @brief 获取全局累计丢弃字节数 @return 字节数 */
    quint64 totalBytesDropped() const;
    /** @brief 获取全局累计错误次数 @return 错误数 */
    quint64 totalErrors() const;

    /** @brief 重置所有规则的统计计数器归零 */
    void resetStatistics();

signals:
    /**
     * @brief 数据已转发信号
     * @param from 源连接 ID
     * @param to 目标连接 ID
     * @param bytes 转发的字节数
     */
    void dataForwarded(const QString& from, const QString& to, int bytes);

    /**
     * @brief 数据被过滤器拦截信号
     * @param rule 拦截此数据的规则名称
     * @param bytes 被拦截的字节数
     */
    void dataFiltered(const QString& rule, int bytes);

    /**
     * @brief 桥接错误信号
     * @param rule 发生错误的规则名称
     * @param error 错误描述
     */
    void bridgeError(const QString& rule, const QString& error);

private:
    /**
     * @brief 在规则列表中查找指定名称的规则索引
     * @param ruleName 规则名称
     * @return 索引，不存在返回 -1
     */
    int findRuleIndex(const QString& ruleName) const;

    /**
     * @brief 对单条规则执行单方向的数据转发评估
     * @param ruleIdx 规则索引
     * @param fromId 数据来源 ID
     * @param toId 转发目标 ID
     * @param data 待转发数据
     */
    void evaluateAndForward(int ruleIdx, const QString& fromId,
                            const QString& toId, const QByteArray& data);

    QVector<BridgeRule> m_rules;              ///< 桥接规则列表
    QMap<QString, BridgeStats> m_ruleStats;   ///< 规则名称到统计的映射
};

#endif // PORTBRIDGE_H
