/**
 * @file IconManager.cpp
 * @brief 图标管理器实现 — SVG加载、currentColor着色、LRU缓存
 */

#include "core/theme/IconManager.h"
#include "core/theme/ThemeManager.h"

#include <QFile>
#include <QPainter>
#include <QSvgRenderer>
#include <QApplication>

IconManager& IconManager::instance()
{
    static IconManager inst(qApp);
    return inst;
}

/** @brief 构造函数 — 注册主题切换监听，自动清空缓存 */
IconManager::IconManager(QObject* parent)
    : QObject(parent)
{
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &IconManager::clearCache);
}

IconManager::~IconManager() = default;

/** @brief 拼接SVG资源路径 */
QString IconManager::svgPath(const QString& name) const
{
    return QStringLiteral(":/icons/lucide/%1.svg").arg(name);
}

/** @brief 检查SVG文件是否存在于Qt资源中 */
bool IconManager::hasIcon(const QString& name) const
{
    return QFile::exists(svgPath(name));
}

/** @brief 生成唯一缓存键: name_size_colorKey */
QString IconManager::cacheKey(const QString& name, int size,
                               const QString& colorKey) const
{
    return QStringLiteral("%1_%2_%3").arg(name).arg(size).arg(colorKey);
}

/** @brief 加载SVG文件并将currentColor替换为实际颜色 @param name 图标名称(不含扩展名) @param color 目标颜色 @return 着色后的SVG字节数据，失败返回空 */
QByteArray IconManager::loadAndColorSvg(const QString& name,
                                         const QColor& color) const
{
    QFile file(svgPath(name));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QString svg = QString::fromUtf8(file.readAll());
    file.close();

    /* Lucide SVG 统一使用 stroke="currentColor"，直接替换即可 */
    svg.replace(QStringLiteral("currentColor"), color.name(QColor::HexRgb));
    return svg.toUtf8();
}

/** @brief 获取QIcon(多尺寸)，首次调用时加载+着色+缓存 @param name 图标名称 @return QIcon实例，图标不存在时返回空QIcon */
QIcon IconManager::icon(const QString& name) const
{
    if (m_iconCache.contains(name)) {
        ++m_totalCacheHits;   ///< 统计: QIcon缓存命中递增
        return m_iconCache.value(name);
    }

    ++m_totalCacheMisses;     ///< 统计: QIcon缓存未命中递增
    ++m_totalLoads;           ///< 统计: 图标加载递增

    QColor clr = ThemeManager::instance().color(
        ThemeManager::SemanticColor::TextSecondary);
    QByteArray svgData = loadAndColorSvg(name, clr);

    if (svgData.isEmpty()) {
        m_iconCache[name] = QIcon();
        return {};
    }

    QIcon ico;
    QSvgRenderer renderer(svgData);
    for (int sz : {16, 20, 24, 48}) {
        QPixmap pm(sz, sz);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        renderer.render(&p);
        ico.addPixmap(pm);
    }

    m_iconCache[name] = ico;
    return ico;
}

/**
 * @brief 获取指定尺寸和颜色的QPixmap
 * @param name 图标名称
 * @param size 目标尺寸(px)
 * @param colorKey 语义色键名(预留扩展)
 * @return QPixmap实例，图标不存在时返回空QPixmap
 */
QPixmap IconManager::pixmap(const QString& name, int size,
                             const QString& colorKey) const
{
    QString key = cacheKey(name, size, colorKey);
    if (m_pixmapCache.contains(key)) {
        ++m_totalCacheHits;   ///< 统计: QPixmap缓存命中递增
        return m_pixmapCache.value(key);
    }

    ++m_totalCacheMisses;     ///< 统计: QPixmap缓存未命中递增
    ++m_totalLoads;           ///< 统计: 图标加载递增

    /* 当前仅支持 TextSecondary，后续可按 colorKey 映射更多语义色 */
    QColor clr = ThemeManager::instance().color(
        ThemeManager::SemanticColor::TextSecondary);

    QByteArray svgData = loadAndColorSvg(name, clr);
    if (svgData.isEmpty()) {
        m_pixmapCache[key] = QPixmap();
        return {};
    }

    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QSvgRenderer renderer(svgData);
    QPainter p(&pm);
    renderer.render(&p);

    m_pixmapCache[key] = pm;
    return pm;
}

/** @brief 清空全部缓存，主题切换时由信号触发 */
void IconManager::clearCache()
{
    m_iconCache.clear();
    m_pixmapCache.clear();
}

// ---- 统计计数器实现 ----

/** @brief 获取图标加载总次数 @return 加载操作总次数 */
quint64 IconManager::totalLoads() const
{
    return m_totalLoads;
}

/** @brief 获取缓存命中总次数 @return 命中次数 */
quint64 IconManager::totalCacheHits() const
{
    return m_totalCacheHits;
}

/** @brief 获取缓存未命中总次数 @return 未命中次数 */
quint64 IconManager::totalCacheMisses() const
{
    return m_totalCacheMisses;
}

/** @brief 获取当前缓存条目数量 @return 缓存大小 */
quint64 IconManager::cacheSize() const
{
    return static_cast<quint64>(m_iconCache.size() + m_pixmapCache.size());
}

/** @brief 重置所有图标管理统计计数器为零 */
void IconManager::resetIconStatistics()
{
    m_totalLoads = 0;
    m_totalCacheHits = 0;
    m_totalCacheMisses = 0;
}
