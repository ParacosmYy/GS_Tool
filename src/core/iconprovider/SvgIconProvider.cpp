#include "core/iconprovider/SvgIconProvider.h"
#include <QFile>
#include <QPainter>
#include <QSvgRenderer>
#include <QtMath>

SvgIconProvider::SvgIconProvider(QObject *parent) : QObject(parent) {}
SvgIconProvider::~SvgIconProvider() = default;

void SvgIconProvider::registerIcon(const QString &name, const QString &svgPath) {
    QFile f(svgPath);
    if (f.open(QIODevice::ReadOnly)) { m_svgData[name] = f.readAll(); emit iconRegistered(name); }
}

void SvgIconProvider::registerSvgData(const QString &name, const QByteArray &data) {
    m_svgData[name] = data; emit iconRegistered(name);
}

QPixmap SvgIconProvider::icon(const QString &name, const QSize &size, const QColor &color) const {
    QColor c = color.isValid() ? color : m_defaultColor;
    quint64 key = (static_cast<quint64>(size.width()) << 32) | static_cast<quint64>(c.rgba());
    auto cacheIt = m_cache.constFind(name);
    if (cacheIt != m_cache.constEnd()) {
        auto pixIt = cacheIt->constFind(key);
        if (pixIt != cacheIt->constEnd()) return pixIt.value();
    }
    auto svgIt = m_svgData.constFind(name);
    if (svgIt == m_svgData.constEnd()) return QPixmap();
    QByteArray svg = c.isValid() ? applyColor(svgIt.value(), c) : svgIt.value();
    QSvgRenderer renderer(svg);
    QPixmap pix(size); pix.fill(Qt::transparent);
    QPainter p(&pix); renderer.render(&p);
    const_cast<SvgIconProvider*>(this)->m_cache[name][key] = pix;
    return pix;
}

QPixmap SvgIconProvider::tintedIcon(const QString &name, const QSize &size, const QColor &tint) const {
    QPixmap base = icon(name, size);
    if (base.isNull()) return base;
    QPainter p(&base);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(base.rect(), tint);
    return base;
}

bool SvgIconProvider::hasIcon(const QString &name) const { return m_svgData.contains(name); }
QStringList SvgIconProvider::availableIcons() const { return m_svgData.keys(); }
void SvgIconProvider::setDefaultColor(const QColor &c) { m_defaultColor = c; }
void SvgIconProvider::clearCache() { m_cache.clear(); }

void SvgIconProvider::preloadIcons(const QStringList &names, const QSize &size) {
    for (const auto &n : names) { icon(n, size); emit iconLoaded(n); }
}

QByteArray SvgIconProvider::applyColor(const QByteArray &svg, const QColor &color) const {
    QString s = QString::fromUtf8(svg);
    s.replace("currentColor", color.name());
    return s.toUtf8();
}
