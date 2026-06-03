/**
 * @file ClipboardManager.cpp
 * @brief 剪贴板管理器实现 — 复制/粘贴/格式转换/历史记录
 * @since score-131
 */
#include "core/clipboard/ClipboardManager.h"
#include <QApplication>
#include <QClipboard>
#include <QDateTime>

/** @brief 构造函数 - 初始化剪贴板监听 @param parent 父对象 */
ClipboardManager::ClipboardManager(QObject *parent) : QObject(parent) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) connect(clipboard, &QClipboard::dataChanged, this, &ClipboardManager::clipboardContentChanged);
}
/** @brief 析构函数 */
ClipboardManager::~ClipboardManager() = default;
/** @brief 获取单例实例 @return 剪贴板管理器引用 */
ClipboardManager &ClipboardManager::instance() { static ClipboardManager inst; return inst; }

/** @brief 复制文本到剪贴板 @param text 文本内容 @param source 来源标识 */
void ClipboardManager::copyText(const QString &text, const QString &source) {
    if (text.isEmpty()) return;
    ++m_totalCopyOps;
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) clipboard->setText(text);
    ClipboardEntry entry; entry.text = text; entry.format = QStringLiteral("text");
    entry.timestampMs = QDateTime::currentMSecsSinceEpoch(); entry.source = source;
    addHistoryEntry(entry);
}
/** @brief 复制十六进制数据到剪贴板 @param data 原始数据 @param source 来源标识 */
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
/** @brief 复制Base64编码数据到剪贴板 @param data 原始数据 @param source 来源标识 */
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
/** @brief 获取当前剪贴板文本 @return 剪贴板文本内容 */
QString ClipboardManager::currentText() const {
    QClipboard *clipboard = QGuiApplication::clipboard();
    return clipboard ? clipboard->text() : QString();
}
/** @brief 粘贴文本并累计粘贴计数 @return 剪贴板文本 */
QString ClipboardManager::pasteText() const { ++m_totalPasteOps; return currentText(); }

/** @brief 文本转十六进制字符串 @param text 原始文本 @return 十六进制格式字符串 */
QString ClipboardManager::textToHex(const QString &text) {
    QByteArray data = text.toUtf8();
    QStringList hexParts; hexParts.reserve(data.size());
    for (unsigned char byte : data) hexParts.append(QStringLiteral("%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper());
    return hexParts.join(QLatin1Char(' '));
}
/** @brief 十六进制字符串转字节数组 @param hexStr 十六进制字符串 @return 字节数组 */
QByteArray ClipboardManager::hexToBytes(const QString &hexStr) {
    QString cleaned = hexStr;
    cleaned.remove(QLatin1Char(' ')); cleaned.remove(QLatin1Char('\n')); cleaned.remove(QLatin1Char('\r'));
    if (cleaned.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) cleaned = cleaned.mid(2);
    return QByteArray::fromHex(cleaned.toUtf8());
}
/** @brief 字节数组转Base64字符串 @param data 原始数据 @return Base64编码字符串 */
QString ClipboardManager::bytesToBase64(const QByteArray &data) { return QString::fromUtf8(data.toBase64()); }
/** @brief Base64字符串转字节数组 @param base64 Base64编码字符串 @return 解码后字节数组 */
QByteArray ClipboardManager::base64ToBytes(const QString &base64) { return QByteArray::fromBase64(base64.toUtf8()); }
/** @brief 文本转转义字符串(不可见字符转\xHH) @param text 原始文本 @return 转义后字符串 */
QString ClipboardManager::textToEscape(const QString &text) {
    QByteArray data = text.toUtf8();
    QStringList parts; parts.reserve(data.size());
    for (unsigned char byte : data) {
        if (byte >= 0x20 && byte < 0x7F) parts.append(QChar(byte));
        else parts.append(QStringLiteral("\\x%1").arg(byte, 2, 16, QLatin1Char('0')));
    }
    return parts.join(QString());
}

/** @brief 获取剪贴板历史记录 @return 历史条目列表 */
QList<ClipboardEntry> ClipboardManager::history() const { return m_history; }
/** @brief 获取最近N条历史记录 @param count 获取数量 @return 最近的剪贴板条目 */
QList<ClipboardEntry> ClipboardManager::recentHistory(int count) const {
    if (count <= 0 || m_history.isEmpty()) return QList<ClipboardEntry>();
    return m_history.mid(0, qMin(count, m_history.size()));
}
/** @brief 清除所有历史记录 */
void ClipboardManager::clearHistory() { m_history.clear(); emit historyCleared(); }
/** @brief 获取历史记录数量 @return 历史条目数 */
int ClipboardManager::historySize() const { return m_history.size(); }
/** @brief 设置历史记录最大容量 @param maxSize 最大条目数 */
void ClipboardManager::setMaxHistorySize(int maxSize) {
    m_maxHistorySize = qMax(1, maxSize);
    while (m_history.size() > m_maxHistorySize) m_history.removeLast();
}
/** @brief 获取历史记录最大容量 @return 最大条目数 */
int ClipboardManager::maxHistorySize() const { return m_maxHistorySize; }
/** @brief 从历史记录恢复到剪贴板 @param index 历史索引 */
void ClipboardManager::restoreFromHistory(int index) {
    if (index < 0 || index >= m_history.size()) return;
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) clipboard->setText(m_history[index].text);
    ++m_totalCopyOps;
}

/** @brief 获取累计复制操作次数 @return 复制次数 */
quint64 ClipboardManager::totalCopyOps() const { return m_totalCopyOps; }
/** @brief 获取累计粘贴操作次数 @return 粘贴次数 */
quint64 ClipboardManager::totalPasteOps() const { return m_totalPasteOps; }
/** @brief 获取累计格式转换次数 @return 转换次数 */
quint64 ClipboardManager::totalConversions() const { return m_totalConversions; }
/** @brief 重置所有统计计数器 */
void ClipboardManager::resetStatistics() { m_totalCopyOps = 0; m_totalPasteOps = 0; m_totalConversions = 0; }

/** @brief 添加条目到历史记录(去重，限制容量) @param entry 剪贴板条目 */
void ClipboardManager::addHistoryEntry(const ClipboardEntry &entry) {
    if (!m_history.isEmpty() && m_history.first().text == entry.text) { m_history.first().timestampMs = entry.timestampMs; return; }
    m_history.prepend(entry);
    while (m_history.size() > m_maxHistorySize) m_history.removeLast();
    emit historyEntryAdded(entry);
}
