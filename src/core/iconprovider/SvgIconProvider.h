#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QPixmap>
#include <QSize>
#include <QColor>

class SvgIconProvider : public QObject {
    Q_OBJECT
public:
    explicit SvgIconProvider(QObject *parent = nullptr);
    ~SvgIconProvider() override;
    void registerIcon(const QString &name, const QString &svgPath);
    void registerSvgData(const QString &name, const QByteArray &svgData);
    QPixmap icon(const QString &name, const QSize &size, const QColor &color = QColor()) const;
    QPixmap tintedIcon(const QString &name, const QSize &size, const QColor &tintColor) const;
    bool hasIcon(const QString &name) const;
    QStringList availableIcons() const;
    void setDefaultColor(const QColor &color);
    void clearCache();
    void preloadIcons(const QStringList &names, const QSize &size);
signals:
    void iconRegistered(const QString &name);
    void iconLoaded(const QString &name);
private:
    QByteArray applyColor(const QByteArray &svg, const QColor &color) const;
    QMap<QString, QByteArray> m_svgData;
    QMap<QString, QMap<quint64, QPixmap>> m_cache;
    QColor m_defaultColor;
};
