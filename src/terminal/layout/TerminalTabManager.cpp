/**
 * @file TerminalTabManager.cpp
 * @brief 终端标签页管理控件实现
 *
 * 实现基于 QTabWidget 的多标签页终端管理。每个标签页内放置
 * QTextEdit 作为终端占位控件，支持关闭按钮和切换通知。
 */

#include "terminal/layout/TerminalTabManager.h"

#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

/**
 * @brief 构造函数
 *
 * 初始化控件并调用 setupUI() 构建界面。
 *
 * @param parent 父控件指针
 */
TerminalTabManager::TerminalTabManager(QWidget *parent)
    : QWidget(parent)
    , m_tabWidget(nullptr)
    , m_totalTabAdds(0)
    , m_totalTabRemoves(0)
    , m_totalTabSwitches(0)
{
    setObjectName(QStringLiteral("TerminalTabManager"));
    setupUI();
}

/**
 * @brief 析构函数
 *
 * QObject 父子树自动回收子控件，无需手动释放。
 */
TerminalTabManager::~TerminalTabManager() = default;

/**
 * @brief 初始化界面布局
 *
 * 创建 QVBoxLayout 主布局，内嵌 QTabWidget 作为标签页容器。
 * 启用标签页关闭按钮，并连接 tabCloseRequested 和 currentChanged 信号。
 */
void TerminalTabManager::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setObjectName(QStringLiteral("terminalTabWidget"));
    m_tabWidget->setTabsClosable(true);

    /* 标签页关闭按钮点击时自动调用 removeTab */
    connect(m_tabWidget, &QTabWidget::tabCloseRequested,
            this, &TerminalTabManager::removeTab);

    /* 当前标签页切换时转发信号并计数 */
    connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index >= 0) {
            ++m_totalTabSwitches;
        }
        emit currentTabChanged(index);
    });

    mainLayout->addWidget(m_tabWidget);
    setLayout(mainLayout);
}

/**
 * @brief 添加一个新标签页
 *
 * 创建 QTextEdit 作为终端占位控件，添加到 QTabWidget 末尾。
 * 控件以 "terminalTabContent" 命名，便于 QSS 选择器匹配。
 *
 * @param title 标签页标题
 * @return 新标签页的索引号
 */
int TerminalTabManager::addTab(const QString &title)
{
    auto *placeholder = new QTextEdit(m_tabWidget);
    placeholder->setObjectName(QStringLiteral("terminalTabContent"));
    placeholder->setReadOnly(true);
    placeholder->setPlaceholderText(tr("终端内容"));

    const int index = m_tabWidget->addTab(placeholder, title);
    ++m_totalTabAdds;
    emit tabAdded(index);
    return index;
}

/**
 * @brief 移除指定索引的标签页
 *
 * 从 QTabWidget 中移除指定索引位置的标签页并删除其内容控件。
 * 若 m_tabWidget 未初始化或索引无效则不做任何操作。
 *
 * @param index 要移除的标签页索引
 */
void TerminalTabManager::removeTab(int index)
{
    if (!m_tabWidget) {
        return;
    }

    QWidget *page = m_tabWidget->widget(index);
    if (!page) {
        return;
    }

    m_tabWidget->removeTab(index);
    delete page;

    ++m_totalTabRemoves;
    emit tabRemoved(index);
}

/**
 * @brief 获取标签页数量
 * @return 当前标签页总数，m_tabWidget 未初始化时返回 0
 */
int TerminalTabManager::tabCount() const
{
    return m_tabWidget ? m_tabWidget->count() : 0;
}

/**
 * @brief 获取当前选中标签页的索引
 * @return 当前标签页索引，无标签页或未初始化时返回 -1
 */
int TerminalTabManager::currentTabIndex() const
{
    return m_tabWidget ? m_tabWidget->currentIndex() : -1;
}

/**
 * @brief 重置所有统计计数器为零
 */
void TerminalTabManager::resetStatistics()
{
    m_totalTabAdds = 0;
    m_totalTabRemoves = 0;
    m_totalTabSwitches = 0;
}
