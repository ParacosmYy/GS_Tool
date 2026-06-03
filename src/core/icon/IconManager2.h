/**
 * @file IconManager2.h
 * @brief 图标管理器，支持亮/暗主题图标切换和缓存
 */
#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QPixmap>
#include <QSize>
#include <QIcon>

/**
 * @class IconManager
 * @brief 图标管理器，支持注册亮/暗双主题图标路径、按主题切换和按需缓存
 */
class IconManager : public QObject {
    Q_OBJECT
public:
    /** @brief 构造函数 @param parent 父对象指针 */
    explicit IconManager(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~IconManager() override;

    /** @brief 注册单路径图标 @param name 图标名称 @param path 图标文件路径 */
    void registerIcon(const QString &name, const QString &path);
    /** @brief 注册亮/暗双主题图标 @param name 图标名称 @param lightPath 亮色主题路径 @param darkPath 暗色主题路径 */
    void registerThemedIcon(const QString &name, const QString &lightPath, const QString &darkPath);
    /** @brief 获取图标对象 @param name 图标名称 @return QIcon对象 */
    QIcon icon(const QString &name) const;
    /** @brief 获取指定尺寸的像素图 @param name 图标名称 @param size 目标尺寸 @return QPixmap对象 */
    QPixmap pixmap(const QString &name, const QSize &size) const;
    /** @brief 查询图标是否已注册 @param name 图标名称 @return 是否存在 */
    bool hasIcon(const QString &name) const;
    /** @brief 获取所有已注册图标名称 @return 图标名称列表 */
    QStringList availableIcons() const;
    /** @brief 切换亮/暗主题 @param dark 是否为暗色主题 */
    void setTheme(bool dark);
    /** @brief 预加载所有图标到指定尺寸缓存 @param size 目标尺寸 */
    void preloadAll(const QSize &size);
    /** @brief 清除所有图标缓存 */
    void clearCache();

signals:
    /** @brief 新图标注册时发射 @param name 图标名称 */
    void iconRegistered(const QString &name);
    /** @brief 主题切换时发射 @param dark 是否为暗色主题 */
    void themeChanged(bool dark);

private:
    QMap<QString, QString> m_lightPaths;  ///< 亮色主题图标路径映射
    QMap<QString, QString> m_darkPaths;   ///< 暗色主题图标路径映射
    QMap<QString, QIcon> m_cache;         ///< 图标缓存
    bool m_darkTheme = false;             ///< 当前是否为暗色主题
};
