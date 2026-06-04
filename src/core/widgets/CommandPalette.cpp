/**
 * @file CommandPalette.cpp
 * @brief 命令面板实现 — Ctrl+P 模糊搜索快速导航
 *
 * 浮动在主窗口中央的搜索面板。半透明背景遮罩 + 圆角面板。
 * 支持模糊子序列匹配(如输入"ter"匹配"终端Terminal")。
 */

#include "core/widgets/CommandPalette.h"

#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <QEvent>

#include "core/theme/ThemeManager.h"

// ─── 构造 ───────────────────────────────────────────────

/** @brief 构造命令面板控件，初始化搜索框和命令列表 @param parent 父控件指针 */
CommandPalette::CommandPalette(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(480, 400);
    setObjectName("commandPaletteOverlay");
    installEventFilter(this);

    // 内部面板容器(白色/深色圆角面板)
    m_panelWidget = new QWidget(this);
    m_panelWidget->setObjectName("commandPalettePanel");
    m_panelWidget->setFixedSize(480, 400);

    auto* layout = new QVBoxLayout(m_panelWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 搜索框
    m_searchEdit = new QLineEdit(m_panelWidget);
    m_searchEdit->setObjectName("commandPaletteSearch");
    m_searchEdit->setFixedHeight(40);
    m_searchEdit->setPlaceholderText(tr("搜索面板或命令..."));
    QFont searchFont;
    searchFont.setPointSize(14);
    m_searchEdit->setFont(searchFont);
    layout->addWidget(m_searchEdit);

    // 命令列表
    m_listWidget = new QListWidget(m_panelWidget);
    m_listWidget->setObjectName("commandPaletteList");
    m_listWidget->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    layout->addWidget(m_listWidget);

    // 信号连接
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &CommandPalette::onSearchChanged);
    connect(m_listWidget, &QListWidget::itemActivated,
            this, &CommandPalette::onItemActivated);
    connect(m_listWidget, &QListWidget::itemClicked,
            this, &CommandPalette::onItemActivated);
}

// ─── 命令注册 ───────────────────────────────────────────

/** @brief 注册单个命令条目到面板 @param entry 命令条目(含id/label/category/shortcut/action) */
void CommandPalette::registerCommand(const CommandEntry& entry)
{
    m_commands.append(entry);
}

/** @brief 批量注册多个命令条目到面板 @param entries 命令条目向量 */
void CommandPalette::registerCommands(const QVector<CommandEntry>& entries)
{
    m_commands.append(entries);
}

// ─── 显示/隐藏 ─────────────────────────────────────────

/** @brief 显示命令面板，居中于父窗口并偏上100px，自动清空搜索框并聚焦 */
void CommandPalette::showPalette()
{
    // 居中于父窗口(偏上100px)
    if (parentWidget()) {
        QPoint center = parentWidget()->geometry().center();
        move(center.x() - width() / 2, center.y() - height() / 2 - 100);
    }

    m_searchEdit->clear();
    refreshList();
    m_searchEdit->setFocus();

    show();
    raise();
}

/** @brief 隐藏命令面板 */
void CommandPalette::hidePalette()
{
    hide();
}

// ─── 事件过滤(ESC关闭) ─────────────────────────────────

/** @brief 事件过滤器，处理ESC关闭/上下键导航列表 @param obj 事件目标对象 @param event 事件指针 @return true表示事件已拦截处理 */
bool CommandPalette::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        ++m_totalKeyEvents;
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            hidePalette();
            return true;
        }
        // 上下键导航列表
        if (keyEvent->key() == Qt::Key_Down) {
            int row = m_listWidget->currentRow() + 1;
            if (row < m_listWidget->count())
                m_listWidget->setCurrentRow(row);
            return true;
        }
        if (keyEvent->key() == Qt::Key_Up) {
            int row = m_listWidget->currentRow() - 1;
            if (row >= 0)
                m_listWidget->setCurrentRow(row);
            return true;
        }
    }
    // 点击面板外部关闭(Qt::Popup已自动处理，此处作为后备)
    return QWidget::eventFilter(obj, event);
}

// ─── 绘制半透明遮罩 ─────────────────────────────────────

/** @brief 自绘事件，绘制半透明背景遮罩和圆角面板边框 @param event 绘制事件(未使用) */
void CommandPalette::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 半透明背景遮罩
    QColor overlay = ThemeManager::instance().color(ThemeManager::SemanticColor::BgPrimary);
    overlay.setAlpha(120);
    p.fillRect(rect(), overlay);

    // 面板圆角矩形背景
    QColor panelBg = ThemeManager::instance().color(ThemeManager::SemanticColor::BgSecondary);
    QPainterPath path;
    path.addRoundedRect(m_panelWidget->geometry(), 12, 12);
    p.fillPath(path, panelBg);

    // 面板边框
    QColor borderColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Border);
    p.setPen(borderColor);
    p.drawPath(path);
}

// onSearchChanged/onItemActivated/refreshList/fuzzyMatch/resetPaletteStatistics
// 已移至 CommandPaletteFilter.cpp
