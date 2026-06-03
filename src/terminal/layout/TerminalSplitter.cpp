/**
 * @file TerminalSplitter.cpp
 * @brief 终端分栏布局控件实现
 *
 * 实现可动态增删分栏的终端布局。每个分栏内放置
 * QTextEdit 作为终端占位控件，待后续集成 TerminalWidget。
 */

#include "terminal/layout/TerminalSplitter.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QTextEdit>

/** @brief 构造函数，初始化分栏控件并调用setupUI() @param parent 父控件指针 */
TerminalSplitter::TerminalSplitter(QWidget *parent)
    : QWidget(parent)
    , m_splitter(nullptr)
    , m_sectionCount(0)
    , m_totalSplits(0)
    , m_totalMerges(0)
    , m_totalSplitResized(0)
{
    setObjectName(QStringLiteral("TerminalSplitter"));
    setupUI();
}

/** @brief 析构函数，QObject父子树自动回收子控件 */
TerminalSplitter::~TerminalSplitter() = default;

/** @brief 初始化界面布局(QHBoxLayout+QSplitter分栏容器) */
void TerminalSplitter::setupUI()
{
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_splitter = new QSplitter(this);
    m_splitter->setObjectName(QStringLiteral("terminalSplitter"));
    m_splitter->setOrientation(Qt::Horizontal);

    /* 分割条拖拽时递增调整计数 */
    connect(m_splitter, &QSplitter::splitterMoved, this, [this]() {
        ++m_totalSplitResized;
    });

    mainLayout->addWidget(m_splitter);
    setLayout(mainLayout);
}

/** @brief 设置分栏方向 @param orientation Qt::Horizontal为水平分栏，Qt::Vertical为垂直分栏 */
void TerminalSplitter::setOrientation(Qt::Orientation orientation)
{
    if (m_splitter) {
        m_splitter->setOrientation(orientation);
    }
}

/** @brief 添加一个新分栏区域(创建QTextEdit占位控件) @return 新分栏的索引号(从0开始) */
int TerminalSplitter::addSection()
{
    auto *placeholder = new QTextEdit(m_splitter);
    placeholder->setObjectName(QStringLiteral("terminalSection_%1").arg(m_sectionCount));
    placeholder->setReadOnly(true);
    placeholder->setPlaceholderText(tr("终端 %1").arg(m_sectionCount + 1));

    m_splitter->addWidget(placeholder);
    const int index = m_sectionCount;
    ++m_sectionCount;
    ++m_totalSplits;

    emit sectionAdded(index);
    return index;
}

/** @brief 移除指定索引的分栏区域 @param index 要移除的分栏索引 */
void TerminalSplitter::removeSection(int index)
{
    if (!m_splitter) {
        return;
    }

    QWidget *widget = m_splitter->widget(index);
    if (!widget) {
        return;
    }

    widget->setParent(nullptr);
    delete widget;

    --m_sectionCount;
    ++m_totalMerges;
    emit sectionRemoved(index);
}

/** @brief 获取当前分栏数量 @return 分栏数量 */
int TerminalSplitter::sectionCount() const
{
    return m_sectionCount;
}

/** @brief 重置所有统计计数器为零 */
void TerminalSplitter::resetStatistics()
{
    m_totalSplits = 0;
    m_totalMerges = 0;
    m_totalSplitResized = 0;
}
