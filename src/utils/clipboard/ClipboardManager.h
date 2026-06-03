#pragma once
#include <QObject>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QClipboard>

class ClipboardManager : public QObject {
    Q_OBJECT
public:
    struct ClipEntry {
        QString text;
        QByteArray binary;
        qint64 timestamp;
        QString format;
    };

    explicit ClipboardManager(QObject *parent = nullptr);
    ~ClipboardManager() override;
    void pushText(const QString &text);
    void pushBinary(const QByteArray &data, const QString &format = "hex");
    void pushToSystemClipboard(const QString &text);
    QString systemClipboardText() const;
    ClipEntry entry(int index) const;
    QList<ClipEntry> history() const;
    int historySize() const;
    void setMaxHistory(int max);
    void clearHistory();
    void pinEntry(int index);
    void unpinEntry(int index);
signals:
    void entryAdded(const ClipEntry &entry);
    void clipboardChanged();
private:
    void onClipboardChanged();
    QList<ClipEntry> m_history;
    int m_maxHistory = 50;
    QClipboard *m_clipboard = nullptr;
};
