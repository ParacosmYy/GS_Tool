/**
 * @file ClipboardManager.h
 * @brief 剪贴板管理器 - 统一管理应用内剪贴板操作和历史
 * @since score-131
 */
#ifndef CLIPBOARDMANAGER_H
#define CLIPBOARDMANAGER_H
#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>

struct ClipboardEntry {
    QString text;
    QString format;
    qint64 timestampMs;
    QString source;
    bool isValid() const { return !text.isEmpty(); }
};

class ClipboardManager : public QObject {
    Q_OBJECT
public:
    static ClipboardManager &instance();
    void copyText(const QString &text, const QString &source = QString());
    void copyHex(const QByteArray &data, const QString &source = QString());
    void copyBase64(const QByteArray &data, const QString &source = QString());
    QString currentText() const;
    QString pasteText() const;
    static QString textToHex(const QString &text);
    static QByteArray hexToBytes(const QString &hexStr);
    static QString bytesToBase64(const QByteArray &data);
    static QByteArray base64ToBytes(const QString &base64);
    static QString textToEscape(const QString &text);
    QList<ClipboardEntry> history() const;
    QList<ClipboardEntry> recentHistory(int count) const;
    void clearHistory();
    int historySize() const;
    void setMaxHistorySize(int maxSize);
    int maxHistorySize() const;
    void restoreFromHistory(int index);
    quint64 totalCopyOps() const;
    quint64 totalPasteOps() const;
    quint64 totalConversions() const;
    void resetStatistics();
signals:
    void historyEntryAdded(const ClipboardEntry &entry);
    void historyCleared();
    void clipboardContentChanged();
private:
    explicit ClipboardManager(QObject *parent = nullptr);
    ~ClipboardManager() override;
    ClipboardManager(const ClipboardManager &) = delete;
    ClipboardManager &operator=(const ClipboardManager &) = delete;
    void addHistoryEntry(const ClipboardEntry &entry);
    QList<ClipboardEntry> m_history;
    int m_maxHistorySize = 100;
    mutable quint64 m_totalCopyOps = 0;
    mutable quint64 m_totalPasteOps = 0;
    mutable quint64 m_totalConversions = 0;
};
#endif // CLIPBOARDMANAGER_H
