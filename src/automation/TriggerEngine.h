/** @file TriggerEngine.h @brief 触发器引擎 -- 对接收数据进行规则匹配评估。维护规则列表，支持原始字节(ExactString/Regex/HexBytes)和解析后数值(ValueRange)匹配 */
#ifndef TRIGGERENGINE_H
#define TRIGGERENGINE_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QElapsedTimer>
#include "automation/TriggerRule.h"

/**
 * @brief 触发器引擎
 * 维护规则列表，对外提供数据评估接口。命中时发出triggered信号。
 * 协作: TriggerManager(管理规则集合) / TriggerAction(执行动作) / IConnection(数据输入)
 */
class TriggerEngine : public QObject {
    Q_OBJECT

public:
    /** @brief 构造触发器引擎 @param parent 父对象 */
    explicit TriggerEngine(QObject* parent = nullptr);
    /** @brief 评估原始字节数据(ExactString/Regex/HexBytes) @param data 待评估数据 */
    void evaluateData(const QByteArray& data);
    /** @brief 评估解析后数值(ValueRange) @param name 数据标识 @param value 数值 */
    void evaluateValue(const QString& name, double value);
    /** @brief 添加触发器规则 @param rule 规则配置 */
    void addRule(const TriggerRuleConfig& rule);
    /** @brief 替换指定索引的规则(保持位置不变) @param index 规则索引 @param rule 新规则配置 */
    void replaceRule(int index, const TriggerRuleConfig& rule);
    /** @brief 移除指定索引规则 @param index 规则索引 */
    void removeRule(int index);
    /** @brief 设置所有规则启用/禁用 @param enabled true=启用所有规则 */
    void setRulesEnabled(bool enabled);
    /** @brief 设置指定规则启用/禁用 @param index 规则索引 @param enabled 启用状态 */
    void setRuleEnabled(int index, bool enabled);
    /** @brief 获取所有规则列表 @return 规则配置列表的const引用 */
    const QList<TriggerRuleConfig>& rules() const;
    /** @brief 清空所有规则 */
    void clearRules();
    /** @brief 获取累计成功匹配次数 @return 匹配计数 */
    int matchCount() const;
    /** @brief 上次匹配距现在的毫秒数 @return 毫秒数，无匹配返回-1 */
    qint64 msSinceLastMatch() const;
    /** @brief 重置统计(不重置规则) */
    void resetStatistics();
    /** @brief 获取指定规则的匹配次数 @param index 规则索引 @return 该规则命中次数 */
    int ruleMatchCount(int index) const;
    // ---- 扩展统计 getter ----
    /** @brief 获取总评估次数(evaluateData/evaluateValue) @return 评估总次数 */
    quint64 totalEvaluations() const;
    /** @brief 获取总匹配成功次数(quint64精度) @return 匹配总次数 */
    quint64 totalMatches() const;
    /** @brief 获取总动作执行次数 @return 动作执行总次数 */
    quint64 totalActionsExecuted() const;
    /** @brief 获取总错误次数(正则编译失败等) @return 错误总次数 */
    quint64 totalErrors() const;
    /** @brief 获取总规则评估次数(每条每次) @return 规则评估总次数 */
    quint64 totalTriggersEvaluated() const;
    /** @brief 获取总触发器命中次数 @return 触发器命中总次数 */
    quint64 totalTriggersFired() const;
    /** @brief 获取总跳过禁用规则次数 @return 跳过禁用规则总次数 */
    quint64 totalTriggersDisabled() const;
    /** @brief 获取总动作执行错误次数 @return 动作错误总次数 */
    quint64 totalActionErrors() const;
    /** @brief 获取累计规则激活(从禁用切到启用)次数 @return 激活总次数 */
    quint64 totalRulesActive() const;
    /** @brief 获取历史同时启用规则数峰值 @return 峰值规则数 */
    int peakRulesActive() const;
    /** @brief 重置所有扩展统计计数器 */
    void resetStats();

signals:
    /** @brief 触发器命中 @param ruleIndex 命中规则索引 @param ruleName 规则名称 */
    void triggered(int ruleIndex, const QString& ruleName);
    /** @brief 动作请求 @param actionType 动作类型(ActionType枚举值) @param actionData 附加数据 */
    void actionRequired(int actionType, const QByteArray& actionData);

private:
    QList<TriggerRuleConfig> m_rules;       ///< 规则列表
    QVector<int> m_ruleMatchCounts;         ///< 每条规则命中次数
    bool m_enabled = true;                  ///< 全局启用标志
    int m_matchCount = 0;                   ///< 累计匹配计数
    QElapsedTimer m_lastMatchTimer;         ///< 上次匹配时间
    bool m_hasMatched = false;              ///< 是否有过匹配
    // 扩展统计计数器
    quint64 m_totalEvaluations = 0, m_totalMatches = 0, m_totalActionsExecuted = 0;
    quint64 m_totalErrors = 0, m_totalTriggersEvaluated = 0, m_totalTriggersFired = 0;
    quint64 m_totalTriggersDisabled = 0, m_totalActionErrors = 0;
    quint64 m_totalRulesActive = 0;         ///< 累计规则激活次数
    int m_peakRulesActive = 0;              ///< 同时启用规则数峰值
};

#endif // TRIGGERENGINE_H
