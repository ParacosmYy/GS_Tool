/**
 * @file TerminalFilterBar.cpp
 * @brief 终端过滤工具栏实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "terminal/filter/TerminalFilterBar.h"

#include <QRegularExpression>

/** @brief 构造函数，初始化过滤栏UI布局并连接信号槽 @param parent 父控件 */
TerminalFilterBar::TerminalFilterBar(QWidget *parent)
    : QWidget(parent)
    , m_patternEdit(new QLineEdit(this))
    , m_historyCombo(new QComboBox(this))
    , m_caseCheck(new QCheckBox(tr("区分大小写"), this))
    , m_invertCheck(new QCheckBox(tr("反转"), this))
    , m_applyBtn(new QPushButton(tr("应用过滤"), this))
    , m_clearBtn(new QPushButton(tr("清除"), this))
{
    setObjectName(QStringLiteral("TerminalFilterBar"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);

    m_patternEdit->setObjectName("filterPatternEdit");
    m_patternEdit->setPlaceholderText(tr("输入正则表达式..."));
    m_patternEdit->setClearButtonEnabled(true);

    m_historyCombo->setObjectName("filterHistoryCombo");
    m_historyCombo->setFixedWidth(150);
    m_historyCombo->setEditable(false);
    m_historyCombo->setToolTip(tr("过滤历史"));

    m_caseCheck->setObjectName("filterCaseCheck");
    m_invertCheck->setObjectName("filterInvertCheck");
    m_applyBtn->setObjectName("filterApplyBtn");
    m_applyBtn->setFixedWidth(100);
    m_clearBtn->setObjectName("filterClearBtn");
    m_clearBtn->setFixedWidth(60);

    layout->addWidget(m_patternEdit);
    layout->addWidget(m_historyCombo);
    layout->addWidget(m_caseCheck);
    layout->addWidget(m_invertCheck);
    layout->addWidget(m_applyBtn);
    layout->addWidget(m_clearBtn);

    // 连接信号
    connect(m_applyBtn, &QPushButton::clicked,
            this, &TerminalFilterBar::onApplyClicked);

    connect(m_patternEdit, &QLineEdit::returnPressed,
            this, &TerminalFilterBar::onApplyClicked);

    connect(m_clearBtn, &QPushButton::clicked, this, [this]() {
        m_patternEdit->clear();
        clearPatternError();
        ++m_totalFilterClears;  ///< 统计: 过滤清除递增
        emit filterCleared();
    });

    connect(m_historyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (index >= 0) {
            m_patternEdit->setText(m_historyCombo->itemText(index));
        }
    });
}

/** @brief 返回当前输入的正则表达式文本 @return 正则表达式字符串 */
QString TerminalFilterBar::currentPattern() const
{
    return m_patternEdit->text();
}

/** @brief 返回大小写敏感复选框状态 @return true=区分大小写 */
bool TerminalFilterBar::isCaseSensitive() const
{
    return m_caseCheck->isChecked();
}

/** @brief 返回反转过滤复选框状态 @return true=反转过滤 */
bool TerminalFilterBar::isInverted() const
{
    return m_invertCheck->isChecked();
}

/** @brief 应用过滤按钮点击处理，发射filterRequested信号并将正则添加到历史记录(去重，最多20条) */
void TerminalFilterBar::onApplyClicked()
{
    const QString pattern = m_patternEdit->text().trimmed();
    if (pattern.isEmpty()) {
        m_patternEdit->clear();
        clearPatternError();
        ++m_totalFilterClears;
        emit filterCleared();
        return;
    }

    m_patternEdit->setText(pattern);
    QRegularExpression regex(pattern);
    if (!m_caseCheck->isChecked()) {
        regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }
    if (!regex.isValid()) {
        setPatternError(tr("正则表达式无效: %1").arg(regex.errorString()));
        return;
    }

    clearPatternError();
    emit filterRequested(pattern, m_caseCheck->isChecked(),
                         m_invertCheck->isChecked());

    ++m_totalFilterChanges;

    /* 添加到历史（去重，最多20条） */
    int idx = m_historyCombo->findText(pattern);
    if (idx >= 0) {
        m_historyCombo->removeItem(idx);
    }
    m_historyCombo->insertItem(0, pattern);
    m_historyCombo->setCurrentIndex(0);
    while (m_historyCombo->count() > 20) {
        m_historyCombo->removeItem(m_historyCombo->count() - 1);
    }
}

void TerminalFilterBar::setPatternError(const QString &message)
{
    m_patternEdit->setToolTip(message);
    m_patternEdit->setStyleSheet(QStringLiteral("QLineEdit#filterPatternEdit { border: 1px solid #d93025; }"));
}

void TerminalFilterBar::clearPatternError()
{
    m_patternEdit->setToolTip(QString());
    m_patternEdit->setStyleSheet(QString());
}

/** @brief 获取累计过滤变更次数 @return 过滤变更次数 */
quint64 TerminalFilterBar::totalFilterChanges() const
{
    return m_totalFilterChanges;
}

/** @brief 获取累计高亮切换次数 @return 高亮切换次数 */
quint64 TerminalFilterBar::totalHighlightToggles() const
{
    return m_totalHighlightToggles;
}

/** @brief 获取累计方向变更次数 @return 方向变更次数 */
quint64 TerminalFilterBar::totalDirectionChanges() const
{
    return m_totalDirectionChanges;
}

/** @brief 获取累计过滤清除次数 @return 过滤清除次数 */
quint64 TerminalFilterBar::totalFilterClears() const
{
    return m_totalFilterClears;
}

/** @brief 重置所有过滤统计计数器为零 */
void TerminalFilterBar::resetFilterStatistics()
{
    m_totalFilterChanges = 0;
    m_totalHighlightToggles = 0;
    m_totalDirectionChanges = 0;
    m_totalFilterClears = 0;
}
