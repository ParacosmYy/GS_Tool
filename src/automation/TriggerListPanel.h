/**
 * @file TriggerListPanel.h
 * @brief 触发器规则列表面板 — 展示和管理触发器规则的 UI 控件
 *
 * 以列表形式展示所有触发器规则，支持添加/移除/启用禁用操作。
 * 用户交互通过信号通知外部管理器执行实际操作。
 *
 * 协作关系:
 *   - TriggerManager: 响应面板信号执行规则 CRUD
 *   - TriggerRule: 规则数据结构
 */
#ifndef TRIGGERLISTPANEL_H
#define TRIGGERLISTPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include "automation/TriggerRule.h"

/**
 * @brief 触发器列表面板统计计数器
 *
 * 追踪面板展示、用户交互（编辑/删除/创建/选择）的累计指标。
 */
struct TriggerListPanelStats {
    quint64 totalDisplays = 0;       ///< 累计面板显示/刷新次数(setRules调用)
    quint64 totalRuleEdits = 0;      ///< 累计规则编辑次数
    quint64 totalRuleDeletes = 0;    ///< 累计规则删除请求次数
    quint64 totalRuleCreates = 0;    ///< 累计规则创建请求次数
    quint64 totalRuleReorders = 0;   ///< 累计规则重排次数(上移/下移)
    quint64 totalRuleToggles = 0;    ///< 累计规则启停切换次数
    quint64 selectionChanges = 0;    ///< 累计列表选择变化次数
};

/**
 * @brief 触发器规则列表面板
 *
 * 显示规则列表，每条规则显示名称、匹配模式和启用状态。
 * 提供添加和移除按钮。
 */
class TriggerListPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    /** @brief 构造触发器规则列表面板 @param parent 父控件 */
    explicit TriggerListPanel(QWidget* parent = nullptr);

    /**
     * @brief 设置要显示的规则列表
     * @param rules 规则配置列表
     */
    void setRules(const QList<TriggerRuleConfig>& rules);

    /**
     * @brief 更新规则计数标签
     * @param total 总规则数
     * @param enabled 启用的规则数
     */
    void updateRuleCount(int total, int enabled);

    // ---- Stats struct 接口 ----

    /** @brief 获取统计计数器只读引用 @return 当前统计快照 */
    const TriggerListPanelStats& stats() const { return m_stats; }

    /** @brief 重置所有面板统计计数器为初始值 */
    void resetStats() { m_stats = TriggerListPanelStats{}; }

    // ---- 兼容性 getter（委托给 m_stats） ----

    /** @brief 获取累计规则编辑次数 */
    quint64 totalRuleEdits() const { return m_stats.totalRuleEdits; }
    /** @brief 获取累计规则启停切换次数 */
    quint64 totalRuleToggles() const { return m_stats.totalRuleToggles; }
    /** @brief 获取累计规则重排次数 */
    quint64 totalRuleReorders() const { return m_stats.totalRuleReorders; }
    /** @brief 重置面板统计计数器归零（兼容旧接口） */
    void resetTriggerListStatistics() { resetStats(); }

signals:
    /** @brief 用户点击添加规则按钮信号 */
    void addRuleRequested();

    /**
     * @brief 用户请求移除指定索引的规则
     * @param index 规则索引
     */
    void removeRuleRequested(int index);

    /**
     * @brief 用户切换规则启用状态
     * @param index 规则索引
     * @param enabled true 启用，false 禁用
     */
    void ruleEnabledChanged(int index, bool enabled);

    /**
     * @brief 用户请求编辑指定索引的规则
     * @param index 规则索引
     */
    void editRuleRequested(int index);

    /**
     * @brief 用户请求上移指定索引的规则
     * @param index 规则索引
     */
    void moveUpRequested(int index);

    /**
     * @brief 用户请求下移指定索引的规则
     * @param index 规则索引
     */
    void moveDownRequested(int index);

private:
    /** @brief 初始化 UI 布局和控件 */
    void setupUI();

    QListWidget* m_ruleList;        ///< 规则列表控件
    QPushButton* m_addBtn;          ///< 添加规则按钮
    QPushButton* m_removeBtn;       ///< 移除规则按钮
    QPushButton* m_editBtn;         ///< 编辑规则按钮
    QPushButton* m_moveUpBtn;       ///< 上移规则按钮
    QPushButton* m_moveDownBtn;     ///< 下移规则按钮
    QLabel* m_countLabel;           ///< 规则计数标签

    TriggerListPanelStats m_stats;  ///< 面板统计计数器
};

#endif // TRIGGERLISTPANEL_H
