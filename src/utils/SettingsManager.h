#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QVariantMap>
#include <QByteArray>
#include <QString>

// 配置管理器 - 封装QSettings，提供类型安全的配置读写
// 使用JSON格式存储配置文件
class SettingsManager : public QObject {
    Q_OBJECT

public:
    static SettingsManager& instance();

    // 读取配置值，不存在时返回defaultValue
    QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) const;

    // 写入配置值
    void set(const QString& key, const QVariant& value);

    // 删除配置项
    void remove(const QString& key);

    // 检查配置项是否存在
    bool contains(const QString& key) const;

    // 同步到磁盘
    void sync();

    // --- 便捷方法: 串口配置 ---
    // 保存串口配置(端口名、波特率、数据位等)
    void saveSerialConfig(const QVariantMap& config);
    // 加载串口配置, 不存在时返回空map
    QVariantMap loadSerialConfig() const;

    // --- 便捷方法: 窗口几何 ---
    // 保存窗口位置和大小
    void saveWindowGeometry(const QByteArray& geometry);
    // 加载窗口位置和大小, 不存在时返回空
    QByteArray loadWindowGeometry() const;

    // --- 便捷方法: 主题 ---
    // 保存主题名称
    void saveTheme(const QString& themeName);
    // 加载主题名称, 不存在时返回默认主题
    QString loadTheme() const;

private:
    SettingsManager(QObject* parent = nullptr);
    ~SettingsManager() override;
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
