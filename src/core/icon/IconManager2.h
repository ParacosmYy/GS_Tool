#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QPixmap>
#include <QSize>
#include <QIcon>

class IconManager : public QObject {
    Q_OBJECT
public:
    explicit IconManager(QObject *parent = nullptr);
    ~IconManager() override;
    void registerIcon(const QString &name, const QString &path);
    void registerThemedIcon(const QString &name, const QString &lightPath, const QString &darkPath);
    QIcon icon(const QString &name) const;
    QPixmap pixmap(const QString &name, const QSize &size) const;
    bool hasIcon(const QString &name) const;
    QStringList availableIcons() const;
    void setTheme(bool dark);
    void preloadAll(const QSize &size);
    void clearCache();
signals:
    void iconRegistered(const QString &name);
    void themeChanged(bool dark);
private:
    QMap<QString, QString> m_lightPaths;
    QMap<QString, QString> m_darkPaths;
    QMap<QString, QIcon> m_cache;
    bool m_darkTheme = false;
};
