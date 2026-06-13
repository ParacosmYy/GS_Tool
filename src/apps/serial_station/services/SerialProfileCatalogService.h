#ifndef SERIAL_PROFILE_CATALOG_SERVICE_H
#define SERIAL_PROFILE_CATALOG_SERVICE_H

#include <QtCore/QString>
#include <QtCore/QStringList>

class SettingsManager;

namespace serial_station {

/**
 * @brief Serial Station 配置档案索引服务。
 *
 * 只负责最近档案路径和上次档案路径的持久化，不读取或写入
 * `.edserialprofile` 内容。档案内容仍由 SerialProfileService 负责。
 */
class SerialProfileCatalogService {
public:
    explicit SerialProfileCatalogService(SettingsManager* settings = nullptr);

    /**
     * @brief 记录一条成功使用过的档案路径。
     * @param filePath 档案文件路径
     * @return true 表示路径有效并已写入索引
     */
    bool recordProfilePath(const QString& filePath);

    /**
     * @brief 最近档案路径，按最近使用优先排序。
     */
    QStringList recentProfilePaths() const;

    /**
     * @brief 判断档案路径是否已经进入最近档案索引。
     */
    bool containsProfilePath(const QString& filePath) const;

    /**
     * @brief 从最近档案索引移除一条路径。
     * @return true 表示路径存在且已移除
     */
    bool removeProfilePath(const QString& filePath);

    /**
     * @brief 最近一次成功使用的档案路径。
     */
    QString lastProfilePath() const;

    /**
     * @brief 清理已持久化的档案索引。
     */
    void clear();

private:
    QString normalizePath(const QString& filePath) const;
    QStringList normalizedRecentPaths() const;
    void writeRecentPaths(const QStringList& paths);

    SettingsManager* m_settings = nullptr; ///< 应用配置管理器，默认复用全局 SettingsManager
};

} // namespace serial_station

#endif // SERIAL_PROFILE_CATALOG_SERVICE_H
