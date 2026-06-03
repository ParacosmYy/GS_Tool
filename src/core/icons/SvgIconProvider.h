/**
 * @file SvgIconProvider.h
 * @brief SVG图标管理器 - 统一管理应用内所有SVG图标的加载、缓存和着色
 * @since score-131
 */
#ifndef SVGICONPROVIDER_H
#define SVGICONPROVIDER_H

#include <QIcon>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QStringList>

class SvgIconProvider : public QObject
{
    Q_OBJECT
public:
    static SvgIconProvider &instance();
    void initialize();
    QIcon icon(const QString &name, const QSize &size = QSize(24, 24),
               const QString &color = QString());
    QPixmap pixmap(const QString &name, const QSize &size = QSize(24, 24),
                   const QString &color = QString());
    void preload(const QStringList &names, const QSize &size = QSize(24, 24));
    void setSearchPaths(const QStringList &paths);
    QStringList searchPaths() const;
    void setDefaultTint(const QString &color);
    QString defaultTint() const;
    void clearCache();
    void clearIconCache(const QString &name);
    int cacheSize() const;
    qint64 cacheMemoryEstimate() const;
    quint64 totalLookups() const;
    quint64 totalCacheHits() const;
    double cacheHitRate() const;
    quint64 totalLoads() const;
    quint64 totalTintOps() const;
    void resetStatistics();
    void setMaxCacheSize(int maxCount);
    int maxCacheSize() const;
signals:
    void iconLoadFailed(const QString &name, const QString &reason);
    void cacheCleared();
    void themeColorChanged(const QString &newColor);
private:
    explicit SvgIconProvider(QObject *parent = nullptr);
    ~SvgIconProvider() override;
    SvgIconProvider(const SvgIconProvider &) = delete;
    SvgIconProvider &operator=(const SvgIconProvider &) = delete;
    QByteArray loadSvgData(const QString &name);
    QByteArray applyTint(const QByteArray &svgData, const QString &color);
    QString cacheKey(const QString &name, const QSize &size, const QString &color) const;
    void evictIfNeeded();
    struct CacheEntry {
        QIcon icon; QPixmap pixmap; QString key; int lastAccessTick = 0;
    };
    mutable QMutex m_mutex;
    QMap<QString, CacheEntry> m_cache;
    QStringList m_searchPaths;
    QString m_defaultTint;
    int m_maxCacheSize = 512;
    int m_accessTick = 0;
    mutable quint64 m_totalLookups = 0;
    mutable quint64 m_totalCacheHits = 0;
    mutable quint64 m_totalLoads = 0;
    mutable quint64 m_totalTintOps = 0;
    QString resolveIconPath(const QString &name) const;
};
#endif // SVGICONPROVIDER_H
