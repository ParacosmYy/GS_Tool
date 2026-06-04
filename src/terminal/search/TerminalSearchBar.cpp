/**
 * @file TerminalSearchBar.cpp
 * @brief 终端搜索栏实现 - 嵌入终端顶部的搜索控件
 *
 * 展开/收起动画使用 QPropertyAnimation 驱动 maximumHeight 属性:
 *   - 展开: 0 -> 36, 200ms, QEasingCurve::OutCubic
 *   - 收起: 36 -> 0, 150ms, QEasingCurve::InCubic
 *
 * 搜索历史通过 QCompleter 提供自动补全下拉列表。
 */

#include "terminal/search/TerminalSearchBar.h"
#include "shared/Constants.h"
#include "utils/crypto/HexConverter.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QRegularExpression>
#include <QStyle>
#include <QCompleter>
#include <QStringListModel>

/** @brief 构造函数 - 初始化界面(含搜索历史补全器)并隐藏搜索栏(Ctrl+F激活) @param parent 父控件 */
TerminalSearchBar::TerminalSearchBar(QWidget* parent)
    : QWidget(parent)
    , m_searchInput(nullptr)
    , m_closeBtn(nullptr)
    , m_regexCheck(nullptr)
    , m_hexCheck(nullptr)
    , m_caseCheck(nullptr)
    , m_wordCheck(nullptr)
    , m_resultLabel(nullptr)
    , m_completer(nullptr)
{
    setupUI();
    // 初始隐藏, 等待 Ctrl+F 激活
    hide();
}

/** @brief 构建界面布局和样式(水平布局:输入框|正则|HEX|大小写|全词|结果|弹簧|关闭) */
void TerminalSearchBar::setupUI()
{
    setObjectName("terminalSearchBar");

    // 水平布局: 输入框 + 正则 + HEX + 大小写 + 全词 + 结果标签 + 弹簧 + 关闭按钮
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(Layout::kToolbarPadding, Layout::kToolbarSpacing,
                               Layout::kToolbarPadding, Layout::kToolbarSpacing);
    layout->setSpacing(Layout::kControlSpacing);

    // ---- 搜索输入框(带历史补全) ----
    m_searchInput = new QLineEdit(this);
    m_searchInput->setObjectName("searchBarInput");
    m_searchInput->setPlaceholderText(tr("搜索... (支持正则表达式)"));
    m_searchInput->setMinimumWidth(Layout::kSearchInputMinWidth);
    m_searchInput->setClearButtonEnabled(true);
    layout->addWidget(m_searchInput);

    // 搜索历史补全器: 大小写不敏感，弹出列表最多10条
    m_completer = new QCompleter(this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setMaxVisibleItems(10);
    auto* historyModel = new QStringListModel(this);
    m_completer->setModel(historyModel);
    m_searchInput->setCompleter(m_completer);

    // ---- 正则模式复选框 ----
    m_regexCheck = new QCheckBox(tr("正则"), this);
    m_regexCheck->setObjectName("searchBarRegexCheck");
    m_regexCheck->setToolTip(tr("使用正则表达式模式搜索"));
    layout->addWidget(m_regexCheck);

    // ---- HEX模式复选框 ----
    m_hexCheck = new QCheckBox(tr("HEX"), this);
    m_hexCheck->setObjectName("searchBarHexCheck");
    m_hexCheck->setToolTip(tr("使用十六进制模式搜索 (如: AA 55)"));
    layout->addWidget(m_hexCheck);

    // ---- 大小写敏感复选框 ----
    m_caseCheck = new QCheckBox(tr("Aa"), this);
    m_caseCheck->setObjectName("searchBarCaseCheck");
    m_caseCheck->setToolTip(tr("区分大小写"));
    layout->addWidget(m_caseCheck);

    // ---- 全词匹配复选框 ----
    m_wordCheck = new QCheckBox(tr("全词"), this);
    m_wordCheck->setObjectName("searchBarWordCheck");
    m_wordCheck->setToolTip(tr("全词匹配 (仅纯文本模式)"));
    layout->addWidget(m_wordCheck);

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

    // 正则/HEX/大小写/全词复选框变化时重新触发搜索
    auto retrigger = [this]() { triggerSearch(); };
    connect(m_regexCheck, &QCheckBox::toggled, this, retrigger);
    connect(m_hexCheck, &QCheckBox::toggled, this, retrigger);
    connect(m_caseCheck, &QCheckBox::toggled, this, retrigger);
    connect(m_wordCheck, &QCheckBox::toggled, this, retrigger);

    // HEX模式与全词/大小写互斥: HEX启用时禁用全词和大小写
    connect(m_hexCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_caseCheck->setEnabled(!checked);
        m_wordCheck->setEnabled(!checked);
        if (checked) {
            m_caseCheck->setChecked(false);
            m_wordCheck->setChecked(false);
        }
    });

    // 正则模式与全词互斥: 正则启用时禁用全词
    connect(m_regexCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_wordCheck->setEnabled(!checked && !m_hexCheck->isChecked());
        if (checked) {
            m_wordCheck->setChecked(false);
        }
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

/** @brief 返回大小写敏感复选框是否选中 */
bool TerminalSearchBar::isCaseSensitive() const
{
    return m_caseCheck->isChecked();
}

/** @brief 返回全词匹配复选框是否选中 */
bool TerminalSearchBar::isWholeWord() const
{
    return m_wordCheck->isChecked();
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
        setFixedHeight(Layout::kSearchBarHeight);
        emit closed();
    });
    m_activeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

/**
 * @brief 搜索文本变化时验证HEX合法性并触发搜索
 * @param text 当前搜索框文本
 *
 * HEX模式下验证输入合法性，非法时显示错误样式。
 * 合法或非HEX模式时委托给 triggerSearch() 统一处理。
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

    triggerSearch();
}

/**
 * @brief 统一的搜索触发入口
 *
 * 根据当前搜索框内容和选项状态发射 searchRequested 或 searchCleared 信号。
 * 被文本变化和选项变化两种场景共用。
 */
void TerminalSearchBar::triggerSearch()
{
    const QString text = m_searchInput->text();
    if (text.isEmpty()) {
        m_resultLabel->clear();
        emit searchCleared();
    } else {
        ++m_totalSearches;
        if (m_regexCheck->isChecked()) {
            ++m_totalRegexSearches;
        }
        emit searchRequested(text, m_regexCheck->isChecked(), m_hexCheck->isChecked(),
                             m_caseCheck->isChecked(), m_wordCheck->isChecked());
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

/**
 * @brief 更新搜索历史补全列表
 * @param history 最新的搜索历史列表
 *
 * 将历史列表设置到 QCompleter 的 QStringListModel 中，
 * 补全器会自动根据当前输入过滤匹配项。
 */
void TerminalSearchBar::updateSearchHistory(const QStringList& history)
{
    auto* model = qobject_cast<QStringListModel*>(m_completer->model());
    if (model) {
        model->setStringList(history);
    }
}

/** @brief 重置搜索栏统计计数器 */
void TerminalSearchBar::resetSearchBarStatistics()
{
    m_totalSearches = 0;
    m_totalMatches = 0;
    m_totalReplacements = 0;
    m_totalRegexSearches = 0;
}
