/**
 * @file ConnectionProfileManager.h
 * @brief 连接配置管理器 — 保存/加载/切换连接配置
 *
 * 功能: 管理多套连接配置(Serial/TCP/UDP/BLE等)，支持快速切换、
 *       导入导出、配置克隆、最近使用记录。
 *
 * 协作: ConnectionFactory(创建连接) / SettingsManager(持久化)
 */
#ifndef CONNECTIONPROFILEMANAGER_H
#define CONNECTIONPROFILEMANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QVariantMap>

/**
 * @brief 连接配置管理器 — 多设备配置快速切换
 */
class ConnectionProfileManager : public QObject {
    Q_OBJECT

public:
    /** @brief 连接类型 */
    enum class ConnectionType {
        Serial,     ///< 串口
        TcpClient,  ///< TCP客户端
        TcpServer,  ///< TCP服务端
        Udp,        ///< UDP
        WebSocket,  ///< WebSocket
        Ble         ///< BLE蓝牙
    };
    Q_ENUM(ConnectionType)

    /** @brief 连接配置 */
    struct Profile {
        QString name;               ///< 配置名称
        ConnectionType type;        ///< 连接类型
        QVariantMap settings;       ///< 类型特定配置
        qint64 lastUsed = 0;        ///< 最后使用时间(ms)
        int useCount = 0;           ///< 使用次数
        bool isFavorite = false;    ///< 是否收藏
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalProfiles = 0;
        quint64 totalSwitches = 0;
        quint64 totalImports = 0;
        quint64 totalExports = 0;
        quint64 totalCreations = 0;
        quint64 totalDeletions = 0;
        int     peakProfiles = 0;
    };

    explicit ConnectionProfileManager(QObject* parent = nullptr);

    /** @brief 创建新配置 @param name 名称 @param type 连接类型 @return 配置ID */
    QString createProfile(const QString& name, ConnectionType type);

    /** @brief 删除配置 @param id 配置ID @return 是否成功 */
    bool deleteProfile(const QString& id);

    /** @brief 更新配置 @param id 配置ID @param profile 新数据 @return 是否成功 */
    bool updateProfile(const QString& id, const Profile& profile);

    /** @brief 获取配置 @param id 配置ID @return 配置数据 */
    Profile profile(const QString& id) const;

    /** @brief 获取所有配置 @return 配置映射 */
    QMap<QString, Profile> allProfiles() const;

    /** @brief 获取最近使用的配置 @param count 数量 @return 配置列表(按时间降序) */
    QList<Profile> recentProfiles(int count = 5) const;

    /** @brief 获取收藏的配置 @return 收藏列表 */
    QList<Profile> favoriteProfiles() const;

    /** @brief 激活配置(标记为当前使用) @param id 配置ID */
    void activateProfile(const QString& id);

    /** @brief 克隆配置 @param sourceId 源配置ID @param newName 新名称 @return 新配置ID */
    QString cloneProfile(const QString& sourceId, const QString& newName);

    /** @brief 导出配置 @param id 配置ID @param filePath 目标路径 @return 是否成功 */
    bool exportProfile(const QString& id, const QString& filePath);

    /** @brief 导入配置 @param filePath JSON文件路径 @return 新配置ID */
    QString importProfile(const QString& filePath);

    /** @brief 保存所有配置到磁盘 */
    void save();

    /** @brief 从磁盘加载配置 */
    void load();

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void profileCreated(const QString& id);
    void profileDeleted(const QString& id);
    void profileUpdated(const QString& id);
    void profileActivated(const QString& id);
    void profilesChanged();

private:
    QString generateId() const;
    void updateRecentList(const QString& id);

    QMap<QString, Profile> m_profiles;  ///< 配置映射(ID→Profile)
    QString m_activeProfileId;          ///< 当前激活的配置ID
    QStringList m_recentIds;            ///< 最近使用ID列表(最多20个)

    Stats m_stats;                      ///< 运行统计
};

#endif // CONNECTIONPROFILEMANAGER_H
