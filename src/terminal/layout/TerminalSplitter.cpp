/**
 * @file TerminalSplitter.cpp
 * @brief 终端分栏布局控件实现
 */

#include "terminal/layout/TerminalSplitter.h"

#include <QSplitter>
#include <QVBoxLayout>

/**
 * @brief 构造函数
 *
 * 创建内部 QSplitter 并初始化为水平分栏。
 *
 * @param parent 父控件指针
 */
TerminalSplitter::TerminalSplitter(QWidget *parent)
    : QWidget(parent)
    , m_splitter(nullptr)
    , m_sectionCount(0)
{
    setObjectName(QStringLiteral("TerminalSplitter"));
    setupUI();
}

/**
 * @brief 析构函数
 */
TerminalSplitter::~TerminalSplitter() = default;

/**
 * @brief 初始化界面布局
 *
 * 创建 QSplitter 并将其设置为控件的主布局。
 */
void TerminalSplitter::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_splitter = new QSplitter(this);
    m_splitter->setOrientation(Qt::Horizontal);
    layout->addWidget(m_splitter);

    setLayout(layout);
}

/**
 * @brief 设置分栏方向
 *
 * 将内部分栏控件的方向切换为水平或垂直。
 *
 * @param orientation Qt::Horizontal 为水平分栏，Qt::Vertical 为垂直分栏
 */
void TerminalSplitter::setOrientation(Qt::Orientation orientation)
{
    if (m_splitter) {
        m_splitter->setOrientation(orientation);
    }
}

/**
 * @brief 添加一个新分栏区域
 *
 * 在分栏控件末尾追加一个新的占位 QWidget，
 * 并发射 sectionAdded 信号。
 *
 * @return 新分栏的索引号
 */
int TerminalSplitter::addSection()
{
    auto *section = new QWidget(m_splitter);
    m_splitter->addWidget(section);
    const int index = m_sectionCount;
    ++m_sectionCount;
    emit sectionAdded(index);
    return index;
}

/**
 * @brief 移除指定索引的分栏区域
 *
 * 从分栏控件中移除指定索引位置的 widget，
 * 并发射 sectionRemoved 信号。
 *
 * @param index 要移除的分栏索引
 */
void TerminalSplitter::removeSection(int index)
{
    Q_UNUSED(index)
    // TODO: 从 m_splitter 中移除 index 对应的 widget
    emit sectionRemoved(index);
}

/**
 * @brief 获取当前分栏数量
 * @return 分栏数量
 */
int TerminalSplitter::sectionCount() const
{
    return m_sectionCount;
}
