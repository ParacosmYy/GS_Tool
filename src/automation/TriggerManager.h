/**
 * @file TriggerManager.h
 * @brief 触发器管理器 — 统一管理触发器规则、引擎和动作的生命周期
 *
 * 作为触发器子系统的门面（Facade），对外提供规则增删改查和持久化接口，
 * 内部协调 TriggerEngine（匹配）和 TriggerAction（执行）。
 *
 * 协作关系:
 *   - TriggerEngine: 数据匹配评估
 *   - TriggerAction: 动作执行
 *   - TriggerListPanel: UI 编辑规则
 */
#ifndef TRIGGERMANAGER_H
#define TRIGGERMANAGER_H

#include <QObject>
#include <QList>
#include "automation/TriggerRule.h"

class TriggerEngine;
class TriggerAction;

/**
 * @brief 触发器管理器
 *
 * 门面模式，整合引擎和动作执行器。
 * 提供规则的 CRUD 操作和 JSON 文件持久化。
 */
class TriggerManager : public QObject {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit TriggerManager(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~TriggerManager() override;

    /**
     * @brief 从文件加载规则
     * @param filePath JSON 文件路径
     * @return true 加载成功，false 加载失败
     */
    bool loadRules(const QString& filePath);

    /**
     * @brief 保存规则到文件
     * @param filePath JSON 文件路径
     * @return true 保存成功，false 保存失败
     */
    bool saveRules(const QString& filePath);

    /**
     * @brief 获取所有规则
     * @return 规则配置列表
     */
    QList<TriggerRuleConfig> rules() const;

    /**
     * @brief 添加一条规则
     * @param rule 规则配置
     */
    void addRule(const TriggerRuleConfig& rule);

    /**
     * @brief 移除指定索引的规则
     * @param index 规则索引
     */
    void removeRule(int index);

    /**
     * @brief 设置指定规则的启用/禁用状态
     * @param index 规则索引
     * @param enabled true 启用，false 禁用
     */
    void setRuleEnabled(int index, bool enabled);

    /**
     * @brief 更新指定索引的规则配置
     * @param index 规则索引
     * @param rule 新的规则配置
     */
    void updateRule(int index, const TriggerRuleConfig& rule);

    /**
     * @brief 获取累计匹配次数
     * @return 所有规则的总命中次数
     */
    int matchCount() const;

    /**
     * @brief 获取上次匹配距现在的毫秒数
     * @return 距上次匹配的毫秒数，无匹配返回 -1
     */
    qint64 msSinceLastMatch() const;

    /**
     * @brief 重置引擎统计计数
     */
    void resetStatistics();

    // ---- 管理器统计 getter ----

    /** @brief 获取累计添加规则次数 */
    quint64 totalRulesAdded() const;

    /** @brief 获取累计移除规则次数 */
    quint64 totalRulesRemoved() const;

    /** @brief 获取累计更新规则次数 */
    quint64 totalRuleUpdates() const;

    /** @brief 重置管理器统计计数器为初始值 */
    void resetManagerStatistics();

    /**
     * @brief 更新规则列表UI
     * @param panel 触发器列表面板指针
     */
    void syncListPanel(class TriggerListPanel* panel);

signals:
    /** @brief 规则列表变更信号 */
    void rulesChanged();

private:
    TriggerEngine* m_engine = nullptr;   ///< 触发器引擎实例
    TriggerAction* m_action = nullptr;   ///< 动作执行器实例

    // 管理器统计计数器
    quint64 m_totalRulesAdded = 0;       ///< 累计添加规则次数
    quint64 m_totalRulesRemoved = 0;     ///< 累计移除规则次数
    quint64 m_totalRuleUpdates = 0;      ///< 累计更新规则次数
};

#endif // TRIGGERMANAGER_H
