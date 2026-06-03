/**
 * @file SvgIconProvider.cpp
 * @brief SVG图标管理器实现
 * @since score-131
 */
#include "core/icons/SvgIconProvider.h"
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QRegularExpression>
#include <QSvgRenderer>

SvgIconProvider::SvgIconProvider(QObject *parent) : QObject(parent) {
    m_searchPaths = QStringList{
        QStringLiteral(":/icons/svg"), QStringLiteral(":/icons"),
        QApplication::applicationDirPath() + QStringLiteral("/icons/svg"),
        QApplication::applicationDirPath() + QStringLiteral("/icons")};
    m_defaultTint = QStringLiteral("#E0E0E0");
}
SvgIconProvider::~SvgIconProvider() { clearCache(); }
SvgIconProvider &SvgIconProvider::instance() { static SvgIconProvider inst; return inst; }
void SvgIconProvider::initialize() {}

QIcon SvgIconProvider::icon(const QString &name, const QSize &size, const QString &color) {
    QMutexLocker locker(&m_mutex);
    ++m_totalLookups;
    const QString tint = color.isEmpty() ? m_defaultTint : color;
    const QString key = cacheKey(name, size, tint);
    auto it = m_cache.find(key);
    if (it != m_cache.end()) {
        ++m_totalCacheHits;
        it->lastAccessTick = ++m_accessTick;
        return it->icon;
    }
    locker.unlock();
    QByteArray svgData = loadSvgData(name);
    locker.relock();
    if (svgData.isEmpty()) { emit iconLoadFailed(name, tr("SVG file not found")); return QIcon(); }
    QByteArray tintedData = applyTint(svgData, tint);
    ++m_totalTintOps;
    QSvgRenderer renderer(tintedData);
    if (!renderer.isValid()) { emit iconLoadFailed(name, tr("SVG render failed")); return QIcon(); }
    QPixmap px(size * qApp->devicePixelRatio());
    px.fill(Qt::transparent);
    QPainter painter(&px);
    renderer.render(&painter);
    painter.end();
    px.setDevicePixelRatio(qApp->devicePixelRatio());
    evictIfNeeded();
    CacheEntry entry;
    entry.icon = QIcon(px); entry.pixmap = px; entry.key = key;
    entry.lastAccessTick = ++m_accessTick;
    m_cache[key] = entry;
    ++m_totalLoads;
    return entry.icon;
}

QPixmap SvgIconProvider::pixmap(const QString &name, const QSize &size, const QString &color) {
    icon(name, size, color);
    QMutexLocker locker(&m_mutex);
    const QString tint = color.isEmpty() ? m_defaultTint : color;
    const QString key = cacheKey(name, size, tint);
    auto it = m_cache.find(key);
    return it != m_cache.end() ? it->pixmap : QPixmap();
}

void SvgIconProvider::preload(const QStringList &names, const QSize &size) {
    for (const auto &n : names) icon(n, size);
}

void SvgIconProvider::setSearchPaths(const QStringList &paths) {
    QMutexLocker locker(&m_mutex); m_searchPaths = paths; clearCache();
}
QStringList SvgIconProvider::searchPaths() const {
    QMutexLocker locker(&m_mutex); return m_searchPaths;
}
void SvgIconProvider::setDefaultTint(const QString &color) {
    QMutexLocker locker(&m_mutex);
    if (m_defaultTint != color) { m_defaultTint = color; locker.unlock(); clearCache(); emit themeColorChanged(color); }
}
QString SvgIconProvider::defaultTint() const {
    QMutexLocker locker(&m_mutex); return m_defaultTint;
}

void SvgIconProvider::clearCache() {
    QMutexLocker locker(&m_mutex); m_cache.clear(); m_accessTick = 0;
    locker.unlock(); emit cacheCleared();
}
void SvgIconProvider::clearIconCache(const QString &name) {
    QMutexLocker locker(&m_mutex);
    auto it = m_cache.begin();
    while (it != m_cache.end()) {
        if (it.key().startsWith(name + QLatin1Char('|'))) it = m_cache.erase(it);
        else ++it;
    }
}
int SvgIconProvider::cacheSize() const { QMutexLocker locker(&m_mutex); return m_cache.size(); }
qint64 SvgIconProvider::cacheMemoryEstimate() const {
    QMutexLocker locker(&m_mutex); qint64 total = 0;
    for (auto it = m_cache.cbegin(); it != m_cache.cend(); ++it)
        total += static_cast<qint64>(it->pixmap.width()) * it->pixmap.height() * it->pixmap.depth() / 8;
    return total;
}
void SvgIconProvider::setMaxCacheSize(int maxCount) {
    QMutexLocker locker(&m_mutex); m_maxCacheSize = qMax(1, maxCount); evictIfNeeded();
}
int SvgIconProvider::maxCacheSize() const { QMutexLocker locker(&m_mutex); return m_maxCacheSize; }

quint64 SvgIconProvider::totalLookups() const { QMutexLocker locker(&m_mutex); return m_totalLookups; }
quint64 SvgIconProvider::totalCacheHits() const { QMutexLocker locker(&m_mutex); return m_totalCacheHits; }
double SvgIconProvider::cacheHitRate() const {
    QMutexLocker locker(&m_mutex);
    return m_totalLookups == 0 ? 0.0 : static_cast<double>(m_totalCacheHits) / static_cast<double>(m_totalLookups);
}
quint64 SvgIconProvider::totalLoads() const { QMutexLocker locker(&m_mutex); return m_totalLoads; }
quint64 SvgIconProvider::totalTintOps() const { QMutexLocker locker(&m_mutex); return m_totalTintOps; }
void SvgIconProvider::resetStatistics() {
    QMutexLocker locker(&m_mutex); m_totalLookups = 0; m_totalCacheHits = 0; m_totalLoads = 0; m_totalTintOps = 0;
}

QByteArray SvgIconProvider::loadSvgData(const QString &name) {
    QMutexLocker locker(&m_mutex); QString resolvedPath = resolveIconPath(name); locker.unlock();
    if (resolvedPath.isEmpty()) return QByteArray();
    QFile file(resolvedPath);
    if (!file.open(QIODevice::ReadOnly)) return QByteArray();
    return file.readAll();
}

QByteArray SvgIconProvider::applyTint(const QByteArray &svgData, const QString &color) {
    QString svgString = QString::fromUtf8(svgData);
    static const QRegularExpression fillRe(QStringLiteral("fill\\s*=\\s*\"(?!none|transparent)([^\"]*)\""), QRegularExpression::CaseInsensitiveOption);
    svgString.replace(fillRe, QStringLiteral("fill=\"%1\"").arg(color));
    static const QRegularExpression strokeRe(QStringLiteral("stroke\\s*=\\s*\"(?!none)([^\"]*)\""), QRegularExpression::CaseInsensitiveOption);
    svgString.replace(strokeRe, QStringLiteral("stroke=\"%1\"").arg(color));
    return svgString.toUtf8();
}

QString SvgIconProvider::cacheKey(const QString &name, const QSize &size, const QString &color) const {
    return QStringLiteral("%1|%2x%3|%4").arg(name).arg(size.width()).arg(size.height()).arg(color);
}

void SvgIconProvider::evictIfNeeded() {
    while (m_cache.size() > m_maxCacheSize && !m_cache.isEmpty()) {
        auto oldest = m_cache.begin();
        for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
            if (it->lastAccessTick < oldest->lastAccessTick) oldest = it;
        }
        m_cache.erase(oldest);
    }
}

QString SvgIconProvider::resolveIconPath(const QString &name) const {
    const QStringList exts = { QStringLiteral(".svg"), QStringLiteral(".SVG") };
    for (const auto &prefix : m_searchPaths) {
        for (const auto &ext : exts) {
            QString path = prefix + QLatin1Char('/') + name + ext;
            if (path.startsWith(QLatin1Char(':'))) { if (QFile::exists(path)) return path; }
            else { QFileInfo fi(path); if (fi.exists() && fi.isFile()) return path; }
        }
    }
    if (QFile::exists(name)) return name;
    return QString();
}
