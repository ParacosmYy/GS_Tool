#include "utils/clipboard/ClipboardManager.h"
#include <QApplication>
#include <QDateTime>

ClipboardManager::ClipboardManager(QObject *parent) : QObject(parent), m_clipboard(QApplication::clipboard()) {
    if (m_clipboard) connect(m_clipboard, &QClipboard::changed, this, &ClipboardManager::onClipboardChanged);
}
ClipboardManager::~ClipboardManager() = default;

void ClipboardManager::pushText(const QString &text) {
    ClipEntry e; e.text = text; e.timestamp = QDateTime::currentMSecsSinceEpoch(); e.format = "text";
    m_history.prepend(e);
    if (m_history.size() > m_maxHistory) m_history.removeLast();
    emit entryAdded(e);
}

void ClipboardManager::pushBinary(const QByteArray &data, const QString &fmt) {
    ClipEntry e; e.binary = data; e.timestamp = QDateTime::currentMSecsSinceEpoch(); e.format = fmt;
    m_history.prepend(e);
    if (m_history.size() > m_maxHistory) m_history.removeLast();
    emit entryAdded(e);
}

void ClipboardManager::pushToSystemClipboard(const QString &text) {
    if (m_clipboard) m_clipboard->setText(text);
}

QString ClipboardManager::systemClipboardText() const {
    return m_clipboard ? m_clipboard->text() : QString();
}

ClipboardManager::ClipEntry ClipboardManager::entry(int i) const {
    return (i >= 0 && i < m_history.size()) ? m_history[i] : ClipEntry{};
}
QList<ClipboardManager::ClipEntry> ClipboardManager::history() const { return m_history; }
int ClipboardManager::historySize() const { return m_history.size(); }
void ClipboardManager::setMaxHistory(int m) { m_maxHistory = m; }
void ClipboardManager::clearHistory() { m_history.clear(); }
void ClipboardManager::pinEntry(int i) { if (i >= 0 && i < m_history.size()) m_history[i].format = "pinned"; }
void ClipboardManager::unpinEntry(int i) { if (i >= 0 && i < m_history.size()) m_history[i].format = "text"; }

void ClipboardManager::onClipboardChanged() { emit clipboardChanged(); }
