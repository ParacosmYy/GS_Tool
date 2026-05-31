/**
 * @file TerminalSearchBar.cpp
 * @brief 终端搜索栏实现 - 嵌入终端顶部的搜索控件
 *
 * 展开/收起动画使用 QPropertyAnimation 驱动 maximumHeight 属性:
 *   - 展开: 0 -> 36, 200ms, QEasingCurve::OutCubic
 *   - 收起: 36 -> 0, 150ms, QEasingCurve::InCubic
 */

#include "TerminalSearchBar.h"
#include "utils/HexConverter.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QRegularExpression>
#include <QStyle>

/**
 * @brief 构造函数 - 初始化界面并隐藏搜索栏
 * @param parent 父控件
 *
 * 初始状态为隐藏，等待 TerminalWidget 的 Ctrl+F 快捷键调用 activate()。
 */
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

/**
 * @brief 构建界面布局和样式
 *
 * 水平布局: 搜索输入框 | 正则复选框 | HEX复选框 | 结果标签 | 弹簧 | 关闭按钮
 * 固定高度 36px，与工具栏高度协调。
 */
void TerminalSearchBar::setupUI()
{
    setObjectName("terminalSearchBar");

    // 水平布局: 输入框 + 正则复选框 + HEX复选框 + 结果标签 + 弹簧 + 关闭按钮
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(6);

    // ---- 搜索输入框 ----
    m_searchInput = new QLineEdit(this);
    m_searchInput->setObjectName("searchBarInput");
    m_searchInput->setPlaceholderText(tr("搜索... (支持正则表达式)"));
    m_searchInput->setMinimumWidth(240);
    m_searchInput->setClearButtonEnabled(true);
    layout->addWidget(m_searchInput);

    // ---- 正则模式复选框 ----
    m_regexCheck = new QCheckBox(tr("Regex"), this);
    m_regexCheck->setObjectName("searchBarRegexCheck");
    layout->addWidget(m_regexCheck);

    // ---- HEX模式复选框 ----
    m_hexCheck = new QCheckBox(tr("HEX"), this);
    m_hexCheck->setObjectName("searchBarHexCheck");
    layout->addWidget(m_hexCheck);

    // ---- 结果标签 ----
    m_resultLabel = new QLabel(this);
    m_resultLabel->setObjectName("searchBarResult");
    m_resultLabel->setMinimumWidth(80);
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
    setFixedHeight(36);

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

/**
 * @brief 激活搜索栏并聚焦输入框
 *
 * 如果已经可见，仅聚焦并全选文本，不重新播放动画。
 * 如果不可见:
 *   1. 设置 maximumHeight 为 0 并 show()
 *   2. 启动 QPropertyAnimation: maximumHeight 0 -> 36, 200ms, OutCubic
 *   3. 动画结束后恢复 setFixedHeight(36)，避免布局异常
 */
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

    QPropertyAnimation* expandAnim = new QPropertyAnimation(this, "maximumHeight");
    expandAnim->setStartValue(0);
    expandAnim->setEndValue(36);
    expandAnim->setDuration(200);
    expandAnim->setEasingCurve(QEasingCurve::OutCubic);
    // 动画结束后恢复固定高度，避免布局异常
    connect(expandAnim, &QPropertyAnimation::finished, this, [this]() {
        setFixedHeight(36);
    });
    expandAnim->start(QAbstractAnimation::DeleteWhenStopped);

    m_searchInput->setFocus();
    m_searchInput->selectAll();
}

/**
 * @brief 关闭搜索栏并清除内容
 *
 * 清除搜索文本和结果标签后，播放收起动画:
 *   1. QPropertyAnimation: maximumHeight 36 -> 0, 150ms, InCubic
 *   2. 动画完成后隐藏控件
 *   3. 恢复 setFixedHeight(36) 为下次展开做准备
 *   4. 发射 closed() 信号通知 TerminalWidget
 */
void TerminalSearchBar::deactivate()
{
    m_searchInput->clear();
    m_resultLabel->clear();

    // 收起动画: maximumHeight 从 36 到 0, 150ms, InCubic 缓动
    // 完成后隐藏并恢复状态
    QPropertyAnimation* collapseAnim = new QPropertyAnimation(this, "maximumHeight");
    collapseAnim->setStartValue(36);
    collapseAnim->setEndValue(0);
    collapseAnim->setDuration(150);
    collapseAnim->setEasingCurve(QEasingCurve::InCubic);
    connect(collapseAnim, &QPropertyAnimation::finished, this, [this]() {
        hide();
        // 恢复固定高度，为下次展开做准备
        setFixedHeight(36);
        emit closed();
    });
    collapseAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

/**
 * @brief 搜索文本变化时触发搜索或清除
 * @param text 当前搜索框文本
 *
 * 处理逻辑:
 *   1. HEX 模式下验证输入合法性，非法时设置 hasError 属性切换错误样式
 *   2. 恢复正常样式（清除 hasError）
 *   3. 文本为空时发射 searchCleared()，非空时发射 searchRequested()
 *
 * 动态属性切换通过 style()->unpolish() + style()->polish() 触发 QSS 重新应用。
 */
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

/** @brief 关闭按钮点击，委托给 deactivate() */
void TerminalSearchBar::onCloseClicked()
{
    deactivate();
}

/**
 * @brief 验证HEX输入是否合法
 * @param text 待验证的字符串
 * @return true 如果是合法的十六进制字符串
 *
 * 委托给公共组件 HexConverter::isValidHex() 实现，不重复造轮子。
 */
bool TerminalSearchBar::isValidHex(const QString& text) const
{
    return HexConverter::isValidHex(text);
}

/**
 * @brief 设置匹配结果显示文本
 * @param text 要显示的结果文本（如 "3/15" 或 "非法HEX"）
 */
void TerminalSearchBar::setResultText(const QString& text)
{
    m_resultLabel->setText(text);
}
