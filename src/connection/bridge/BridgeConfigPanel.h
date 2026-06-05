/**
 * @file BridgeConfigPanel.h
 * @brief 桥接规则配置面板 — UI 控件用于创建/编辑/删除桥接规则
 *
 * 提供源/目标连接选择、方向设置、过滤选项等 UI 控件。
 * 用户操作通过信号通知外部 PortBridge 执行实际规则变更。
 *
 * 协作关系:
 *   - PortBridge: 响应面板信号执行规则增删改
 *   - BridgeTypes: 规则/过滤器数据结构
 */
#ifndef BRIDGECONFIGPANEL_H
#define BRIDGECONFIGPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QCheckBox>
#include <QVector>
#include <QStringList>
#include "connection/bridge/BridgeTypes.h"

/**
 * @brief 桥接配置面板统计计数器
 *
 * 追踪用户通过面板执行规则增删改操作的累计指标。
 */
struct BridgeConfigPanelStats {
    quint64 totalRulesCreated = 0;   ///< 累计创建规则次数
    quint64 totalRulesModified = 0;  ///< 累计修改规则次数
    quint64 totalRulesDeleted = 0;   ///< 累计删除规则次数
};

/**
 * @brief 桥接规则配置面板
 *
 * 左侧为规则列表，右侧为规则编辑区（源/目标连接、方向、过滤）。
 * 通过信号通知外部执行实际的规则变更。
 */
class BridgeConfigPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造桥接配置面板 @param parent 父控件 */
    explicit BridgeConfigPanel(QWidget* parent = nullptr);

    /**
     * @brief 设置可用连接 ID 列表（填充源/目标下拉框）
     * @param ids 连接 ID 字符串列表
     */
    void setAvailableConnections(const QStringList& ids);

    /**
     * @brief 获取当前编辑区中的规则配置
     * @return 当前表单组装的 BridgeRule
     */
    BridgeRule currentRule() const;

    /**
     * @brief 获取面板中所有规则列表
     * @return 规则列表
     */
    QVector<BridgeRule> allRules() const;

    // ---- 统计接口（委托给 BridgeConfigPanelStats.cpp） ----

    /** @brief 获取累计创建规则次数 @return 创建次数 */
    quint64 totalRulesCreated() const;
    /** @brief 获取累计修改规则次数 @return 修改次数 */
    quint64 totalRulesModified() const;
    /** @brief 获取累计删除规则次数 @return 删除次数 */
    quint64 totalRulesDeleted() const;
    /** @brief 重置面板统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 用户添加新规则信号 @param rule 新建的规则配置 */
    void bridgeAdded(const BridgeRule& rule);

    /** @brief 用户移除规则信号 @param name 规则名称 */
    void bridgeRemoved(const QString& name);

    /** @brief 用户修改规则信号 @param rule 修改后的规则配置 */
    void bridgeModified(const BridgeRule& rule);

private:
    /** @brief 初始化 UI 布局和控件 */
    void setupUI();

    /** @brief 将当前编辑区表单内容填充到控件中（编辑已有规则时调用） */
    void populateForm(const BridgeRule& rule);

    /** @brief 从表单组装 BridgeRule 并重置编辑区 */
    void onAddClicked();

    /** @brief 移除列表中选中的规则 */
    void onRemoveClicked();

    /** @brief 将编辑区的内容应用到选中规则 */
    void onApplyClicked();

    /** @brief 切换选中规则的启用状态 */
    void onToggleClicked();

    /** @brief 列表选择变化时将规则加载到编辑区 */
    void onRuleSelected(int row);

    // ---- 控件 ----
    QListWidget* m_ruleList;         ///< 规则列表
    QLineEdit*   m_nameEdit;         ///< 规则名称输入
    QComboBox*   m_sourceCombo;      ///< 源连接选择
    QComboBox*   m_targetCombo;      ///< 目标连接选择
    QComboBox*   m_directionCombo;   ///< 方向选择
    QComboBox*   m_filterTypeCombo;  ///< 过滤类型选择
    QLineEdit*   m_filterPatternEdit;///< 过滤模式输入
    QCheckBox*   m_filterInclusiveCheck; ///< 过滤白名单/黑名单
    QPushButton* m_addBtn;           ///< 添加按钮
    QPushButton* m_removeBtn;        ///< 移除按钮
    QPushButton* m_applyBtn;         ///< 应用按钮
    QPushButton* m_toggleBtn;        ///< 启停切换按钮
    QLabel*      m_countLabel;       ///< 规则计数标签

    QVector<BridgeRule> m_rules;     ///< 面板持有的规则列表
    BridgeConfigPanelStats m_stats;  ///< 面板统计计数器
};

#endif // BRIDGECONFIGPANEL_H
