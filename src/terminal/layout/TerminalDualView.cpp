/**
 * @file TerminalDualView.cpp
 * @brief 文本/十六进制双视图控件实现
 *
 * 左右水平分栏布局：左侧 QTextEdit 显示文本视图，
 * 右侧 QTextEdit 显示十六进制视图。使用等宽数学字体
 * 确保十六进制对齐。
 */

#include "terminal/layout/TerminalDualView.h"

#include <QFont>
#include <QList>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

/**
 * @brief 构造函数
 *
 * 初始化控件并调用 setupUI() 构建双视图界面。
 *
 * @param parent 父控件指针
 */
TerminalDualView::TerminalDualView(QWidget *parent)
    : QWidget(parent)
    , m_textView(nullptr)
    , m_hexView(nullptr)
    , m_splitter(nullptr)
    , m_totalViewSwitches(0)
    , m_totalSyncs(0)
    , m_totalSplitsChanged(0)
{
    setObjectName(QStringLiteral("TerminalDualView"));
    setupUI();
}

/**
 * @brief 析构函数
 *
 * QObject 父子树自动回收子控件，无需手动释放。
 */
TerminalDualView::~TerminalDualView() = default;

/**
 * @brief 初始化界面布局
 *
 * 创建水平 QSplitter，左侧放置文本视图 QTextEdit，
 * 右侧放置十六进制视图 QTextEdit。初始比例为 1:1 (400:400)。
 *
 * - textView: 只读，带占位文本 "Text View"
 * - hexView:  只读，使用 Courier New 9pt 等宽字体，占位文本 "Hex View"
 */
void TerminalDualView::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    /* 水平分割器，左右布局 */
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setObjectName(QStringLiteral("dualViewSplitter"));

    /* 左侧：文本视图 */
    m_textView = new QTextEdit(this);
    m_textView->setObjectName(QStringLiteral("textView"));
    m_textView->setReadOnly(true);
    m_textView->setPlaceholderText(tr("文本视图"));

    /* 右侧：十六进制视图 */
    m_hexView = new QTextEdit(this);
    m_hexView->setObjectName(QStringLiteral("hexView"));
    m_hexView->setReadOnly(true);
    m_hexView->setFont(QFont(QStringLiteral("Courier New"), 9));
    m_hexView->setPlaceholderText(tr("十六进制视图"));

    m_splitter->addWidget(m_textView);
    m_splitter->addWidget(m_hexView);
    m_splitter->setSizes(QList<int>() << 400 << 400);

    /* 分割条拖拽时递增变更计数 */
    connect(m_splitter, &QSplitter::splitterMoved, this, [this]() {
        ++m_totalSplitsChanged;
    });

    mainLayout->addWidget(m_splitter);
    setLayout(mainLayout);
}

/**
 * @brief 重置所有统计计数器为零
 */
void TerminalDualView::resetStatistics()
{
    m_totalViewSwitches = 0;
    m_totalSyncs = 0;
    m_totalSplitsChanged = 0;
}
