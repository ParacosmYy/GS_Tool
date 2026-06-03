/**
 * @file ClipboardManager.cpp
 * @brief 剪贴板管理器实现
 * @since score-131
 */
#include "core/clipboard/ClipboardManager.h"
#include <QApplication>
#include <QClipboard>
#include <QDateTime>

ClipboardManager::ClipboardManager(QObject *parent) : QObject(parent) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) connect(clipboard, &QClipboard::dataChanged, this, &ClipboardManager::clipboardContentChanged);
}
ClipboardManager::~ClipboardManager() = default;
ClipboardManager &ClipboardManager::instance() { static ClipboardManager inst; return inst; }

void ClipboardManager::copyText(const QString &text, const QString &source) {
    if (text.isEmpty()) return;
    ++m_totalCopyOps;
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) clipboard->setText(text);
    ClipboardEntry entry; entry.text = text; entry.format = QStringLiteral("text");
    entry.timestampMs = QDateTime::currentMSecsSinceEpoch(); entry.source = source;
    addHistoryEntry(entry);
}
void ClipboardManager::copyHex(const QByteArray &data, const QString &source) {
    if (data.isEmpty()) return;
    ++m_totalCopyOps;
    QString hexStr = textToHex(QString::fromUtf8(data));
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) clipboard->setText(hexStr);
    ClipboardEntry entry; entry.text = hexStr; entry.format = QStringLiteral("hex");
    entry.timestampMs = QDateTime::currentMSecsSinceEpoch(); entry.source = source;
    addHistoryEntry(entry);
}
void ClipboardManager::copyBase64(const QByteArray &data, const QString &source) {
    if (data.isEmpty()) return;
    ++m_totalCopyOps;
    QString b64 = bytesToBase64(data);
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) clipboard->setText(b64);
    ClipboardEntry entry; entry.text = b64; entry.format = QStringLiteral("base64");
    entry.timestampMs = QDateTime::currentMSecsSinceEpoch(); entry.source = source;
    addHistoryEntry(entry);
}
QString ClipboardManager::currentText() const {
    QClipboard *clipboard = QGuiApplication::clipboard();
    return clipboard ? clipboard->text() : QString();
}
QString ClipboardManager::pasteText() const { ++m_totalPasteOps; return currentText(); }

QString ClipboardManager::textToHex(const QString &text) {
    QByteArray data = text.toUtf8();
    QStringList hexParts; hexParts.reserve(data.size());
    for (unsigned char byte : data) hexParts.append(QStringLiteral("%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper());
    return hexParts.join(QLatin1Char(' '));
}
QByteArray ClipboardManager::hexToBytes(const QString &hexStr) {
    QString cleaned = hexStr;
    cleaned.remove(QLatin1Char(' ')); cleaned.remove(QLatin1Char('\n')); cleaned.remove(QLatin1Char('\r'));
    if (cleaned.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) cleaned = cleaned.mid(2);
    return QByteArray::fromHex(cleaned.toUtf8());
}
QString ClipboardManager::bytesToBase64(const QByteArray &data) { return QString::fromUtf8(data.toBase64()); }
QByteArray ClipboardManager::base64ToBytes(const QString &base64) { return QByteArray::fromBase64(base64.toUtf8()); }
QString ClipboardManager::textToEscape(const QString &text) {
    QByteArray data = text.toUtf8();
    QStringList parts; parts.reserve(data.size());
    for (unsigned char byte : data) {
        if (byte >= 0x20 && byte < 0x7F) parts.append(QChar(byte));
        else parts.append(QStringLiteral("\\x%1").arg(byte, 2, 16, QLatin1Char('0')));
    }
    return parts.join(QString());
}

QList<ClipboardEntry> ClipboardManager::history() const { return m_history; }
QList<ClipboardEntry> ClipboardManager::recentHistory(int count) const {
    if (count <= 0 || m_history.isEmpty()) return QList<ClipboardEntry>();
    return m_history.mid(0, qMin(count, m_history.size()));
}
void ClipboardManager::clearHistory() { m_history.clear(); emit historyCleared(); }
int ClipboardManager::historySize() const { return m_history.size(); }
void ClipboardManager::setMaxHistorySize(int maxSize) {
    m_maxHistorySize = qMax(1, maxSize);
    while (m_history.size() > m_maxHistorySize) m_history.removeLast();
}
int ClipboardManager::maxHistorySize() const { return m_maxHistorySize; }
void ClipboardManager::restoreFromHistory(int index) {
    if (index < 0 || index >= m_history.size()) return;
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) clipboard->setText(m_history[index].text);
    ++m_totalCopyOps;
}

quint64 ClipboardManager::totalCopyOps() const { return m_totalCopyOps; }
quint64 ClipboardManager::totalPasteOps() const { return m_totalPasteOps; }
quint64 ClipboardManager::totalConversions() const { return m_totalConversions; }
void ClipboardManager::resetStatistics() { m_totalCopyOps = 0; m_totalPasteOps = 0; m_totalConversions = 0; }

void ClipboardManager::addHistoryEntry(const ClipboardEntry &entry) {
    if (!m_history.isEmpty() && m_history.first().text == entry.text) { m_history.first().timestampMs = entry.timestampMs; return; }
    m_history.prepend(entry);
    while (m_history.size() > m_maxHistorySize) m_history.removeLast();
    emit historyEntryAdded(entry);
}
