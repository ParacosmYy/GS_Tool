#include "TerminalSearchBar.h"
#include "utils/HexConverter.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QStyle>

TerminalSearchBar::TerminalSearchBar(QWidget* parent)
    : QWidget(parent)
    , m_searchInput(nullptr)
    , m_closeBtn(nullptr)
    , m_regexCheck(nullptr)
    , m_hexCheck(nullptr)
    , m_resultLabel(nullptr)
{
    setupUI();
    // 初始隐藏, 等待 Ctrl+F 激活
    hide();
}

void TerminalSearchBar::setupUI()
{
    setObjectName("terminalSearchBar");

    // 水平布局: 输入框 + 正则复选框 + HEX复选框 + 结果标签 + 弹簧 + 关闭按钮
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(6);

    // --- 搜索输入框 ---
    m_searchInput = new QLineEdit(this);
    m_searchInput->setObjectName("searchBarInput");
    m_searchInput->setPlaceholderText(tr("搜索... (支持正则表达式)"));
    m_searchInput->setMinimumWidth(240);
    m_searchInput->setClearButtonEnabled(true);
    layout->addWidget(m_searchInput);

    // --- 正则模式复选框 ---
    m_regexCheck = new QCheckBox(QStringLiteral("正则"), this);
    m_regexCheck->setObjectName("searchBarRegexCheck");
    layout->addWidget(m_regexCheck);

    // --- HEX模式复选框 ---
    m_hexCheck = new QCheckBox(QStringLiteral("HEX"), this);
    m_hexCheck->setObjectName("searchBarHexCheck");
    layout->addWidget(m_hexCheck);

    // --- 结果标签 ---
    m_resultLabel = new QLabel(this);
    m_resultLabel->setObjectName("searchBarResult");
    m_resultLabel->setMinimumWidth(80);
    layout->addWidget(m_resultLabel);

    // 弹簧, 将关闭按钮推到右侧
    layout->addStretch();

    // --- 关闭按钮 ---
    m_closeBtn = new QPushButton(this);
    m_closeBtn->setObjectName("searchBarCloseBtn");
    m_closeBtn->setFixedSize(24, 24);
    m_closeBtn->setToolTip(tr("关闭搜索栏 (Esc)"));
    m_closeBtn->setText(QStringLiteral("X"));
    layout->addWidget(m_closeBtn);

    // 固定高度, 不占用过多终端空间
    setFixedHeight(36);

    // --- 信号连接 ---
    // 文字变化时触发搜索
    connect(m_searchInput, &QLineEdit::textChanged,
            this, &TerminalSearchBar::onSearchTextChanged);

    // 正则/HEX复选框变化时也重新触发搜索
    connect(m_regexCheck, &QCheckBox::toggled,
            this, [this]() {
                onSearchTextChanged(m_searchInput->text());
            });
    connect(m_hexCheck, &QCheckBox::toggled,
            this, [this]() {
                onSearchTextChanged(m_searchInput->text());
            });

    // 关闭按钮
    connect(m_closeBtn, &QPushButton::clicked,
            this, &TerminalSearchBar::onCloseClicked);
}

QString TerminalSearchBar::searchPattern() const
{
    return m_searchInput->text();
}

bool TerminalSearchBar::isRegexMode() const
{
    return m_regexCheck->isChecked();
}

bool TerminalSearchBar::isHexMode() const
{
    return m_hexCheck->isChecked();
}

void TerminalSearchBar::activate()
{
    show();
    m_searchInput->setFocus();
    m_searchInput->selectAll();
}

void TerminalSearchBar::deactivate()
{
    m_searchInput->clear();
    m_resultLabel->clear();
    hide();
    emit closed();
}

void TerminalSearchBar::onSearchTextChanged(const QString& text)
{
    // HEX模式下验证输入合法性
    if (m_hexCheck->isChecked() && !text.isEmpty()) {
        if (!isValidHex(text)) {
            // 非法HEX: 通过动态属性切换错误样式
            m_searchInput->setProperty("hasError", true);
            m_searchInput->style()->unpolish(m_searchInput);
            m_searchInput->style()->polish(m_searchInput);
            m_resultLabel->setProperty("hasError", true);
            m_resultLabel->style()->unpolish(m_resultLabel);
            m_resultLabel->style()->polish(m_resultLabel);
            m_resultLabel->setText(tr("非法HEX"));
            return;
        }
    }

    // 恢复正常样式
    m_searchInput->setProperty("hasError", false);
    m_searchInput->style()->unpolish(m_searchInput);
    m_searchInput->style()->polish(m_searchInput);
    m_resultLabel->setProperty("hasError", false);
    m_resultLabel->style()->unpolish(m_resultLabel);
    m_resultLabel->style()->polish(m_resultLabel);

    if (text.isEmpty()) {
        // 文本为空时清除搜索
        m_resultLabel->clear();
        emit searchCleared();
    } else {
        // 文本非空时发出搜索请求
        emit searchRequested(text, m_regexCheck->isChecked(), m_hexCheck->isChecked());
    }
}

void TerminalSearchBar::onCloseClicked()
{
    deactivate();
}

bool TerminalSearchBar::isValidHex(const QString& text) const
{
    return HexConverter::isValidHex(text);
}
