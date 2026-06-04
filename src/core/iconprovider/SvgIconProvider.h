/**
 * @file SvgIconProvider.h
 * @brief SVG图标提供者，支持SVG着色渲染和缓存管理
 */
#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QPixmap>
#include <QSize>
#include <QColor>

/**
 * @class SvgIconProvider
 * @brief SVG图标提供者，支持从文件或原始数据加载SVG，运行时着色并按尺寸+颜色缓存
 */
class SvgIconProvider : public QObject {
    Q_OBJECT
public:
    /** @brief 构造函数 @param parent 父对象指针 */
    explicit SvgIconProvider(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~SvgIconProvider() override;

    /** @brief 从文件注册SVG图标 @param name 图标名称 @param svgPath SVG文件路径 */
    void registerIcon(const QString &name, const QString &svgPath);
    /** @brief 从原始数据注册SVG图标 @param name 图标名称 @param svgData SVG二进制数据 */
    void registerSvgData(const QString &name, const QByteArray &svgData);
    /** @brief 获取着色后的图标像素图 @param name 图标名称 @param size 目标尺寸 @param color 着色颜色，无效颜色使用默认色 @return 渲染后的QPixmap */
    QPixmap icon(const QString &name, const QSize &size, const QColor &color = QColor()) const;
    /** @brief 获取强制着色的图标像素图 @param name 图标名称 @param size 目标尺寸 @param tintColor 着色颜色 @return 着色后的QPixmap */
    QPixmap tintedIcon(const QString &name, const QSize &size, const QColor &tintColor) const;
    /** @brief 查询图标是否已注册 @param name 图标名称 @return 是否存在 */
    bool hasIcon(const QString &name) const;
    /** @brief 获取所有已注册图标名称 @return 图标名称列表 */
    QStringList availableIcons() const;
    /** @brief 设置默认着色颜色 @param color 默认颜色 */
    void setDefaultColor(const QColor &color);
    /** @brief 清除所有缓存 */
    void clearCache();
    /** @brief 批量预加载图标到缓存 @param names 图标名称列表 @param size 目标尺寸 */
    void preloadIcons(const QStringList &names, const QSize &size);

    // ---- 统计计数器 ----
    /** @brief 获取累计图标注册次数 @return 注册次数 */
    quint64 totalIconRegistrations() const { return m_totalIconRegistrations; }
    /** @brief 获取累计图标渲染次数 @return 渲染次数 */
    quint64 totalRenders() const { return m_totalRenders; }
    /** @brief 获取累计缓存命中次数 @return 命中次数 */
    quint64 totalCacheHits() const { return m_totalCacheHits; }
    /** @brief 获取累计缓存清除次数 @return 清除次数 */
    quint64 totalCacheClears() const { return m_totalCacheClears; }
    /** @brief 重置所有统计计数器 */
    void resetIconProviderStatistics() { m_totalIconRegistrations = 0; m_totalRenders = 0; m_totalCacheHits = 0; m_totalCacheClears = 0; }

signals:
    /** @brief 新图标注册完成时发射 @param name 图标名称 */
    void iconRegistered(const QString &name);
    /** @brief 图标首次加载完成时发射 @param name 图标名称 */
    void iconLoaded(const QString &name);

private:
    /** @brief 对SVG数据应用着色替换 @param svg 原始SVG数据 @param color 目标颜色 @return 着色后的SVG数据 */
    QByteArray applyColor(const QByteArray &svg, const QColor &color) const;

    QMap<QString, QByteArray> m_svgData;                    ///< 图标名称到SVG原始数据的映射
    QMap<QString, QMap<quint64, QPixmap>> m_cache;          ///< 二级缓存：名称->(缓存键->像素图)
    QColor m_defaultColor;                                   ///< 默认着色颜色

    // ---- 统计 ----
    quint64 m_totalIconRegistrations = 0;       ///< 统计: 累计图标注册次数
    mutable quint64 m_totalRenders = 0;         ///< 统计: 累计SVG渲染次数
    mutable quint64 m_totalCacheHits = 0;       ///< 统计: 累计缓存命中次数
    quint64 m_totalCacheClears = 0;             ///< 统计: 累计缓存清除次数
};
