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
 * @brief 触发器规则列表面板
 *
 * 显示规则列表，每条规则显示名称、匹配模式和启用状态。
 * 提供添加和移除按钮。
 */
class TriggerListPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造函数 */
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

    // ---- 统计计数器 ----
    quint64 m_totalRuleEdits = 0;   ///< 累计规则编辑次数
    quint64 m_totalRuleToggles = 0; ///< 累计规则启停切换次数
    quint64 m_totalRuleReorders = 0;///< 累计规则重排次数
public:
    quint64 totalRuleEdits() const { return m_totalRuleEdits; }
    quint64 totalRuleToggles() const { return m_totalRuleToggles; }
    quint64 totalRuleReorders() const { return m_totalRuleReorders; }
    void resetTriggerListStatistics() { m_totalRuleEdits = 0; m_totalRuleToggles = 0; m_totalRuleReorders = 0; }
};

#endif // TRIGGERLISTPANEL_H
