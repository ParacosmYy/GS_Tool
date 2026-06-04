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
 * @brief 触发器管理器统计计数器
 *
 * 追踪规则生命周期、触发命中、动作执行和管理器级操作的累计指标。
 */
struct TriggerManagerStats {
    quint64 totalRulesAdded = 0;       ///< 累计添加规则次数
    quint64 totalRulesRemoved = 0;     ///< 累计移除规则次数
    quint64 totalRuleUpdates = 0;      ///< 累计更新规则次数
    quint64 totalTriggersFired = 0;    ///< 累计触发器命中次数
    quint64 totalActionsExecuted = 0;  ///< 累计动作执行次数
    quint64 totalRuleImports = 0;      ///< 累计规则导入次数
    quint64 totalRuleExports = 0;      ///< 累计规则导出次数
    int     activeRuleCount = 0;       ///< 当前活跃（已启用）规则数
    double  avgResponseTimeMs = 0.0;   ///< 从命中到动作执行完成的平均响应时间(毫秒)
};

/**
 * @brief 触发器管理器
 *
 * 门面模式，整合引擎和动作执行器。
 * 提供规则的 CRUD 操作和 JSON 文件持久化。
 */
class TriggerManager : public QObject {
    Q_OBJECT

public:
    /** @brief 构造触发器管理器 @param parent 父对象 */
    explicit TriggerManager(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~TriggerManager() override;

    /** @brief 从文件加载规则 @param filePath JSON文件路径 @return true=加载成功，false=加载失败 */
    bool loadRules(const QString& filePath);

    /** @brief 保存规则到文件 @param filePath JSON文件路径 @return true=保存成功，false=保存失败 */
    bool saveRules(const QString& filePath);

    /** @brief 获取所有规则 @return 规则配置列表 */
    QList<TriggerRuleConfig> rules() const;

    /** @brief 添加一条规则 @param rule 规则配置 */
    void addRule(const TriggerRuleConfig& rule);

    /** @brief 移除指定索引的规则 @param index 规则索引 */
    void removeRule(int index);

    /** @brief 设置指定规则的启用/禁用状态 @param index 规则索引 @param enabled true=启用，false=禁用 */
    void setRuleEnabled(int index, bool enabled);

    /** @brief 更新指定索引的规则配置 @param index 规则索引 @param rule 新的规则配置 */
    void updateRule(int index, const TriggerRuleConfig& rule);

    /** @brief 获取累计匹配次数 @return 所有规则的总命中次数 */
    int matchCount() const;

    /** @brief 获取上次匹配距现在的毫秒数 @return 距上次匹配的毫秒数，无匹配返回-1 */
    qint64 msSinceLastMatch() const;

    /** @brief 重置引擎统计计数 */
    void resetStatistics();

    // ---- Stats struct 接口 ----

    /** @brief 获取统计计数器只读引用 @return 当前统计快照 */
    const TriggerManagerStats& stats() const { return m_stats; }

    /** @brief 重置所有管理器统计计数器为初始值 */
    void resetStats() { m_stats = TriggerManagerStats{}; }

    // ---- 兼容性 getter（委托给 m_stats） ----

    /** @brief 获取累计添加规则次数 @return 添加次数 */
    quint64 totalRulesAdded() const { return m_stats.totalRulesAdded; }

    /** @brief 获取累计移除规则次数 @return 移除次数 */
    quint64 totalRulesRemoved() const { return m_stats.totalRulesRemoved; }

    /** @brief 获取累计更新规则次数 @return 更新次数 */
    quint64 totalRuleUpdates() const { return m_stats.totalRuleUpdates; }

    /** @brief 获取累计修改规则次数(同totalRuleUpdates) @return 修改次数 */
    quint64 totalRulesModified() const { return m_stats.totalRuleUpdates; }

    /** @brief 获取累计触发器命中次数 @return 命中次数 */
    quint64 totalTriggersFired() const { return m_stats.totalTriggersFired; }

    /** @brief 获取累计动作执行次数 @return 执行次数 */
    quint64 totalActionsExecuted() const { return m_stats.totalActionsExecuted; }

    /** @brief 获取累计错误次数(正则编译失败等) @return 错误次数 */
    quint64 totalErrors() const;

    /** @brief 获取累计规则导入次数 @return 导入次数 */
    quint64 totalRuleImports() const { return m_stats.totalRuleImports; }

    /** @brief 获取累计规则导出次数 @return 导出次数 */
    quint64 totalRuleExports() const { return m_stats.totalRuleExports; }

    /** @brief 重置管理器统计计数器为初始值（兼容旧接口） */
    void resetManagerStatistics() { resetStats(); }

    /** @brief 更新规则列表UI @param panel 触发器列表面板指针 */
    void syncListPanel(class TriggerListPanel* panel);

signals:
    /** @brief 规则列表变更信号 */
    void rulesChanged();

private:
    TriggerEngine* m_engine = nullptr;   ///< 触发器引擎实例
    TriggerAction* m_action = nullptr;   ///< 动作执行器实例

    TriggerManagerStats m_stats;         ///< 管理器统计计数器
    /* totalErrors() 委托给 TriggerEngine::totalErrors()，无需独立成员 */
};

#endif // TRIGGERMANAGER_H
