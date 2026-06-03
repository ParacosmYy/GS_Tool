#include "widgets/palette/CommandPalette2.h"
#include <QKeyEvent>
CommandPalette::CommandPalette(QWidget *parent) : QWidget(parent, Qt::Popup) {
    setObjectName("CommandPalette2");
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8,8,8,8);
    m_search = new QLineEdit(this); m_search->setObjectName("paletteSearch");
    m_search->setPlaceholderText(tr("Type a command..."));
    layout->addWidget(m_search);
    m_list = new QListWidget(this); m_list->setObjectName("paletteList");
    layout->addWidget(m_list);
    connect(m_search, &QLineEdit::textChanged, this, &CommandPalette::onTextChanged);
    connect(m_search, &QLineEdit::returnPressed, this, &CommandPalette::onReturnPressed);
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) { Q_UNUSED(item); onReturnPressed(); });
    setFixedSize(400, 300);
}
CommandPalette::~CommandPalette() = default;
void CommandPalette::addCommand(const QString &name, const QString &sc, CommandAction a) { m_commands[name] = {name, sc, a}; }
void CommandPalette::removeCommand(const QString &name) { m_commands.remove(name); }
void CommandPalette::setFilter(const QString &text) { m_search->setText(text); onTextChanged(text); }
void CommandPalette::showPalette() { m_search->clear(); m_list->clear(); onTextChanged(""); show(); m_search->setFocus(); emit paletteShown(); }
void CommandPalette::hidePalette() { hide(); emit paletteHidden(); }
int CommandPalette::commandCount() const { return m_commands.size(); }
void CommandPalette::keyPressEvent(QKeyEvent *e) { if (e->key() == Qt::Key_Escape) hidePalette(); else QWidget::keyPressEvent(e); }
void CommandPalette::onReturnPressed() {
    auto *item = m_list->currentItem(); if (!item) return;
    QString name = item->text().split("\t").first();
    auto it = m_commands.find(name);
    if (it != m_commands.end() && it->action) { it->action(); emit commandExecuted(name); }
    hidePalette();
}
void CommandPalette::onTextChanged(const QString &text) {
    m_list->clear();
    for (auto it = m_commands.constBegin(); it != m_commands.constEnd(); ++it) {
        if (text.isEmpty() || it.key().contains(text, Qt::CaseInsensitive)) {
            QString display = it->shortcut.isEmpty() ? it->name : it->name + "\t" + it->shortcut;
            m_list->addItem(display);
        }
    }
    if (m_list->count() > 0) m_list->setCurrentRow(0);
}
