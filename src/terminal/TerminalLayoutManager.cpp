#include "TerminalLayoutManager.h"
#include "TerminalWidget.h"
#include "TerminalSearchBar.h"
#include "TerminalModel.h"

#include <QVBoxLayout>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

TerminalLayoutManager::TerminalLayoutManager(QObject* parent)
    : QObject(parent)
    , m_mainTerminal(nullptr)
    , m_rxTerminal(nullptr)
    , m_txTerminal(nullptr)
    , m_searchBar(nullptr)
    , m_container(nullptr)
    , m_splitter(nullptr)
    , m_layout(TerminalLayout::Mixed)
    , m_model(nullptr)
    , m_displayMode(DisplayMode::Text)
    , m_showTimestamp(false)
    , m_showDirectionPrefix(false)
{
    // 创建容器widget: 所有布局变化都在这个容器内进行
    m_container = new QWidget;
    m_container->setObjectName("terminalContainer");
    auto* layout = new QVBoxLayout(m_container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}

TerminalLayoutManager::~TerminalLayoutManager()
{
    // m_container 由Qt父子树管理，无需手动delete
}

void TerminalLayoutManager::initialize(TerminalWidget* mainTerminal, TerminalSearchBar* searchBar)
{
    m_mainTerminal = mainTerminal;
    m_searchBar = searchBar;

    // 缓存主终端的初始显示设置
    if (m_mainTerminal) {
        m_displayMode = m_mainTerminal->displayMode();
        m_showTimestamp = m_mainTerminal->showTimestamp();
        m_showDirectionPrefix = m_mainTerminal->showDirectionPrefix();
    }

    // 初始布局: 混合模式
    applyLayout();
}

void TerminalLayoutManager::setTerminalModel(TerminalModel* model)
{
    m_model = model;
}

QWidget* TerminalLayoutManager::container() const
{
    return m_container;
}

TerminalLayout TerminalLayoutManager::layout() const
{
    return m_layout;
}

QList<TerminalWidget*> TerminalLayoutManager::terminalWidgets() const
{
    if (m_layout == TerminalLayout::Mixed) {
        return {m_mainTerminal};
    } else {
        return {m_rxTerminal, m_txTerminal};
    }
}

TerminalWidget* TerminalLayoutManager::primaryTerminal() const
{
    if (m_layout == TerminalLayout::Mixed) {
        return m_mainTerminal;
    }
    return m_rxTerminal;
}

void TerminalLayoutManager::setLayout(int layoutIndex)
{
    // 下拉框索引映射: 0=Mixed, 1=SplitHorizontal, 2=SplitVertical
    TerminalLayout layouts[] = {
        TerminalLayout::Mixed,
        TerminalLayout::SplitHorizontal,
        TerminalLayout::SplitVertical
    };
    if (layoutIndex >= 0 && layoutIndex < 3) {
        setLayout(layouts[layoutIndex]);
    }
}

void TerminalLayoutManager::setLayout(TerminalLayout layout)
{
    if (m_layout == layout) return;
    m_layout = layout;
    applyLayout();
    emit layoutChanged(m_layout);
}

void TerminalLayoutManager::applyLayout()
{
    // 清空容器中的所有子widget
    // 使用QBoxLayout指针以支持addWidget(widget, stretch)语法
    auto* containerLayout = qobject_cast<QBoxLayout*>(m_container->layout());

    // 先从布局中移除所有子项（不删除widget，只是脱离布局）
    while (containerLayout->count() > 0) {
        QLayoutItem* item = containerLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->hide();
            item->widget()->setParent(nullptr);
        }
        delete item;
    }

    // 销毁旧的分割器
    if (m_splitter) {
        m_splitter->deleteLater();
        m_splitter = nullptr;
    }

    // 从主终端缓存最新的显示设置
    if (m_mainTerminal) {
        m_displayMode = m_mainTerminal->displayMode();
        m_showTimestamp = m_mainTerminal->showTimestamp();
        m_showDirectionPrefix = m_mainTerminal->showDirectionPrefix();
    }

    // 搜索栏始终显示在顶部（三种模式共用一个搜索栏）
    if (m_searchBar) {
        m_searchBar->setParent(m_container);
        containerLayout->addWidget(m_searchBar);
        m_searchBar->show();
    }

    if (m_layout == TerminalLayout::Mixed) {
        // ---- 混合模式: 显示主终端（包含TX和RX所有数据） ----
        // 销毁分栏终端（如果存在）
        if (m_rxTerminal) {
            m_rxTerminal->deleteLater();
            m_rxTerminal = nullptr;
        }
        if (m_txTerminal) {
            m_txTerminal->deleteLater();
            m_txTerminal = nullptr;
        }

        // 恢复主终端的无过滤状态
        if (m_mainTerminal) {
            m_mainTerminal->clearDirectionFilter();
            m_mainTerminal->setParent(m_container);
            containerLayout->addWidget(m_mainTerminal, 1);
            m_mainTerminal->show();
        }
    } else {
        // ---- 分栏模式: 创建RX/TX两个终端，用QSplitter分割 ----
        Qt::Orientation orient = (m_layout == TerminalLayout::SplitHorizontal)
            ? Qt::Horizontal : Qt::Vertical;

        // 隐藏主终端（分栏模式下不使用主终端显示数据，但仍保留其模型连接）
        if (m_mainTerminal) {
            m_mainTerminal->hide();
            m_mainTerminal->setParent(nullptr);
        }

        // 创建分割器
        m_splitter = new QSplitter(orient, m_container);
        m_splitter->setObjectName("terminalSplitter");
        m_splitter->setChildrenCollapsible(false);  // 不允许完全折叠子面板

        // 创建/重建分栏终端
        if (m_rxTerminal) {
            m_rxTerminal->deleteLater();
        }
        if (m_txTerminal) {
            m_txTerminal->deleteLater();
        }

        // RX终端: 左侧(左右分栏) 或 上方(上下分栏)
        m_rxTerminal = createSplitTerminal(DataDirection::Rx,
            tr("RX (接收)"));
        m_splitter->addWidget(m_rxTerminal);

        // TX终端: 右侧(左右分栏) 或 下方(上下分栏)
        m_txTerminal = createSplitTerminal(DataDirection::Tx,
            tr("TX (发送)"));
        m_splitter->addWidget(m_txTerminal);

        // 均分分割器空间
        m_splitter->setSizes({1, 1});

        containerLayout->addWidget(m_splitter, 1);
    }
}

TerminalWidget* TerminalLayoutManager::createSplitTerminal(DataDirection direction, const QString& label)
{
    Q_UNUSED(label);

    auto* terminal = new TerminalWidget;
    terminal->setObjectName((direction == DataDirection::Rx)
        ? "rxTerminal" : "txTerminal");

    // 设置共享的数据模型（与主终端共用同一个TerminalModel）
    if (m_model) {
        terminal->setModel(m_model);
    }

    // 设置方向过滤器: 只显示指定方向的数据
    terminal->setDirectionFilter(direction);

    // 同步主终端的显示设置
    syncDisplaySettings(terminal);

    // 分栏终端设置最小尺寸（比主终端小，适合分栏显示）
    terminal->setMinimumSize(200, 100);

    return terminal;
}

void TerminalLayoutManager::syncDisplaySettings(TerminalWidget* target) const
{
    if (!target) return;
    target->setDisplayMode(m_displayMode);
    target->setShowTimestamp(m_showTimestamp);
    target->setShowDirectionPrefix(m_showDirectionPrefix);
    target->setAutoScroll(true);
}
