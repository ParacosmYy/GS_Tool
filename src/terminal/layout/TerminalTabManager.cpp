/**
 * @file TerminalTabManager.cpp
 * @brief 终端标签页管理控件实现
 */

#include "terminal/layout/TerminalTabManager.h"

#include <QTabWidget>
#include <QVBoxLayout>

/**
 * @brief 构造函数
 *
 * 创建内部 QTabWidget 并连接标签页切换信号。
 *
 * @param parent 父控件指针
 */
TerminalTabManager::TerminalTabManager(QWidget *parent)
    : QWidget(parent)
    , m_tabWidget(nullptr)
{
    setObjectName(QStringLiteral("TerminalTabManager"));
    setupUI();
}

/**
 * @brief 析构函数
 */
TerminalTabManager::~TerminalTabManager() = default;

/**
 * @brief 初始化界面布局
 *
 * 创建 QTabWidget 并将其设置为控件的主布局。
 */
void TerminalTabManager::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    layout->addWidget(m_tabWidget);

    setLayout(layout);
}

/**
 * @brief 添加一个新标签页
 *
 * 创建一个占位 QWidget 作为新标签页内容，
 * 并发射 tabAdded 信号。
 *
 * @param title 标签页标题
 * @return 新标签页的索引号
 */
int TerminalTabManager::addTab(const QString &title)
{
    auto *page = new QWidget(m_tabWidget);
    const int index = m_tabWidget->addTab(page, title);
    emit tabAdded(index);
    return index;
}

/**
 * @brief 移除指定索引的标签页
 *
 * 移除并删除指定索引位置的标签页及其内容 widget，
 * 并发射 tabRemoved 信号。
 *
 * @param index 要移除的标签页索引
 */
void TerminalTabManager::removeTab(int index)
{
    if (m_tabWidget) {
        QWidget *page = m_tabWidget->widget(index);
        m_tabWidget->removeTab(index);
        delete page;
    }
    emit tabRemoved(index);
}

/**
 * @brief 获取标签页数量
 * @return 当前标签页总数
 */
int TerminalTabManager::tabCount() const
{
    return m_tabWidget ? m_tabWidget->count() : 0;
}

/**
 * @brief 获取当前选中标签页的索引
 * @return 当前标签页索引，无标签页时返回 -1
 */
int TerminalTabManager::currentTabIndex() const
{
    return m_tabWidget ? m_tabWidget->currentIndex() : -1;
}
