#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariant>

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

private:
    SettingsManager(QObject* parent = nullptr);
    ~SettingsManager() override;
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
