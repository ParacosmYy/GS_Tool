/**
 * @file TerminalSearchBar.cpp
 * @brief 终端搜索栏实现 - 嵌入终端顶部的搜索控件
 *
 * 展开/收起动画使用 QPropertyAnimation 驱动 maximumHeight 属性:
 *   - 展开: 0 -> 36, 200ms, QEasingCurve::OutCubic
 *   - 收起: 36 -> 0, 150ms, QEasingCurve::InCubic
 *
 * 搜索历史通过 QCompleter 提供自动补全下拉列表。
 *
 * 搜索操作/历史管理/统计 见 TerminalSearchBarHistory.cpp。
 */

#include "terminal/search/TerminalSearchBar.h"
#include "shared/AnimationConstants.h"
#include "shared/LayoutConstants.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPropertyAnimation>
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
    /* 从QSettings加载最近搜索历史到补全器 */
    loadRecentSearches();
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

// 搜索操作/历史管理/统计(triggerSearch/onSearchTextChanged/isValidHex/
// setResultText/updateSearchHistory/resetSearchBarStatistics)
// 见 TerminalSearchBarHistory.cpp
// 展开/收起动画与访问器方法(activate/deactivate/onCloseClicked/
// searchPattern/isRegexMode/isHexMode/isCaseSensitive/isWholeWord)
// 见 TerminalSearchBarAnimation.cpp
