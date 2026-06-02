/**
 * @file TriggerEngine.h
 * @brief 触发器引擎 — 对接收数据进行规则匹配评估
 *
 * 负责将接收到的数据（原始字节或解析后的数值）与已注册的触发器规则
 * 进行匹配评估，命中时发出 triggered 信号。
 *
 * 协作关系:
 *   - TriggerManager: 管理规则集合和引擎生命周期
 *   - TriggerAction: 响应 triggered 信号执行动作
 *   - IConnection: 通过 dataReceived 信号输入待评估数据
 */
#ifndef TRIGGERENGINE_H
#define TRIGGERENGINE_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include "automation/TriggerRule.h"

/**
 * @brief 触发器引擎
 *
 * 维护规则列表，对外提供数据评估接口。
 * 支持对原始字节数据和解析后数值分别进行匹配。
 */
class TriggerEngine : public QObject {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit TriggerEngine(QObject* parent = nullptr);

    /**
     * @brief 评估原始字节数据
     *
     * 将数据与所有已启用规则进行匹配（ExactString/Regex/HexBytes）。
     *
     * @param data 待评估的原始数据
     */
    void evaluateData(const QByteArray& data);

    /**
     * @brief 评估解析后的数值
     *
     * 将数值与 ValueRange 类型的规则进行匹配。
     *
     * @param name 数据标识（如通道名/变量名）
     * @param value 数值
     */
    void evaluateValue(const QString& name, double value);

    /**
     * @brief 添加一条触发器规则
     * @param rule 规则配置
     */
    void addRule(const TriggerRuleConfig& rule);

    /**
     * @brief 移除指定索引的规则
     * @param index 规则索引
     */
    void removeRule(int index);

    /**
     * @brief 设置所有规则的启用/禁用状态
     * @param enabled true 启用，false 禁用
     */
    void setRulesEnabled(bool enabled);

    /**
     * @brief 设置指定规则的启用/禁用状态
     * @param index 规则索引
     * @param enabled true 启用，false 禁用
     */
    void setRuleEnabled(int index, bool enabled);

    /**
     * @brief 获取所有规则列表
     * @return 规则配置列表的常引用
     */
    const QList<TriggerRuleConfig>& rules() const;

    /**
     * @brief 清空所有规则
     */
    void clearRules();

    /**
     * @brief 获取累计成功匹配次数
     * @return 匹配次数
     */
    int matchCount() const;

signals:
    /**
     * @brief 触发器命中信号
     * @param ruleIndex 命中规则的索引
     * @param ruleName 命中规则的名称
     */
    void triggered(int ruleIndex, const QString& ruleName);

    /**
     * @brief 动作请求信号
     * @param actionType 动作类型（对应 ActionType 枚举值）
     * @param actionData 动作附加数据
     */
    void actionRequired(int actionType, const QByteArray& actionData);

private:
    QList<TriggerRuleConfig> m_rules;   ///< 规则列表
    bool m_enabled = true;              ///< 全局启用标志
    int m_matchCount = 0;               ///< 累计匹配计数
};

#endif // TRIGGERENGINE_H
