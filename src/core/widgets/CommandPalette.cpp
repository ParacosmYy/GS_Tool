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

// ─── 搜索过滤 ───────────────────────────────────────────

/** @brief 搜索框文字变更时触发，重新过滤并刷新命令列表 @param text 当前搜索文本 */
void CommandPalette::onSearchChanged(const QString& text)
{
    ++m_totalSearches;
    refreshList(text);
}

// ─── 执行命令 ───────────────────────────────────────────

/** @brief 命令列表项激活时触发，执行对应命令并隐藏面板 @param item 激活的列表项指针 */
void CommandPalette::onItemActivated(QListWidgetItem* item)
{
    if (!item) return;

    int idx = item->data(Qt::UserRole).toInt();
    if (idx >= 0 && idx < m_commands.size()) {
        const auto& cmd = m_commands[idx];
        if (cmd.action) {
            ++m_totalExecutions;
            cmd.action();
        }
        emit commandExecuted(cmd.id);
    }
    hidePalette();
}

// ─── 刷新命令列表 ───────────────────────────────────────

/** @brief 根据过滤文本刷新命令列表，使用模糊子序列匹配 @param filter 过滤文本，为空时显示全部命令 */
void CommandPalette::refreshList(const QString& filter)
{
    m_listWidget->clear();
    m_filteredIndices.clear();

    for (int i = 0; i < m_commands.size(); ++i) {
        const auto& cmd = m_commands[i];
        bool match = filter.isEmpty()
                     || fuzzyMatch(filter, cmd.label)
                     || fuzzyMatch(filter, cmd.category);
        if (!match) continue;

        QString text = tr("%1  %2").arg(cmd.category, cmd.label);
        if (!cmd.shortcut.isEmpty())
            text += tr("  [%1]").arg(cmd.shortcut);

        auto* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, i);
        m_listWidget->addItem(item);
        m_filteredIndices.append(i);
    }

    if (m_listWidget->count() > 0)
        m_listWidget->setCurrentRow(0);
}

// ─── 模糊子序列匹配 ─────────────────────────────────────

/** @brief 模糊子序列匹配算法，判断filter是否为target的子序列 @param filter 搜索过滤器文本 @param target 目标匹配文本 @return true表示匹配成功 */
bool CommandPalette::fuzzyMatch(const QString& filter, const QString& target) const
{
    if (filter.isEmpty()) return true;

    const QString f = filter.toLower();
    const QString t = target.toLower();

    int fi = 0;
    for (int ti = 0; ti < t.length() && fi < f.length(); ++ti) {
        if (t[ti] == f[fi])
            ++fi;
    }
    return fi == f.length();
}

// ============================================================================
// 统计计数器
// ============================================================================

/** @brief 重置命令面板统计计数器(搜索/执行/键盘事件) */
void CommandPalette::resetPaletteStatistics()
{
    m_totalSearches = 0;
    m_totalExecutions = 0;
    m_totalKeyEvents = 0;
}
