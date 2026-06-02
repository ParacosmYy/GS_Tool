/**
 * @file TriggerListPanel.cpp
 * @brief 触发器规则列表面板实现 — 规则展示与用户交互
 */

#include "automation/TriggerListPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>

/**
 * @brief 构造函数
 * @param parent 父控件
 */
TriggerListPanel::TriggerListPanel(QWidget* parent)
    : QWidget(parent)
    , m_ruleList(nullptr)
    , m_addBtn(nullptr)
    , m_removeBtn(nullptr)
{
    setObjectName(QStringLiteral("TriggerListPanel"));
    setupUI();
}

/**
 * @brief 设置要显示的规则列表
 *
 * 清空列表控件，将每条规则添加为列表项。
 * 显示格式: "[启用/禁用] 规则名称 - 匹配模式"
 *
 * @param rules 规则配置列表
 */
void TriggerListPanel::setRules(const QList<TriggerRuleConfig>& rules)
{
    m_ruleList->clear();

    for (const TriggerRuleConfig& rule : rules) {
        const QString status = rule.enabled ? tr("启用") : tr("禁用");

        /* 匹配模式的中文显示 */
        QString modeText;
        switch (rule.matchMode) {
        case MatchMode::ExactString:  modeText = tr("精确匹配"); break;
        case MatchMode::Regex:        modeText = tr("正则表达式"); break;
        case MatchMode::HexBytes:     modeText = tr("十六进制"); break;
        case MatchMode::ValueRange:   modeText = tr("数值范围"); break;
        }

        const QString display = QStringLiteral("[%1] %2 - %3")
                                    .arg(status, rule.name, modeText);

        auto* item = new QListWidgetItem(display, m_ruleList);
        item->setData(Qt::UserRole, rule.name);
    }
}

/**
 * @brief 初始化 UI 布局和控件
 *
 * 布局结构:
 *   - 上方: QListWidget 显示规则列表
 *   - 下方: 水平排列的添加/移除按钮
 *
 * 信号连接:
 *   - addBtn → addRuleRequested()
 *   - removeBtn → removeRuleRequested(currentRow)
 */
void TriggerListPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    /* 规则列表 */
    m_ruleList = new QListWidget(this);
    m_ruleList->setObjectName(QStringLiteral("ruleList"));
    mainLayout->addWidget(m_ruleList);

    /* 按钮栏 */
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(4);

    m_addBtn = new QPushButton(tr("➕"), this);
    m_addBtn->setObjectName(QStringLiteral("addRuleBtn"));
    m_addBtn->setToolTip(tr("添加新规则"));

    m_removeBtn = new QPushButton(tr("➖"), this);
    m_removeBtn->setObjectName(QStringLiteral("removeRuleBtn"));
    m_removeBtn->setToolTip(tr("移除选中规则"));

    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_removeBtn);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);

    /* 信号连接 */
    connect(m_addBtn, &QPushButton::clicked, this, &TriggerListPanel::addRuleRequested);

    connect(m_removeBtn, &QPushButton::clicked, this, [this]() {
        const int row = m_ruleList->currentRow();
        if (row >= 0) {
            emit removeRuleRequested(row);
        }
    });
}
