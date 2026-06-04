/**
 * @file SvgIconProviderPrivate.cpp
 * @brief SVG图标管理器 — 私有实现方法(加载/着色/缓存键/LRU淘汰/路径解析)
 *
 * 从 SvgIconProvider.cpp 拆分而来，包含:
 *   - loadSvgData(): 从搜索路径加载SVG文件原始数据
 *   - applyTint(): SVG数据着色(fill/stroke替换)
 *   - cacheKey(): 缓存键生成(名称|尺寸|颜色)
 *   - evictIfNeeded(): LRU淘汰策略
 *   - resolveIconPath(): 搜索路径中的图标路径解析
 *
 * 公共接口(icon/pixmap/preload/cache操作/统计)见 SvgIconProvider.cpp。
 */

#include "core/icons/SvgIconProvider.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QRegularExpression>
#include <QSvgRenderer>

/** @brief 从搜索路径加载SVG文件原始数据 @param name 图标名称 @return SVG文件字节数据，未找到返回空QByteArray */
QByteArray SvgIconProvider::loadSvgData(const QString &name) {
    QMutexLocker locker(&m_mutex); QString resolvedPath = resolveIconPath(name); locker.unlock();
    if (resolvedPath.isEmpty()) return QByteArray();
    QFile file(resolvedPath);
    if (!file.open(QIODevice::ReadOnly)) return QByteArray();
    return file.readAll();
}

/** @brief 对SVG数据应用着色，替换fill和stroke属性 @param svgData 原始SVG字节数据 @param color 目标颜色(十六进制) @return 着色后的SVG字节数据 */
QByteArray SvgIconProvider::applyTint(const QByteArray &svgData, const QString &color) {
    QString svgString = QString::fromUtf8(svgData);
    static const QRegularExpression fillRe(QStringLiteral("fill\\s*=\\s*\"(?!none|transparent)([^\"]*)\""), QRegularExpression::CaseInsensitiveOption);
    svgString.replace(fillRe, QStringLiteral("fill=\"%1\"").arg(color));
    static const QRegularExpression strokeRe(QStringLiteral("stroke\\s*=\\s*\"(?!none)([^\"]*)\""), QRegularExpression::CaseInsensitiveOption);
    svgString.replace(strokeRe, QStringLiteral("stroke=\"%1\"").arg(color));
    return svgString.toUtf8();
}

/** @brief 生成缓存键，格式为"名称|宽x高|颜色" @param name 图标名称 @param size 图标尺寸 @param color 着色颜色 @return 缓存键字符串 */
QString SvgIconProvider::cacheKey(const QString &name, const QSize &size, const QString &color) const {
    return QStringLiteral("%1|%2x%3|%4").arg(name).arg(size.width()).arg(size.height()).arg(color);
}

/** @brief LRU淘汰策略，当缓存数量超过上限时移除最久未访问的条目 */
void SvgIconProvider::evictIfNeeded() {
    while (m_cache.size() > m_maxCacheSize && !m_cache.isEmpty()) {
        auto oldest = m_cache.begin();
        for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
            if (it->lastAccessTick < oldest->lastAccessTick) oldest = it;
        }
        m_cache.erase(oldest);
    }
}

/** @brief 在搜索路径中解析图标文件的实际路径 @param name 图标名称(不含扩展名) @return 完整文件路径，未找到返回空字符串 */
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
