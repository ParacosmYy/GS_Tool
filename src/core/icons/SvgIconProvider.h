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
    /** @brief 获取SVG图标管理器单例引用 */
    static SvgIconProvider &instance();
    /** @brief 初始化图标管理器，加载搜索路径和默认着色配置 */
    void initialize();
    /** @brief 获取指定名称的SVG图标，支持着色和缓存
     *  @param name 图标名称（不含路径和扩展名）
     *  @param size 图标尺寸（默认24x24）
     *  @param color 着色颜色（空串使用默认着色）
     *  @return QIcon对象 */
    QIcon icon(const QString &name, const QSize &size = QSize(24, 24),
               const QString &color = QString());
    /** @brief 获取指定名称的SVG像素图，支持着色和缓存
     *  @param name 图标名称（不含路径和扩展名）
     *  @param size 像素图尺寸（默认24x24）
     *  @param color 着色颜色（空串使用默认着色）
     *  @return QPixmap对象 */
    QPixmap pixmap(const QString &name, const QSize &size = QSize(24, 24),
                   const QString &color = QString());
    /** @brief 批量预加载图标到缓存，避免运行时延迟
     *  @param names 图标名称列表
     *  @param size 预加载尺寸（默认24x24） */
    void preload(const QStringList &names, const QSize &size = QSize(24, 24));
    /** @brief 设置SVG图标文件搜索路径列表
     *  @param paths 搜索路径列表（按优先级排序） */
    void setSearchPaths(const QStringList &paths);
    /** @brief 获取当前SVG图标文件搜索路径列表
     *  @return 搜索路径列表 */
    QStringList searchPaths() const;
    /** @brief 设置默认图标着色颜色（用于未指定颜色的请求）
     *  @param color 默认着色颜色（十六进制如"#FFFFFF"） */
    void setDefaultTint(const QString &color);
    /** @brief 获取当前默认着色颜色
     *  @return 默认着色颜色字符串 */
    QString defaultTint() const;
    /** @brief 清空所有图标缓存 */
    void clearCache();
    /** @brief 清空指定名称的图标缓存条目
     *  @param name 图标名称 */
    void clearIconCache(const QString &name);
    /** @brief 获取当前缓存中的图标条目数量
     *  @return 缓存条目数 */
    int cacheSize() const;
    /** @brief 估算当前缓存占用的内存大小（字节）
     *  @return 估算内存字节数 */
    qint64 cacheMemoryEstimate() const;

    /**
     * @brief 图标管理器统计计数器
     *
     * 跟踪图标渲染、缓存命中/未命中、着色/尺寸变更和缓存内存占用，
     * 用于性能分析和缓存调优。resetStatistics() 将所有计数器归零。
     */
    struct Stats {
        quint64 totalIconsRendered = 0;     ///< 累计图标渲染总次数(含缓存命中)
        quint64 totalCacheHits = 0;         ///< 累计缓存命中次数
        quint64 totalCacheMisses = 0;       ///< 累计缓存未命中次数
        quint64 totalColorChanges = 0;      ///< 累计着色颜色变更次数
        quint64 totalSizeChanges = 0;       ///< 累计尺寸变更次数
        qint64  cacheSizeBytes = 0;         ///< 当前缓存占用内存估算(字节)
    };

    /** @brief 获取统计计数器只读引用 @return 当前统计快照 */
    Stats stats() const;

    /** @brief 获取累计图标查找总次数 */
    quint64 totalLookups() const;
    /** @brief 获取累计缓存命中次数 */
    quint64 totalCacheHits() const;
    /** @brief 计算缓存命中率（命中次数/查找次数）
     *  @return 命中率（0.0~1.0），无查找时返回0.0 */
    double cacheHitRate() const;
    /** @brief 获取累计SVG文件加载总次数 */
    quint64 totalLoads() const;
    /** @brief 获取累计图标着色操作总次数 */
    quint64 totalTintOps() const;
    /** @brief 重置所有统计计数器 */
    void resetStatistics();
    /** @brief 设置缓存最大条目数，超出后按LRU淘汰
     *  @param maxCount 最大缓存条目数 */
    void setMaxCacheSize(int maxCount);
    /** @brief 获取缓存最大条目数
     *  @return 最大缓存条目数 */
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
    mutable Stats m_stats;                  ///< 扩展统计计数器(渲染/缓存未命中/着色/尺寸)
    QString resolveIconPath(const QString &name) const;
};
#endif // SVGICONPROVIDER_H
