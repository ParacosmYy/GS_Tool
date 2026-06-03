/**
 * @file TerminalSearchBar.cpp
 * @brief 终端搜索栏实现 - 嵌入终端顶部的搜索控件
 *
 * 展开/收起动画使用 QPropertyAnimation 驱动 maximumHeight 属性:
 *   - 展开: 0 -> 36, 200ms, QEasingCurve::OutCubic
 *   - 收起: 36 -> 0, 150ms, QEasingCurve::InCubic
 */

#include "terminal/search/TerminalSearchBar.h"
#include "core/theme/Constants.h"
#include "utils/crypto/HexConverter.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QRegularExpression>
#include <QStyle>

/** @brief 构造函数 - 初始化界面并隐藏搜索栏(Ctrl+F激活) @param parent 父控件 */
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

/** @brief 构建界面布局和样式(水平布局:输入框|正则|HEX|结果|弹簧|关闭) */
void TerminalSearchBar::setupUI()
{
    setObjectName("terminalSearchBar");

    // 水平布局: 输入框 + 正则复选框 + HEX复选框 + 结果标签 + 弹簧 + 关闭按钮
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(Layout::kToolbarPadding, Layout::kToolbarSpacing,
                               Layout::kToolbarPadding, Layout::kToolbarSpacing);
    layout->setSpacing(Layout::kControlSpacing);

    // ---- 搜索输入框 ----
    m_searchInput = new QLineEdit(this);
    m_searchInput->setObjectName("searchBarInput");
    m_searchInput->setPlaceholderText(tr("搜索... (支持正则表达式)"));
    m_searchInput->setMinimumWidth(Layout::kSearchInputMinWidth);
    m_searchInput->setClearButtonEnabled(true);
    layout->addWidget(m_searchInput);

    // ---- 正则模式复选框 ----
    m_regexCheck = new QCheckBox(tr("正则"), this);
    m_regexCheck->setObjectName("searchBarRegexCheck");
    layout->addWidget(m_regexCheck);

    // ---- HEX模式复选框 ----
    m_hexCheck = new QCheckBox(tr("HEX"), this);
    m_hexCheck->setObjectName("searchBarHexCheck");
    layout->addWidget(m_hexCheck);

    // ---- 结果标签 ----
    m_resultLabel = new QLabel(this);
    m_resultLabel->setObjectName("searchBarResult");
    m_resultLabel->setMinimumWidth(Layout::kSearchResultMinWidth);
    layout->addWidget(m_resultLabel);

    // 弹簧, 将关闭按钮推到右侧
    layout->addStretch();

    // ---- 关闭按钮 ----
    m_closeBtn = new QPushButton(this);
    m_closeBtn->setObjectName("searchBarCloseBtn");
    m_closeBtn->setFixedSize(24, 24);
    m_closeBtn->setToolTip(tr("关闭搜索栏 (Esc)"));
    m_closeBtn->setText(tr("X"));
    layout->addWidget(m_closeBtn);

    // 固定高度, 不占用过多终端空间
    setFixedHeight(Layout::kSearchBarHeight);

    // ---- 信号连接 ----
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

/** @brief 获取当前搜索输入框中的文本 */
QString TerminalSearchBar::searchPattern() const
{
    return m_searchInput->text();
}

/** @brief 返回正则模式复选框是否选中 */
bool TerminalSearchBar::isRegexMode() const
{
    return m_regexCheck->isChecked();
}

/** @brief 返回HEX模式复选框是否选中 */
bool TerminalSearchBar::isHexMode() const
{
    return m_hexCheck->isChecked();
}

/** @brief 激活搜索栏并聚焦输入框(已可见仅聚焦，不可见播放展开动画200ms OutCubic) */
void TerminalSearchBar::activate()
{
    // 如果已经可见，仅聚焦
    if (isVisible()) {
        m_searchInput->setFocus();
        m_searchInput->selectAll();
        return;
    }

    // 展开动画: maximumHeight 从 0 到 36, 200ms, OutCubic 缓动
    // 先设为0高度并显示，然后动画展开
    setMaximumHeight(0);
    show();

    // 停止可能残留的收起动画（快速 Ctrl+F -> Esc -> Ctrl+F 场景）
    if (m_activeAnim) {
        m_activeAnim->stop();
        m_activeAnim = nullptr;
    }

    m_activeAnim = new QPropertyAnimation(this, "maximumHeight");
    m_activeAnim->setStartValue(0);
    m_activeAnim->setEndValue(36);
    m_activeAnim->setDuration(Animations::kSearchExpandMs);
    m_activeAnim->setEasingCurve(QEasingCurve::OutCubic);
    // 动画结束后恢复固定高度，避免布局异常
    connect(m_activeAnim, &QPropertyAnimation::finished, this, [this]() {
        setFixedHeight(Layout::kSearchBarHeight);
    });
    m_activeAnim->start(QAbstractAnimation::DeleteWhenStopped);

    m_searchInput->setFocus();
    m_searchInput->selectAll();
}

/** @brief 关闭搜索栏并清除内容(收起动画150ms InCubic完成后发射closed信号) */
void TerminalSearchBar::deactivate()
{
    m_searchInput->clear();
    m_resultLabel->clear();

    // 收起动画: maximumHeight 从 36 到 0, 150ms, InCubic 缓动
    // 完成后隐藏并恢复状态
    // 停止可能残留的展开动画（快速 Ctrl+F -> Esc 场景）
    if (m_activeAnim) {
        m_activeAnim->stop();
        m_activeAnim = nullptr;
    }

    m_activeAnim = new QPropertyAnimation(this, "maximumHeight");
    m_activeAnim->setStartValue(36);
    m_activeAnim->setEndValue(0);
    m_activeAnim->setDuration(Animations::kSearchCollapseMs);
    m_activeAnim->setEasingCurve(QEasingCurve::InCubic);
    connect(m_activeAnim, &QPropertyAnimation::finished, this, [this]() {
        hide();
        // 恢复固定高度，为下次展开做准备
        setFixedHeight(Layout::kSearchBarHeight);
        emit closed();
    });
    m_activeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

/** @brief 搜索文本变化时触发搜索或清除(HEX模式验证合法性+错误样式) @param text 当前搜索框文本 */
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
        ++m_totalSearches;  ///< 统计: 搜索触发
        emit searchRequested(text, m_regexCheck->isChecked(), m_hexCheck->isChecked());
    }
}

/** @brief 关闭按钮点击，委托给 deactivate() */
void TerminalSearchBar::onCloseClicked()
{
    deactivate();
}

/** @brief 验证HEX输入是否合法(委托给HexConverter::isValidHex) @param text 待验证的字符串 @return true合法 */
bool TerminalSearchBar::isValidHex(const QString& text) const
{
    return HexConverter::isValidHex(text);
}

/** @brief 设置匹配结果显示文本 @param text 要显示的结果文本(如"3/15"或"非法HEX") */
void TerminalSearchBar::setResultText(const QString& text)
{
    m_resultLabel->setText(text);
}

/** @brief 重置搜索栏统计计数器 */
void TerminalSearchBar::resetSearchBarStatistics()
{
    m_totalSearches = 0;
    m_totalMatches = 0;
    m_totalReplacements = 0;
}
