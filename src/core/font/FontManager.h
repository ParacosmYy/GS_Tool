/**
 * @file FontManager.h
 * @brief 字体管理器，统一管理应用字体的加载、预设和切换
 */
#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QFont>
#include <QFontDatabase>

/**
 * @class FontManager
 * @brief 字体管理器，支持从文件加载字体、预设字体方案(标题/正文/代码等)和运行时切换
 */
class FontManager : public QObject {
    Q_OBJECT
public:
    /** @brief 预设字体角色枚举 */
    enum PresetFont { Title, Heading, Body, Caption, Code, Monospace };
    Q_ENUM(PresetFont)

    /** @brief 构造函数 @param parent 父对象指针 */
    explicit FontManager(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~FontManager() override;

    /** @brief 从文件加载字体并注册到系统 @param name 自定义名称 @param filePath 字体文件路径 @return 是否加载成功 */
    bool loadFont(const QString &name, const QString &filePath);
    /** @brief 获取预设角色的字体 @param preset 预设角色 @return 字体对象 */
    QFont font(PresetFont preset) const;
    /** @brief 按名称获取字体 @param name 字体名称 @param pointSize 字号，-1表示使用默认 @return 字体对象 */
    QFont font(const QString &name, int pointSize = -1) const;
    /** @brief 设置预设角色的字体 @param preset 预设角色 @param font 字体对象 */
    void setFont(PresetFont preset, const QFont &font);
    /** @brief 设置预设角色的字号 @param preset 预设角色 @param pointSize 字号(磅) */
    void setFontSize(PresetFont preset, int pointSize);
    /** @brief 设置默认字体族 @param family 字体族名称 */
    void setDefaultFamily(const QString &family);
    /** @brief 获取默认字体族名称 @return 字体族名称 */
    QString defaultFamily() const;
    /** @brief 获取已加载的自定义字体名称列表 @return 名称列表 */
    QStringList loadedFonts() const;
    /** @brief 获取系统中所有可用字体族 @return 字体族列表 */
    QStringList availableFamilies() const;
    /** @brief 重置所有预设为默认值 */
    void resetToDefaults();

signals:
    /** @brief 字体文件加载完成时发射 @param name 字体名称 */
    void fontLoaded(const QString &name);
    /** @brief 预设字体被修改时发射 @param preset 预设角色 */
    void fontChanged(PresetFont preset);

private:
    /** @brief 初始化默认预设字体 */
    void initDefaults();

    QMap<PresetFont, QFont> m_presets;  ///< 预设角色到字体的映射
    QMap<QString, int> m_loadedIds;     ///< 已加载字体的名称到ID映射
    QString m_defaultFamily;            ///< 默认字体族名称
};
