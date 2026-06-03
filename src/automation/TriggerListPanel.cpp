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
    , m_editBtn(nullptr)
    , m_moveUpBtn(nullptr)
    , m_moveDownBtn(nullptr)
    , m_countLabel(nullptr)
{
    setObjectName(QStringLiteral("TriggerListPanel"));
    setupUI();
}

/** @brief 设置要显示的规则列表，清空后逐条添加为列表项 @param rules 规则配置列表 */
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
        item->setCheckState(rule.enabled ? Qt::Checked : Qt::Unchecked);
    }
}

/**
 * @brief 更新规则计数标签
 * @param total 总规则数
 * @param enabled 启用的规则数
 */
void TriggerListPanel::updateRuleCount(int total, int enabled)
{
    m_countLabel->setText(tr("规则: %1/%2 启用").arg(enabled).arg(total));
}

/** @brief 初始化UI布局(规则列表+计数标签+按钮栏)并连接信号 */
void TriggerListPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    /* 规则列表 */
    m_ruleList = new QListWidget(this);
    m_ruleList->setObjectName(QStringLiteral("ruleList"));
    mainLayout->addWidget(m_ruleList);

    /* 计数标签 */
    m_countLabel = new QLabel(tr("规则: 0/0 启用"), this);
    m_countLabel->setObjectName(QStringLiteral("ruleCountLabel"));
    mainLayout->addWidget(m_countLabel);

    /* 按钮栏 */
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(4);

    m_addBtn = new QPushButton(tr("➕"), this);
    m_addBtn->setObjectName(QStringLiteral("addRuleBtn"));
    m_addBtn->setToolTip(tr("添加新规则"));

    m_removeBtn = new QPushButton(tr("➖"), this);
    m_removeBtn->setObjectName(QStringLiteral("removeRuleBtn"));
    m_removeBtn->setToolTip(tr("移除选中规则"));

    m_editBtn = new QPushButton(tr("✏️"), this);
    m_editBtn->setObjectName(QStringLiteral("editRuleBtn"));
    m_editBtn->setToolTip(tr("编辑选中规则"));

    m_moveUpBtn = new QPushButton(tr("⬆"), this);
    m_moveUpBtn->setObjectName(QStringLiteral("moveUpRuleBtn"));
    m_moveUpBtn->setToolTip(tr("上移选中规则"));

    m_moveDownBtn = new QPushButton(tr("⬇"), this);
    m_moveDownBtn->setObjectName(QStringLiteral("moveDownRuleBtn"));
    m_moveDownBtn->setToolTip(tr("下移选中规则"));

    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_removeBtn);
    btnLayout->addWidget(m_editBtn);
    btnLayout->addWidget(m_moveUpBtn);
    btnLayout->addWidget(m_moveDownBtn);
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

    connect(m_editBtn, &QPushButton::clicked, this, [this]() {
        const int row = m_ruleList->currentRow();
        if (row >= 0) {
            ++m_totalRuleEdits;
            emit editRuleRequested(row);
        }
    });

    connect(m_moveUpBtn, &QPushButton::clicked, this, [this]() {
        const int row = m_ruleList->currentRow();
        if (row > 0) {
            ++m_totalRuleReorders;
            emit moveUpRequested(row);
        }
    });

    connect(m_moveDownBtn, &QPushButton::clicked, this, [this]() {
        const int row = m_ruleList->currentRow();
        if (row >= 0 && row < m_ruleList->count() - 1) {
            ++m_totalRuleReorders;
            emit moveDownRequested(row);
        }
    });

    /* 双击切换启用状态 */
    connect(m_ruleList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        const int row = m_ruleList->row(item);
        const bool newState = item->checkState() != Qt::Checked;
        item->setCheckState(newState ? Qt::Checked : Qt::Unchecked);
        ++m_totalRuleToggles;
        emit ruleEnabledChanged(row, newState);
    });
}
