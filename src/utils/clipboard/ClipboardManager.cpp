/**
 * @file ClipboardManager.cpp
 * @brief 剪贴板管理器实现 - 管理剪贴板历史记录和系统剪贴板交互
 */

#include "utils/clipboard/ClipboardManager.h"
#include <QApplication>
#include <QDateTime>

/** @brief 构造剪贴板管理器，连接系统剪贴板变化信号 @param parent 父对象指针 */
ClipboardManager::ClipboardManager(QObject *parent) : QObject(parent), m_clipboard(QApplication::clipboard()) {
    if (m_clipboard) connect(m_clipboard, &QClipboard::changed, this, &ClipboardManager::onClipboardChanged);
}

/** @brief 析构函数，使用默认实现 */
ClipboardManager::~ClipboardManager() = default;

/**
 * @brief 添加一条文本记录到历史列表
 * @param text 要记录的文本内容
 */
void ClipboardManager::pushText(const QString &text) {
    ++m_totalTextPushes;
    ClipEntry e; e.text = text; e.timestamp = QDateTime::currentMSecsSinceEpoch(); e.format = "text";
    m_history.prepend(e);
    if (m_history.size() > m_maxHistory) { m_history.removeLast(); ++m_totalEvictions; }
    emit entryAdded(e);
}

/**
 * @brief 添加一条二进制数据记录到历史列表
 * @param data 二进制数据内容
 * @param fmt 数据格式标识
 */
void ClipboardManager::pushBinary(const QByteArray &data, const QString &fmt) {
    ++m_totalBinaryPushes;
    ClipEntry e; e.binary = data; e.timestamp = QDateTime::currentMSecsSinceEpoch(); e.format = fmt;
    m_history.prepend(e);
    if (m_history.size() > m_maxHistory) { m_history.removeLast(); ++m_totalEvictions; }
    emit entryAdded(e);
}

/**
 * @brief 将文本内容写入系统剪贴板
 * @param text 要写入的文本内容
 */
void ClipboardManager::pushToSystemClipboard(const QString &text) {
    ++m_totalSystemWrites;
    if (m_clipboard) m_clipboard->setText(text);
}

/**
 * @brief 获取系统剪贴板中的文本内容
 * @return 系统剪贴板文本，剪贴板不可用时返回空字符串
 */
QString ClipboardManager::systemClipboardText() const {
    return m_clipboard ? m_clipboard->text() : QString();
}

/**
 * @brief 按索引获取历史记录条目
 * @param i 条目索引
 * @return 对应的剪贴板记录条目，索引越界时返回空条目
 */
ClipboardManager::ClipEntry ClipboardManager::entry(int i) const {
    return (i >= 0 && i < m_history.size()) ? m_history[i] : ClipEntry{};
}

/** @brief 获取完整的剪贴板历史记录列表 @return 历史记录列表 */
QList<ClipboardManager::ClipEntry> ClipboardManager::history() const { return m_history; }

/** @brief 获取当前历史记录条数 @return 历史记录数量 */
int ClipboardManager::historySize() const { return m_history.size(); }

/**
 * @brief 设置历史记录最大保留条数
 * @param m 最大条数
 */
void ClipboardManager::setMaxHistory(int m) { m_maxHistory = m; }

/** @brief 清空所有历史记录 */
void ClipboardManager::clearHistory() { m_history.clear(); ++m_totalClears; }

/**
 * @brief 将指定索引的记录标记为已固定
 * @param i 条目索引
 */
void ClipboardManager::pinEntry(int i) { if (i >= 0 && i < m_history.size()) { m_history[i].format = "pinned"; ++m_totalPins; } }

/**
 * @brief 将指定索引的记录取消固定标记
 * @param i 条目索引
 */
void ClipboardManager::unpinEntry(int i) { if (i >= 0 && i < m_history.size()) { m_history[i].format = "text"; ++m_totalUnpins; } }

/** @brief 系统剪贴板内容变化时的内部槽函数，转发clipboardChanged信号 */
void ClipboardManager::onClipboardChanged() { ++m_totalClipboardChanges; emit clipboardChanged(); }

// ============================================================================
// 统计接口
// ============================================================================

/** @brief 获取累计推送文本次数 @return 计数 */
quint64 ClipboardManager::totalTextPushes() const { return m_totalTextPushes; }
/** @brief 获取累计推送二进制次数 @return 计数 */
quint64 ClipboardManager::totalBinaryPushes() const { return m_totalBinaryPushes; }
/** @brief 获取累计系统剪贴板写入次数 @return 计数 */
quint64 ClipboardManager::totalSystemWrites() const { return m_totalSystemWrites; }
/** @brief 获取累计系统剪贴板变更事件次数 @return 计数 */
quint64 ClipboardManager::totalClipboardChanges() const { return m_totalClipboardChanges; }
/** @brief 获取累计固定条目次数 @return 计数 */
quint64 ClipboardManager::totalPins() const { return m_totalPins; }

/** @brief 重置所有剪贴板管理器统计计数器(文本/二进制/系统写入/变更/固定/取消固定/清空/淘汰) */
void ClipboardManager::resetClipboardStatistics() {
    m_totalTextPushes = 0;
    m_totalBinaryPushes = 0;
    m_totalSystemWrites = 0;
    m_totalClipboardChanges = 0;
    m_totalPins = 0;
    m_totalUnpins = 0;
    m_totalClears = 0;
    m_totalEvictions = 0;
}
