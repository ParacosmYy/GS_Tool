#include "TerminalSearchBar.h"
#include "utils/HexConverter.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QStyle>

// 暗色主题配色常量
namespace SearchBarColors {
    constexpr const char* kBackground   = "#313244";  // 搜索栏背景
    constexpr const char* kText         = "#cdd6f4";  // 文字颜色
    constexpr const char* kHighlight    = "#89b4fa";  // 高亮/强调色
    constexpr const char* kError        = "#f38ba8";  // 错误色(非法输入)
    constexpr const char* kBorderNormal = "#45475a";  // 正常边框色
    constexpr const char* kButtonBg     = "#45475a";  // 按钮背景色
    constexpr const char* kPlaceholder  = "#6c7086";  // 占位文字色
}

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
    // 水平布局: 输入框 + 正则复选框 + HEX复选框 + 结果标签 + 弹簧 + 关闭按钮
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(6);

    // --- 搜索输入框 ---
    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText(tr("搜索... (支持正则表达式)"));
    m_searchInput->setMinimumWidth(240);
    m_searchInput->setClearButtonEnabled(true);
    // 暗色主题样式表
    m_searchInput->setStyleSheet(
        QString("QLineEdit {"
                "  background: %1;"
                "  color: %2;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 4px 8px;"
                "  selection-background-color: %4;"
                "}"
                "QLineEdit::placeholder { color: %5; }"
                "QLineEdit:focus { border: 1px solid %4; }")
            .arg(SearchBarColors::kBorderNormal)
            .arg(SearchBarColors::kText)
            .arg(SearchBarColors::kBorderNormal)
            .arg(SearchBarColors::kHighlight)
            .arg(SearchBarColors::kPlaceholder)
    );
    layout->addWidget(m_searchInput);

    // --- 正则模式复选框 ---
    m_regexCheck = new QCheckBox(QStringLiteral("正则"), this);
    m_regexCheck->setStyleSheet(
        QString("QCheckBox { color: %1; spacing: 4px; }"
                "QCheckBox::indicator { width: 14px; height: 14px; }"
                "QCheckBox::indicator:checked { background: %2; border-radius: 2px; }"
                "QCheckBox::indicator:unchecked { background: %3; border: 1px solid %3; border-radius: 2px; }")
            .arg(SearchBarColors::kText)
            .arg(SearchBarColors::kHighlight)
            .arg(SearchBarColors::kBorderNormal)
    );
    layout->addWidget(m_regexCheck);

    // --- HEX模式复选框 ---
    m_hexCheck = new QCheckBox(QStringLiteral("HEX"), this);
    m_hexCheck->setStyleSheet(m_regexCheck->styleSheet());
    layout->addWidget(m_hexCheck);

    // --- 结果标签 ---
    m_resultLabel = new QLabel(this);
    m_resultLabel->setStyleSheet(
        QString("QLabel { color: %1; padding: 0 4px; }")
            .arg(SearchBarColors::kPlaceholder)
    );
    m_resultLabel->setMinimumWidth(80);
    layout->addWidget(m_resultLabel);

    // 弹簧, 将关闭按钮推到右侧
    layout->addStretch();

    // --- 关闭按钮 ---
    m_closeBtn = new QPushButton(this);
    m_closeBtn->setFixedSize(24, 24);
    m_closeBtn->setToolTip(QStringLiteral("关闭搜索栏 (Esc)"));
    // 使用标准像素图作为关闭图标, 回退到文字 "X"
    m_closeBtn->setText(QStringLiteral("X"));
    m_closeBtn->setStyleSheet(
        QString("QPushButton {"
                "  background: %1;"
                "  color: %2;"
                "  border: none;"
                "  border-radius: 4px;"
                "  font-weight: bold;"
                "}"
                "QPushButton:hover { background: %3; }"
                "QPushButton:pressed { background: %3; }")
            .arg(SearchBarColors::kButtonBg)
            .arg(SearchBarColors::kText)
            .arg(SearchBarColors::kHighlight)
    );
    layout->addWidget(m_closeBtn);

    // --- 整个搜索栏的背景样式 ---
    setStyleSheet(
        QString("QWidget { background: %1; }")
            .arg(SearchBarColors::kBackground)
    );
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
                // HEX模式切换时重新验证
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
            // 非法HEX: 输入框边框变红
            m_searchInput->setStyleSheet(
                QString("QLineEdit {"
                        "  background: %1;"
                        "  color: %2;"
                        "  border: 2px solid %3;"
                        "  border-radius: 4px;"
                        "  padding: 4px 8px;"
                        "}"
                        "QLineEdit::placeholder { color: %4; }")
                    .arg(SearchBarColors::kBorderNormal)
                    .arg(SearchBarColors::kText)
                    .arg(SearchBarColors::kError)
                    .arg(SearchBarColors::kPlaceholder)
            );
            m_resultLabel->setText(QStringLiteral("非法HEX"));
            m_resultLabel->setStyleSheet(
                QString("QLabel { color: %1; padding: 0 4px; }")
                    .arg(SearchBarColors::kError)
            );
            // 非法输入时不发出搜索信号
            return;
        }
    }

    // 恢复正常边框样式
    m_searchInput->setStyleSheet(
        QString("QLineEdit {"
                "  background: %1;"
                "  color: %2;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 4px 8px;"
                "  selection-background-color: %4;"
                "}"
                "QLineEdit::placeholder { color: %5; }"
                "QLineEdit:focus { border: 1px solid %4; }")
            .arg(SearchBarColors::kBorderNormal)
            .arg(SearchBarColors::kText)
            .arg(SearchBarColors::kBorderNormal)
            .arg(SearchBarColors::kHighlight)
            .arg(SearchBarColors::kPlaceholder)
    );
    m_resultLabel->setStyleSheet(
        QString("QLabel { color: %1; padding: 0 4px; }")
            .arg(SearchBarColors::kPlaceholder)
    );

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
