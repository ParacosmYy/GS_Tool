#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QKeySequence>

class ShortcutManager : public QObject {
    Q_OBJECT
public:
    struct ShortcutEntry { QString id; QString label; QKeySequence defaultKey; QKeySequence currentKey; QString category; };
    explicit ShortcutManager(QObject *parent = nullptr);
    ~ShortcutManager() override;
    void registerShortcut(const QString &id, const QString &label, const QKeySequence &defaultKey, const QString &category = "General");
    void unregisterShortcut(const QString &id);
    void rebind(const QString &id, const QKeySequence &newKey);
    void resetToDefault(const QString &id);
    void resetAll();
    QKeySequence shortcut(const QString &id) const;
    QString shortcutLabel(const QString &id) const;
    QList<ShortcutEntry> allShortcuts() const;
    QList<ShortcutEntry> shortcutsByCategory(const QString &cat) const;
    QStringList categories() const;
    bool hasConflict(const QKeySequence &key, QString *conflictId = nullptr) const;
signals:
    void shortcutRegistered(const QString &id);
    void shortcutRebound(const QString &id, const QKeySequence &newKey);
    void conflictDetected(const QString &id1, const QString &id2);
private:
    QMap<QString, ShortcutEntry> m_shortcuts;
};
