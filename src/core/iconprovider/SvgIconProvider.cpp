/**
 * @file SvgIconProvider.cpp
 * @brief SVG图标提供者实现 - 支持SVG着色、缓存和多尺寸渲染
 */

#include "core/iconprovider/SvgIconProvider.h"
#include <QFile>
#include <QPainter>
#include <QSvgRenderer>
#include <QtMath>

/** @brief 构造SVG图标提供者 @param parent 父对象指针 */
SvgIconProvider::SvgIconProvider(QObject *parent) : QObject(parent) {}

/** @brief 析构函数，使用默认实现 */
SvgIconProvider::~SvgIconProvider() = default;

/**
 * @brief 从文件路径注册一个SVG图标
 * @param name 图标名称，作为后续查找的键
 * @param svgPath SVG文件的文件系统路径
 */
void SvgIconProvider::registerIcon(const QString &name, const QString &svgPath) {
    QFile f(svgPath);
    if (f.open(QIODevice::ReadOnly)) { m_svgData[name] = f.readAll(); ++m_totalIconRegistrations; emit iconRegistered(name); }
}

/**
 * @brief 从原始字节数据注册一个SVG图标
 * @param name 图标名称，作为后续查找的键
 * @param data SVG文件的原始字节数据
 */
void SvgIconProvider::registerSvgData(const QString &name, const QByteArray &data) {
    m_svgData[name] = data; ++m_totalIconRegistrations; emit iconRegistered(name);
}

/**
 * @brief 获取指定名称的图标渲染为QPixmap，支持着色和缓存
 * @param name 已注册的图标名称
 * @param size 目标渲染尺寸
 * @param color 着色颜色，无效时使用默认颜色
 * @return 渲染后的QPixmap，图标不存在时返回空QPixmap
 */
QPixmap SvgIconProvider::icon(const QString &name, const QSize &size, const QColor &color) const {
    QColor c = color.isValid() ? color : m_defaultColor;
    quint64 key = (static_cast<quint64>(size.width()) << 32) | static_cast<quint64>(c.rgba());
    auto cacheIt = m_cache.constFind(name);
    if (cacheIt != m_cache.constEnd()) {
        auto pixIt = cacheIt->constFind(key);
        if (pixIt != cacheIt->constEnd()) { ++m_totalCacheHits; return pixIt.value(); }
    }
    auto svgIt = m_svgData.constFind(name);
    if (svgIt == m_svgData.constEnd()) return QPixmap();
    ++m_totalRenders;
    QByteArray svg = c.isValid() ? applyColor(svgIt.value(), c) : svgIt.value();
    QSvgRenderer renderer(svg);
    QPixmap pix(size); pix.fill(Qt::transparent);
    QPainter p(&pix); renderer.render(&p);
    const_cast<SvgIconProvider*>(this)->m_cache[name][key] = pix;
    return pix;
}

/**
 * @brief 获取指定图标的着色版本，使用SourceIn合成模式整体着色
 * @param name 已注册的图标名称
 * @param size 目标渲染尺寸
 * @param tint 着色颜色
 * @return 着色后的QPixmap，原始图标为空时返回空QPixmap
 */
QPixmap SvgIconProvider::tintedIcon(const QString &name, const QSize &size, const QColor &tint) const {
    QPixmap base = icon(name, size);
    if (base.isNull()) return base;
    QPainter p(&base);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(base.rect(), tint);
    return base;
}

/**
 * @brief 检查指定名称的图标是否已注册
 * @param name 图标名称
 * @return 已注册返回true，否则返回false
 */
bool SvgIconProvider::hasIcon(const QString &name) const { return m_svgData.contains(name); }

/**
 * @brief 获取所有已注册图标的名称列表
 * @return 图标名称字符串列表
 */
QStringList SvgIconProvider::availableIcons() const { return m_svgData.keys(); }

/**
 * @brief 设置默认着色颜色
 * @param c 默认颜色值
 */
void SvgIconProvider::setDefaultColor(const QColor &c) { m_defaultColor = c; }

/** @brief 清空所有已缓存的渲染结果 */
void SvgIconProvider::clearCache() { m_cache.clear(); ++m_totalCacheClears; }

/**
 * @brief 预加载指定名称列表的图标到缓存
 * @param names 要预加载的图标名称列表
 * @param size 预加载的目标渲染尺寸
 */
void SvgIconProvider::preloadIcons(const QStringList &names, const QSize &size) {
    for (const auto &n : names) { icon(n, size); emit iconLoaded(n); }
}

/**
 * @brief 将SVG数据中的currentColor占位符替换为指定颜色
 * @param svg 原始SVG字节数据
 * @param color 目标颜色
 * @return 替换后的SVG字节数据
 */
QByteArray SvgIconProvider::applyColor(const QByteArray &svg, const QColor &color) const {
    QString s = QString::fromUtf8(svg);
    s.replace("currentColor", color.name());
    return s.toUtf8();
}
