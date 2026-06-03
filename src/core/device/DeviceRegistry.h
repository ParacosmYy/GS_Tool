/**
 * @file DeviceRegistry.h
 * @brief 设备配置注册表
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 管理设备配置文件的增删查和持久化存储。
 */

#ifndef DEVICEREGISTRY_H
#define DEVICEREGISTRY_H

#include <QList>
#include <QObject>
#include <QString>

#include "core/device/DeviceProfile.h"

/**
 * @class DeviceRegistry
 * @brief 设备配置文件注册表，管理设备列表和持久化
 */
class DeviceRegistry : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit DeviceRegistry(QObject *parent = nullptr);

    /**
     * @brief 添加设备配置
     * @param profile 设备配置
     */
    void addProfile(const DeviceProfile &profile);

    /**
     * @brief 移除指定名称的设备配置
     * @param name 设备名称
     */
    void removeProfile(const QString &name);

    /**
     * @brief 获取所有设备配置
     * @return 配置列表
     */
    QList<DeviceProfile> profiles() const;

    /**
     * @brief 按名称查找设备配置
     * @param name 设备名称
     * @return 设备配置（未找到时 name 为空）
     */
    DeviceProfile findProfile(const QString &name) const;

    /**
     * @brief 保存所有配置到 JSON 文件
     * @param filePath 文件路径
     * @return 是否保存成功
     */
    bool saveToFile(const QString &filePath) const;

    /**
     * @brief 从 JSON 文件加载配置
     * @param filePath 文件路径
     * @return 是否加载成功
     */
    bool loadFromFile(const QString &filePath);

signals:
    /**
     * @brief 配置添加信号
     * @param name 设备名称
     */
    void profileAdded(const QString &name);

    /**
     * @brief 配置移除信号
     * @param name 设备名称
     */
    void profileRemoved(const QString &name);

    /**
     * @brief 配置列表变更信号
     */
    void profilesChanged();

private:
    QList<DeviceProfile> m_profiles;    ///< 设备配置列表

    // ---- 统计计数器 ----
    mutable quint64 m_totalAdds = 0;            ///< 累计添加次数
    mutable quint64 m_totalRemoves = 0;         ///< 累计移除次数
    mutable quint64 m_totalSaves = 0;           ///< 累计保存次数
    mutable quint64 m_totalLoads = 0;           ///< 累计加载次数
public:
    quint64 totalAdds() const { return m_totalAdds; }
    quint64 totalRemoves() const { return m_totalRemoves; }
    quint64 totalSaves() const { return m_totalSaves; }
    quint64 totalLoads() const { return m_totalLoads; }
    void resetRegistryStatistics() { m_totalAdds = 0; m_totalRemoves = 0; m_totalSaves = 0; m_totalLoads = 0; }
};

#endif // DEVICEREGISTRY_H
