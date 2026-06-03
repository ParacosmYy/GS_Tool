/**
 * @file IconManager.h
 * @brief 图标管理器 — SVG图标加载、缓存和着色
 *
 * 单例模式，提供统一的图标访问接口:
 *   - 从 resources/icons/lucide/ 加载 SVG 图标
 *   - 根据 ThemeManager 语义色自动着色
 *   - 内置缓存避免重复加载和渲染
 *   - 主题切换时自动清空缓存重新着色
 *
 * 设计模式: 单例(Singleton) + 缓存(Cache)
 * 协作: ThemeManager(着色) / BasePanel(标题图标) / EmptyStateWidget(空状态图标)
 */
#ifndef ICON_MANAGER_H
#define ICON_MANAGER_H

#include <QObject>
#include <QIcon>
#include <QPixmap>
#include <QColor>
#include <QMap>
#include <QString>

/**
 * @brief 图标管理器 — SVG图标统一加载、缓存和着色
 *
 * 使用 Lucide Icons (MIT许可, 线条风格, 1.5px描边)
 * 所有SVG使用 currentColor 填充，运行时替换为 ThemeManager 语义色。
 *
 * 使用方式:
 *   QIcon ico = IconManager::instance().icon("cable");
 *   QPixmap pm = IconManager::instance().pixmap("bluetooth", 16);
 */
class IconManager : public QObject {
    Q_OBJECT

public:
    /** @brief 获取单例实例 */
    static IconManager& instance();

    /** @brief 获取图标(QIcon)，自动缓存+着色 */
    QIcon icon(const QString& name) const;

    /**
     * @brief 获取图标像素图(QPixmap)
     * @param name 图标名称(如 "cable", "bluetooth")
     * @param size 尺寸(px)，默认16
     * @param colorKey 着色使用的语义色键，默认"TextSecondary"
     */
    QPixmap pixmap(const QString& name, int size = 16,
                    const QString& colorKey = "TextSecondary") const;

    /** @brief 清空缓存(主题切换时调用，重新着色) */
    void clearCache();

    /** @brief 获取图标是否存在于资源中 */
    bool hasIcon(const QString& name) const;

    // ---- 统计计数器 ----

    /** @brief 获取图标加载总次数（含缓存未命中） @return 加载操作总次数 */
    quint64 totalLoads() const;

    /** @brief 获取缓存命中总次数 @return 命中次数 */
    quint64 totalCacheHits() const;

    /** @brief 获取缓存未命中总次数 @return 未命中次数 */
    quint64 totalCacheMisses() const;

    /** @brief 获取当前缓存条目数量 @return 缓存大小 */
    quint64 cacheSize() const;

    /** @brief 重置所有图标管理统计计数器为零 */
    void resetIconStatistics();

private:
    explicit IconManager(QObject* parent = nullptr);
    ~IconManager() override;
    IconManager(const IconManager&) = delete;
    IconManager& operator=(const IconManager&) = delete;

    /** @brief 加载SVG文件并着色为指定颜色 */
    QByteArray loadAndColorSvg(const QString& name, const QColor& color) const;

    /** @brief 获取SVG资源路径 */
    QString svgPath(const QString& name) const;

    /** @brief 生成缓存键 (name + size + color) */
    QString cacheKey(const QString& name, int size, const QString& colorKey) const;

    mutable QMap<QString, QIcon> m_iconCache;      ///< QIcon缓存(按name)
    mutable QMap<QString, QPixmap> m_pixmapCache;   ///< QPixmap缓存(按name+size+color)

    // ---- 统计计数器 ----
    mutable quint64 m_totalLoads = 0;       ///< 图标加载总次数(含缓存未命中时的实际加载)
    mutable quint64 m_totalCacheHits = 0;   ///< 缓存命中总次数
    mutable quint64 m_totalCacheMisses = 0; ///< 缓存未命中总次数
};

#endif // ICON_MANAGER_H
