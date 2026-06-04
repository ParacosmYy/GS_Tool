/**
 * @file CommandPalette2.cpp
 * @brief 命令面板v2实现 — 模糊搜索命令列表+快捷键显示+执行
 */
#include "widgets/palette/CommandPalette2.h"
#include <QKeyEvent>

/** @brief 构造函数，创建搜索框+命令列表弹出窗口 @param parent 父Widget */
CommandPalette::CommandPalette(QWidget *parent) : QWidget(parent, Qt::Popup) {
    setObjectName("CommandPalette2");
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8,8,8,8);
    m_search = new QLineEdit(this); m_search->setObjectName("paletteSearch");
    m_search->setPlaceholderText(tr("输入命令..."));
    layout->addWidget(m_search);
    m_list = new QListWidget(this); m_list->setObjectName("paletteList");
    layout->addWidget(m_list);
    connect(m_search, &QLineEdit::textChanged, this, &CommandPalette::onTextChanged);
    connect(m_search, &QLineEdit::returnPressed, this, &CommandPalette::onReturnPressed);
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) { Q_UNUSED(item); onReturnPressed(); });
    setFixedSize(400, 300);
}
/** @brief 析构函数 */
CommandPalette::~CommandPalette() = default;
/** @brief 注册命令 @param name 命令名称 @param sc 快捷键描述 @param a 执行回调 */
void CommandPalette::addCommand(const QString &name, const QString &sc, CommandAction a) { m_commands[name] = {name, sc, a}; }
/** @brief 移除命令 @param name 命令名称 */
void CommandPalette::removeCommand(const QString &name) { m_commands.remove(name); }
/** @brief 设置搜索过滤器文本 @param text 过滤文本 */
void CommandPalette::setFilter(const QString &text) { m_search->setText(text); onTextChanged(text); }
/** @brief 显示命令面板并聚焦搜索框 */
void CommandPalette::showPalette() { ++m_totalPaletteShows; m_search->clear(); m_list->clear(); onTextChanged(""); show(); m_search->setFocus(); emit paletteShown(); }
/** @brief 隐藏命令面板 */
void CommandPalette::hidePalette() { hide(); emit paletteHidden(); }
/** @brief 获取已注册命令数量 @return 命令数 */
int CommandPalette::commandCount() const { return m_commands.size(); }
/** @brief 按键事件处理 — Escape关闭面板 @param e 按键事件 */
void CommandPalette::keyPressEvent(QKeyEvent *e) { if (e->key() == Qt::Key_Escape) hidePalette(); else QWidget::keyPressEvent(e); }

/** @brief 回车执行当前选中命令 */
void CommandPalette::onReturnPressed() {
    auto *item = m_list->currentItem(); if (!item) return;
    QString name = item->text().split("\t").first();
    auto it = m_commands.find(name);
    if (it != m_commands.end() && it->action) { ++m_totalCommandsExecuted; it->action(); emit commandExecuted(name); }
    hidePalette();
}

/** @brief 搜索文本变化时过滤命令列表 @param text 搜索文本 */
void CommandPalette::onTextChanged(const QString &text) {
    ++m_totalFilterChanges;
    m_list->clear();
    for (auto it = m_commands.constBegin(); it != m_commands.constEnd(); ++it) {
        if (text.isEmpty() || it.key().contains(text, Qt::CaseInsensitive)) {
            QString display = it->shortcut.isEmpty() ? it->name : it->name + "\t" + it->shortcut;
            m_list->addItem(display);
        }
    }
    if (m_list->count() > 0) m_list->setCurrentRow(0);
}
