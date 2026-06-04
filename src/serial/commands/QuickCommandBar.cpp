/**
 * @file QuickCommandBar.cpp
 * @brief 快捷指令栏实现 - UI构建
 *
 * 指令列表管理、持久化存储与统计查询已拆分至 QuickCommandBarActions.cpp。
 * 按钮重建、编辑对话框已拆分至 QuickCommandBarDialog.cpp。
 */

#include "serial/commands/QuickCommandBar.h"
#include "core/widgets/AnimatedButton.h"

#include <QHBoxLayout>

/** @brief 构造快捷指令栏(加载保存的指令+构建按钮行) @param parent 父控件 */
QuickCommandBar::QuickCommandBar(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 6, 8, 6);
    mainLayout->setSpacing(6);

    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->setSpacing(6);
    mainLayout->addLayout(m_buttonLayout);

    mainLayout->addStretch();

    // ---- 编辑按钮 ----
    // 使用 setMinimumHeight(32) 替代 setFixedSize，允许水平自适应内容
    // 与 SerialConfigPanel 连接按钮(36px)视觉协调: 次要操作按钮略矮
    m_editBtn = new AnimatedButton(tr("编辑"));
    m_editBtn->setObjectName("quickCmdEditBtn");
    m_editBtn->setMinimumHeight(32);
    connect(m_editBtn, &QPushButton::clicked, this, &QuickCommandBar::onEditRequested);
    mainLayout->addWidget(m_editBtn);

    // ---- 添加按钮 ----
    // 使用 setMinimumHeight(32) 替代 setFixedSize(32,32)，保持与其他按钮等高
    m_addBtn = new AnimatedButton(tr("+"));
    m_addBtn->setObjectName("quickCmdAddBtn");
    m_addBtn->setMinimumHeight(32);
    connect(m_addBtn, &QPushButton::clicked, this, [this]() {
        QuickCommand cmd{tr("指令"), "", false};
        addCommand(cmd);
    });
    mainLayout->addWidget(m_addBtn);

    setObjectName("quickCommandBar");
}

// setCommands/addCommand/clearCommands 已移至 QuickCommandBarActions.cpp
// rebuildButtons/onEditRequested/createEditDialog/populateDialogFields 已移至 QuickCommandBarDialog.cpp
// saveCommands/loadCommands/totalCommandsSent/totalQuickSends/maxCommandLength/resetStatistics 已移至 QuickCommandBarActions.cpp
