/**
 * @file BridgeConfigPanel.cpp
 * @brief 桥接规则配置面板实现 — UI 布局、规则编辑、信号连接
 */

#include "connection/bridge/BridgeConfigPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QListWidgetItem>
#include <QGroupBox>

// ============================================================
// 构造
// ============================================================

/** @brief 构造桥接配置面板 @param parent 父控件 */
BridgeConfigPanel::BridgeConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_ruleList(nullptr)
    , m_nameEdit(nullptr)
    , m_sourceCombo(nullptr)
    , m_targetCombo(nullptr)
    , m_directionCombo(nullptr)
    , m_filterTypeCombo(nullptr)
    , m_filterPatternEdit(nullptr)
    , m_filterInclusiveCheck(nullptr)
    , m_addBtn(nullptr)
    , m_removeBtn(nullptr)
    , m_applyBtn(nullptr)
    , m_toggleBtn(nullptr)
    , m_countLabel(nullptr)
{
    setObjectName(QStringLiteral("BridgeConfigPanel"));
    setupUI();
}

// ============================================================
// 公共接口
// ============================================================

/**
 * @brief 设置可用连接 ID 列表
 * @param ids 连接 ID 字符串列表
 *
 * 清空并重新填充源/目标下拉框，首项为占位提示。
 */
void BridgeConfigPanel::setAvailableConnections(const QStringList& ids)
{
    m_sourceCombo->clear();
    m_targetCombo->clear();

    m_sourceCombo->addItem(tr("-- 选择源 --"), QString());
    m_targetCombo->addItem(tr("-- 选择目标 --"), QString());

    for (const QString& id : ids) {
        m_sourceCombo->addItem(id, id);
        m_targetCombo->addItem(id, id);
    }
}

/**
 * @brief 获取当前编辑区中的规则配置
 * @return 当前表单组装的 BridgeRule
 */
BridgeRule BridgeConfigPanel::currentRule() const
{
    BridgeRule rule;
    rule.name       = m_nameEdit->text().trimmed();
    rule.sourceId   = m_sourceCombo->currentData().toString();
    rule.targetId   = m_targetCombo->currentData().toString();
    rule.direction  = static_cast<BridgeDirection>(m_directionCombo->currentIndex());
    rule.enabled    = true;

    /* 组装单条过滤器（简化场景：面板只提供一组过滤配置） */
    const auto fType = static_cast<BridgeFilterType>(m_filterTypeCombo->currentIndex());
    if (fType != BridgeFilterType::None && !m_filterPatternEdit->text().isEmpty()) {
        BridgeFilter filter;
        filter.type      = fType;
        filter.pattern   = m_filterPatternEdit->text();
        filter.inclusive = m_filterInclusiveCheck->isChecked();
        rule.filters.append(filter);
    }

    return rule;
}

/**
 * @brief 获取面板中所有规则列表
 * @return 规则列表
 */
QVector<BridgeRule> BridgeConfigPanel::allRules() const
{
    return m_rules;
}

// ============================================================
// 私有：UI 初始化
// ============================================================

/** @brief 初始化 UI 布局和控件，连接信号槽 */
void BridgeConfigPanel::setupUI()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(6);

    /* ---- 左侧：规则列表 ---- */
    auto* leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(4);

    m_ruleList = new QListWidget(this);
    m_ruleList->setObjectName(QStringLiteral("bridgeRuleList"));
    leftLayout->addWidget(m_ruleList);

    m_countLabel = new QLabel(tr("桥接规则: 0"), this);
    m_countLabel->setObjectName(QStringLiteral("bridgeCountLabel"));
    leftLayout->addWidget(m_countLabel);

    auto* listBtnLayout = new QHBoxLayout();
    listBtnLayout->setSpacing(4);

    m_removeBtn = new QPushButton(tr("移除"), this);
    m_removeBtn->setObjectName(QStringLiteral("bridgeRemoveBtn"));
    m_removeBtn->setToolTip(tr("移除选中的桥接规则"));

    m_toggleBtn = new QPushButton(tr("切换启用"), this);
    m_toggleBtn->setObjectName(QStringLiteral("bridgeToggleBtn"));
    m_toggleBtn->setToolTip(tr("启用/禁用选中的桥接规则"));

    listBtnLayout->addWidget(m_removeBtn);
    listBtnLayout->addWidget(m_toggleBtn);
    listBtnLayout->addStretch();
    leftLayout->addLayout(listBtnLayout);

    mainLayout->addLayout(leftLayout, 2);

    /* ---- 右侧：编辑区 ---- */
    auto* editGroup = new QGroupBox(tr("规则编辑"), this);
    editGroup->setObjectName(QStringLiteral("bridgeEditGroup"));
    auto* formLayout = new QFormLayout(editGroup);
    formLayout->setSpacing(4);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setObjectName(QStringLiteral("bridgeNameEdit"));
    m_nameEdit->setPlaceholderText(tr("输入规则名称"));
    formLayout->addRow(tr("名称:"), m_nameEdit);

    m_sourceCombo = new QComboBox(this);
    m_sourceCombo->setObjectName(QStringLiteral("bridgeSourceCombo"));
    m_sourceCombo->addItem(tr("-- 选择源 --"), QString());
    formLayout->addRow(tr("源连接:"), m_sourceCombo);

    m_targetCombo = new QComboBox(this);
    m_targetCombo->setObjectName(QStringLiteral("bridgeTargetCombo"));
    m_targetCombo->addItem(tr("-- 选择目标 --"), QString());
    formLayout->addRow(tr("目标连接:"), m_targetCombo);

    m_directionCombo = new QComboBox(this);
    m_directionCombo->setObjectName(QStringLiteral("bridgeDirectionCombo"));
    m_directionCombo->addItem(tr("正向"),     static_cast<int>(BridgeDirection::Forward));
    m_directionCombo->addItem(tr("反向"),     static_cast<int>(BridgeDirection::Backward));
    m_directionCombo->addItem(tr("双向"), static_cast<int>(BridgeDirection::Bidirectional));
    formLayout->addRow(tr("方向:"), m_directionCombo);

    m_filterTypeCombo = new QComboBox(this);
    m_filterTypeCombo->setObjectName(QStringLiteral("bridgeFilterTypeCombo"));
    m_filterTypeCombo->addItem(tr("无过滤"),   static_cast<int>(BridgeFilterType::None));
    m_filterTypeCombo->addItem(tr("前缀匹配"), static_cast<int>(BridgeFilterType::Prefix));
    m_filterTypeCombo->addItem(tr("正则匹配"), static_cast<int>(BridgeFilterType::Regex));
    m_filterTypeCombo->addItem(tr("长度过滤"), static_cast<int>(BridgeFilterType::Length));
    formLayout->addRow(tr("过滤类型:"), m_filterTypeCombo);

    m_filterPatternEdit = new QLineEdit(this);
    m_filterPatternEdit->setObjectName(QStringLiteral("bridgeFilterPatternEdit"));
    m_filterPatternEdit->setPlaceholderText(tr("过滤表达式（如前缀/正则/min-max）"));
    formLayout->addRow(tr("过滤模式:"), m_filterPatternEdit);

    m_filterInclusiveCheck = new QCheckBox(tr("白名单模式（匹配通过）"), this);
    m_filterInclusiveCheck->setObjectName(QStringLiteral("bridgeFilterInclusiveCheck"));
    m_filterInclusiveCheck->setChecked(true);
    formLayout->addRow(QString(), m_filterInclusiveCheck);

    /* 添加/应用按钮 */
    auto* editBtnLayout = new QHBoxLayout();
    editBtnLayout->setSpacing(4);

    m_addBtn = new QPushButton(tr("添加"), this);
    m_addBtn->setObjectName(QStringLiteral("bridgeAddBtn"));
    m_addBtn->setToolTip(tr("创建新的桥接规则"));

    m_applyBtn = new QPushButton(tr("应用修改"), this);
    m_applyBtn->setObjectName(QStringLiteral("bridgeApplyBtn"));
    m_applyBtn->setToolTip(tr("将编辑区的修改应用到选中规则"));

    editBtnLayout->addWidget(m_addBtn);
    editBtnLayout->addWidget(m_applyBtn);
    editBtnLayout->addStretch();
    formLayout->addRow(editBtnLayout);

    mainLayout->addWidget(editGroup, 3);

    /* ---- 信号连接 ---- */
    connect(m_addBtn, &QPushButton::clicked, this, &BridgeConfigPanel::onAddClicked);
    connect(m_removeBtn, &QPushButton::clicked, this, &BridgeConfigPanel::onRemoveClicked);
    connect(m_applyBtn, &QPushButton::clicked, this, &BridgeConfigPanel::onApplyClicked);
    connect(m_toggleBtn, &QPushButton::clicked, this, &BridgeConfigPanel::onToggleClicked);
    connect(m_ruleList, &QListWidget::currentRowChanged,
            this, &BridgeConfigPanel::onRuleSelected);
}

// ============================================================
// 私有：操作槽
// ============================================================

/** @brief 从表单组装 BridgeRule 并添加到列表，发射 bridgeAdded */
void BridgeConfigPanel::onAddClicked()
{
    BridgeRule rule = currentRule();
    if (rule.name.isEmpty() || rule.sourceId.isEmpty() || rule.targetId.isEmpty()) {
        return;
    }

    /* 检查名称唯一性 */
    for (const BridgeRule& existing : m_rules) {
        if (existing.name == rule.name) {
            return;
        }
    }

    m_rules.append(rule);
    ++m_stats.totalRulesCreated;

    /* 刷新列表 */
    const QString status = rule.enabled ? tr("启用") : tr("禁用");
    const QString dirText = m_directionCombo->currentText();
    const QString display = QStringLiteral("[%1] %2 → %3 (%4)")
                                .arg(status, rule.sourceId, rule.targetId, dirText);
    auto* item = new QListWidgetItem(display, m_ruleList);
    item->setData(Qt::UserRole, rule.name);
    item->setCheckState(rule.enabled ? Qt::Checked : Qt::Unchecked);

    m_countLabel->setText(tr("桥接规则: %1").arg(m_rules.size()));
    emit bridgeAdded(rule);
}

/** @brief 移除列表中选中的规则，发射 bridgeRemoved */
void BridgeConfigPanel::onRemoveClicked()
{
    const int row = m_ruleList->currentRow();
    if (row < 0 || row >= m_rules.size()) {
        return;
    }

    const QString name = m_rules.at(row).name;
    m_rules.removeAt(row);
    delete m_ruleList->takeItem(row);
    ++m_stats.totalRulesDeleted;

    m_countLabel->setText(tr("桥接规则: %1").arg(m_rules.size()));
    emit bridgeRemoved(name);
}

/** @brief 将编辑区的内容应用到选中规则，发射 bridgeModified */
void BridgeConfigPanel::onApplyClicked()
{
    const int row = m_ruleList->currentRow();
    if (row < 0 || row >= m_rules.size()) {
        return;
    }

    BridgeRule rule = currentRule();
    if (rule.name.isEmpty()) {
        return;
    }

    m_rules[row] = rule;
    ++m_stats.totalRulesModified;

    /* 更新列表项 */
    const QString status = rule.enabled ? tr("启用") : tr("禁用");
    const QString dirText = m_directionCombo->currentText();
    const QString display = QStringLiteral("[%1] %2 → %3 (%4)")
                                .arg(status, rule.sourceId, rule.targetId, dirText);

    QListWidgetItem* item = m_ruleList->item(row);
    item->setText(display);
    item->setData(Qt::UserRole, rule.name);
    item->setCheckState(rule.enabled ? Qt::Checked : Qt::Unchecked);

    emit bridgeModified(rule);
}

/** @brief 切换选中规则的启用状态 */
void BridgeConfigPanel::onToggleClicked()
{
    const int row = m_ruleList->currentRow();
    if (row < 0 || row >= m_rules.size()) {
        return;
    }

    m_rules[row].enabled = !m_rules[row].enabled;
    ++m_stats.totalRulesModified;

    QListWidgetItem* item = m_ruleList->item(row);
    item->setCheckState(m_rules[row].enabled ? Qt::Checked : Qt::Unchecked);

    /* 更新显示文本 */
    const BridgeRule& rule = m_rules[row];
    const QString status = rule.enabled ? tr("启用") : tr("禁用");
    const QString dirText = m_directionCombo->itemText(static_cast<int>(rule.direction));
    const QString display = QStringLiteral("[%1] %2 → %3 (%4)")
                                .arg(status, rule.sourceId, rule.targetId, dirText);
    item->setText(display);

    emit bridgeModified(rule);
}

/** @brief 列表选择变化时将规则加载到编辑区 */
void BridgeConfigPanel::onRuleSelected(int row)
{
    if (row < 0 || row >= m_rules.size()) {
        return;
    }
    populateForm(m_rules.at(row));
}

/** @brief 将规则字段填充到编辑区控件 */
void BridgeConfigPanel::populateForm(const BridgeRule& rule)
{
    m_nameEdit->setText(rule.name);

    /* 按数据查找下拉框索引 */
    const int srcIdx = m_sourceCombo->findData(rule.sourceId);
    if (srcIdx >= 0) m_sourceCombo->setCurrentIndex(srcIdx);

    const int tgtIdx = m_targetCombo->findData(rule.targetId);
    if (tgtIdx >= 0) m_targetCombo->setCurrentIndex(tgtIdx);

    m_directionCombo->setCurrentIndex(static_cast<int>(rule.direction));

    /* 填充过滤器（取第一条，面板简化为单过滤器编辑） */
    if (!rule.filters.isEmpty()) {
        const BridgeFilter& f = rule.filters.first();
        m_filterTypeCombo->setCurrentIndex(static_cast<int>(f.type));
        m_filterPatternEdit->setText(f.pattern);
        m_filterInclusiveCheck->setChecked(f.inclusive);
    } else {
        m_filterTypeCombo->setCurrentIndex(static_cast<int>(BridgeFilterType::None));
        m_filterPatternEdit->clear();
        m_filterInclusiveCheck->setChecked(true);
    }
}

// 统计 getter / reset 已移至 BridgeConfigPanelStats.cpp
