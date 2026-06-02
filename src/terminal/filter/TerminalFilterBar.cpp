/**
 * @file TerminalFilterBar.cpp
 * @brief 终端过滤工具栏实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "terminal/filter/TerminalFilterBar.h"

/**
 * @brief 构造函数，初始化过滤栏 UI 布局
 */
TerminalFilterBar::TerminalFilterBar(QWidget *parent)
    : QWidget(parent)
    , m_patternEdit(new QLineEdit(this))
    , m_caseCheck(new QCheckBox(tr("区分大小写"), this))
    , m_applyBtn(new QPushButton(tr("应用过滤"), this))
{
    setObjectName(QStringLiteral("TerminalFilterBar"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);

    m_patternEdit->setPlaceholderText(tr("输入正则表达式..."));
    m_patternEdit->setClearButtonEnabled(true);

    m_applyBtn->setFixedWidth(100);

    layout->addWidget(m_patternEdit);
    layout->addWidget(m_caseCheck);
    layout->addWidget(m_applyBtn);

    // 连接信号
    connect(m_applyBtn, &QPushButton::clicked,
            this, &TerminalFilterBar::onApplyClicked);

    connect(m_patternEdit, &QLineEdit::returnPressed,
            this, &TerminalFilterBar::onApplyClicked);
}

/**
 * @brief 返回当前输入的正则表达式
 */
QString TerminalFilterBar::currentPattern() const
{
    return m_patternEdit->text();
}

/**
 * @brief 返回大小写敏感复选框状态
 */
bool TerminalFilterBar::isCaseSensitive() const
{
    return m_caseCheck->isChecked();
}

/**
 * @brief 处理应用按钮点击，发射过滤请求信号
 */
void TerminalFilterBar::onApplyClicked()
{
    emit filterRequested(m_patternEdit->text(), m_caseCheck->isChecked());
}
