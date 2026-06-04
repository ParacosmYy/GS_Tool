/**
 * @file SvgIconProvider.cpp
 * @brief SVG图标管理器实现 — 着色、缓存、LRU淘汰
 * @since score-131
 */
#include "core/icons/SvgIconProvider.h"
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QRegularExpression>
#include <QSvgRenderer>

/** @brief 构造函数，初始化默认搜索路径和默认着色颜色 @param parent 父对象指针 */
SvgIconProvider::SvgIconProvider(QObject *parent) : QObject(parent) {
    m_searchPaths = QStringList{
        QStringLiteral(":/icons/svg"), QStringLiteral(":/icons"),
        QApplication::applicationDirPath() + QStringLiteral("/icons/svg"),
        QApplication::applicationDirPath() + QStringLiteral("/icons")};
    m_defaultTint = QStringLiteral("#E0E0E0");
}

/** @brief 析构函数，清空图标缓存 */
SvgIconProvider::~SvgIconProvider() { clearCache(); }

/** @brief 获取单例实例 @return SvgIconProvider全局唯一实例引用 */
SvgIconProvider &SvgIconProvider::instance() { static SvgIconProvider inst; return inst; }

/** @brief 初始化接口(当前为空实现，预留扩展) */
void SvgIconProvider::initialize() {}

/** @brief 获取指定名称和尺寸的图标，支持着色和LRU缓存 @param name 图标名称(不含扩展名) @param size 目标尺寸 @param color 着色颜色，为空使用默认色 @return 着色后的QIcon对象 */
QIcon SvgIconProvider::icon(const QString &name, const QSize &size, const QString &color) {
    QMutexLocker locker(&m_mutex);
    ++m_totalLookups;
    ++m_stats.totalIconsRendered;
    const QString tint = color.isEmpty() ? m_defaultTint : color;
    if (!color.isEmpty() && color != m_defaultTint) ++m_stats.totalColorChanges;
    const QString key = cacheKey(name, size, tint);
    auto it = m_cache.find(key);
    if (it != m_cache.end()) {
        ++m_totalCacheHits;
        ++m_stats.totalCacheHits;
        it->lastAccessTick = ++m_accessTick;
        return it->icon;
    }
    ++m_stats.totalCacheMisses;
    locker.unlock();
    QByteArray svgData = loadSvgData(name);
    locker.relock();
    if (svgData.isEmpty()) { emit iconLoadFailed(name, tr("SVG文件未找到")); return QIcon(); }
    QByteArray tintedData = applyTint(svgData, tint);
    ++m_totalTintOps;
    QSvgRenderer renderer(tintedData);
    if (!renderer.isValid()) { emit iconLoadFailed(name, tr("SVG渲染失败")); return QIcon(); }
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
    m_stats.cacheSizeBytes = cacheMemoryEstimate();
    return entry.icon;
}

/** @brief 获取指定图标的QPixmap形式 @param name 图标名称 @param size 目标尺寸 @param color 着色颜色 @return 着色后的QPixmap对象 */
QPixmap SvgIconProvider::pixmap(const QString &name, const QSize &size, const QString &color) {
    icon(name, size, color);
    QMutexLocker locker(&m_mutex);
    const QString tint = color.isEmpty() ? m_defaultTint : color;
    const QString key = cacheKey(name, size, tint);
    auto it = m_cache.find(key);
    return it != m_cache.end() ? it->pixmap : QPixmap();
}

/** @brief 批量预加载图标到缓存 @param names 图标名称列表 @param size 目标尺寸 */
void SvgIconProvider::preload(const QStringList &names, const QSize &size) {
    for (const auto &n : names) icon(n, size);
}

/** @brief 设置图标搜索路径列表，同时清空缓存 @param paths 搜索路径列表 */
void SvgIconProvider::setSearchPaths(const QStringList &paths) {
    QMutexLocker locker(&m_mutex); m_searchPaths = paths; clearCache();
}

/** @brief 获取当前图标搜索路径列表 @return 搜索路径字符串列表 */
QStringList SvgIconProvider::searchPaths() const {
    QMutexLocker locker(&m_mutex); return m_searchPaths;
}

/** @brief 设置默认着色颜色，变更时清空缓存并发射主题颜色变更信号 @param color 新的默认着色颜色(十六进制) */
void SvgIconProvider::setDefaultTint(const QString &color) {
    QMutexLocker locker(&m_mutex);
    if (m_defaultTint != color) { m_defaultTint = color; locker.unlock(); clearCache(); emit themeColorChanged(color); }
}

/** @brief 获取当前默认着色颜色 @return 默认着色颜色字符串 */
QString SvgIconProvider::defaultTint() const {
    QMutexLocker locker(&m_mutex); return m_defaultTint;
}

/** @brief 清空全部图标缓存并重置访问计数 */
void SvgIconProvider::clearCache() {
    QMutexLocker locker(&m_mutex); m_cache.clear(); m_accessTick = 0;
    locker.unlock(); emit cacheCleared();
}

/** @brief 清空指定图标名称关联的所有缓存条目 @param name 图标名称 */
void SvgIconProvider::clearIconCache(const QString &name) {
    QMutexLocker locker(&m_mutex);
    auto it = m_cache.begin();
    while (it != m_cache.end()) {
        if (it.key().startsWith(name + QLatin1Char('|'))) it = m_cache.erase(it);
        else ++it;
    }
}

/** @brief 获取当前缓存条目数量 @return 缓存中的图标数量 */
int SvgIconProvider::cacheSize() const { QMutexLocker locker(&m_mutex); return m_cache.size(); }

/** @brief 估算缓存占用内存大小(像素数据) @return 估算字节数 */
qint64 SvgIconProvider::cacheMemoryEstimate() const {
    QMutexLocker locker(&m_mutex); qint64 total = 0;
    for (auto it = m_cache.cbegin(); it != m_cache.cend(); ++it)
        total += static_cast<qint64>(it->pixmap.width()) * it->pixmap.height() * it->pixmap.depth() / 8;
    return total;
}

/** @brief 设置最大缓存条目数，超限时立即触发LRU淘汰 @param maxCount 最大缓存数量 */
void SvgIconProvider::setMaxCacheSize(int maxCount) {
    QMutexLocker locker(&m_mutex); m_maxCacheSize = qMax(1, maxCount); evictIfNeeded();
}

/** @brief 获取最大缓存条目数 @return 最大缓存数量 */
int SvgIconProvider::maxCacheSize() const { QMutexLocker locker(&m_mutex); return m_maxCacheSize; }

/** @brief 获取总查找次数 @return 累计查找计数 */
quint64 SvgIconProvider::totalLookups() const { QMutexLocker locker(&m_mutex); return m_totalLookups; }

/** @brief 获取缓存命中次数 @return 累计命中计数 */
quint64 SvgIconProvider::totalCacheHits() const { QMutexLocker locker(&m_mutex); return m_totalCacheHits; }

/** @brief 计算缓存命中率 @return 命中率(0.0~1.0)，无查找时返回0.0 */
double SvgIconProvider::cacheHitRate() const {
    QMutexLocker locker(&m_mutex);
    return m_totalLookups == 0 ? 0.0 : static_cast<double>(m_totalCacheHits) / static_cast<double>(m_totalLookups);
}

/** @brief 获取总图标加载次数 @return 累计从文件加载次数 */
quint64 SvgIconProvider::totalLoads() const { QMutexLocker locker(&m_mutex); return m_totalLoads; }

/** @brief 获取总着色操作次数 @return 累计SVG着色次数 */
quint64 SvgIconProvider::totalTintOps() const { QMutexLocker locker(&m_mutex); return m_totalTintOps; }

/** @brief 获取扩展统计计数器快照 @return 当前统计值副本 */
SvgIconProvider::Stats SvgIconProvider::stats() const {
    QMutexLocker locker(&m_mutex);
    Stats snapshot = m_stats;
    snapshot.cacheSizeBytes = cacheMemoryEstimate();
    return snapshot;
}

/** @brief 重置所有统计计数器 */
void SvgIconProvider::resetStatistics() {
    QMutexLocker locker(&m_mutex);
    m_totalLookups = 0; m_totalCacheHits = 0; m_totalLoads = 0; m_totalTintOps = 0;
    m_stats = Stats{};
}

// 私有实现方法(loadSvgData/applyTint/cacheKey/evictIfNeeded/resolveIconPath)
// 见 SvgIconProviderPrivate.cpp
