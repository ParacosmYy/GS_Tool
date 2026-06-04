/**
 * @file DashboardSerializer.h
 * @brief 仪表盘布局序列化器 — JSON格式保存/加载 + QSettings配置文件系统
 *
 * 完整属性序列化(gauge范围/LED颜色/数值格式/进度条范围)、
 * QSettings命名配置文件管理、JSON文件导入导出、操作统计。
 */
#ifndef DASHBOARDSERIALIZER_H
#define DASHBOARDSERIALIZER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QJsonObject>
#include <QSettings>

/** @brief 仪表盘面板配置项 — 描述单个组件的类型/位置/大小/属性 */
struct DashboardItemConfig {
    QString widgetType;    ///< 面板类型(gauge/numeric/led/progressbar/chart)
    QString title;         ///< 面板标题
    int row = 0;           ///< 网格行号
    int column = 0;        ///< 网格列号
    int rowSpan = 1;       ///< 行跨度
    int columnSpan = 1;    ///< 列跨度
    QMap<QString, QVariant> properties; ///< 类型特定属性
    QJsonObject toJson() const; ///< 将配置项转换为JSON对象
    static DashboardItemConfig fromJson(const QJsonObject& obj); ///< 从JSON对象构建配置项
};

/** @brief 仪表盘布局序列化器 — JSON文件+QSettings配置文件系统 */
class DashboardSerializer : public QObject {
    Q_OBJECT

public:
    explicit DashboardSerializer(QObject* parent = nullptr); ///< 构造序列化器
    // ---- JSON文件操作 ----
    /** @brief 保存布局到JSON文件 @param filePath 目标文件路径 @param name 布局名称 @param columns 网格列数 @param items 面板配置列表 @return true=保存成功 */
    bool saveToFile(const QString& filePath, const QString& name, int columns, const QList<DashboardItemConfig>& items);
    /** @brief 从JSON文件加载布局 @param filePath 源文件路径 @param name [out]布局名称 @param columns [out]网格列数 @param items [out]面板配置列表 @return true=加载成功 */
    bool loadFromFile(const QString& filePath, QString& name, int& columns, QList<DashboardItemConfig>& items);
    /** @brief 从JSON字节数据加载布局 @param jsonData JSON字节数据 @return true=加载成功 */
    bool loadFromJson(const QByteArray& jsonData, QString& name, int& columns, QList<DashboardItemConfig>& items);
    /** @brief 将布局序列化为JSON字节数据 @return JSON字节数据 */
    QByteArray toJson(const QString& name, int columns, const QList<DashboardItemConfig>& items);
    /** @brief 获取最后一次操作的错误信息 @return 错误字符串 */
    QString lastError() const;
    /** @brief 获取当前序列化文件格式版本号 @return 版本号 */
    static int currentVersion();
    // ---- 验证与文件管理 ----
    /** @brief 验证布局配置合法性 @return 错误信息列表，空表示合法 */
    QStringList validateLayout(const QList<DashboardItemConfig>& items, int columns) const;
    /** @brief 列出指定目录下的布局文件 @param dirPath 目录路径 @return JSON文件名列表 */
    QStringList listLayoutFiles(const QString& dirPath) const;
    /** @brief 删除布局文件(自动备份) @param filePath 目标文件路径 @return true=删除成功 */
    bool deleteLayout(const QString& filePath);
    // ---- QSettings命名配置文件系统 ----
    /** @brief 保存布局到QSettings命名配置 @return true=保存成功 */
    bool saveToProfile(const QString& profileName, const QString& name, int columns, const QList<DashboardItemConfig>& items);
    /** @brief 从QSettings命名配置加载布局 @return true=加载成功 */
    bool loadFromProfile(const QString& profileName, QString& name, int& columns, QList<DashboardItemConfig>& items);
    /** @brief 删除QSettings命名配置 @param profileName 配置名称 @return true=删除成功 */
    bool deleteProfile(const QString& profileName);
    /** @brief 重命名配置文件 @param oldName 原名称 @param newName 新名称 @return true=重命名成功 */
    bool renameProfile(const QString& oldName, const QString& newName);
    /** @brief 列出所有QSettings配置文件名 @return 配置名称列表 */
    QStringList listProfiles() const;
    /** @brief 设置当前激活的配置文件 @param profileName 配置名称 */
    void setCurrentProfile(const QString& profileName);
    /** @brief 获取当前激活的配置文件名 @return 配置名称 */
    QString currentProfile() const;
    /** @brief 检查指定配置文件是否存在 @param profileName 配置名称 @return true=存在 */
    bool hasProfile(const QString& profileName) const;
    // ---- 统计接口 ----

    /** @brief 序列化器运行统计数据结构体，聚合全部运行期间计数器 */
    struct Stats {
        quint64 totalSaves = 0;               ///< 累计保存次数(文件+配置文件)
        quint64 totalLoads = 0;               ///< 累计加载次数(文件+配置文件)
        quint64 totalValidations = 0;         ///< 累计验证次数
        quint64 totalDeletes = 0;             ///< 累计删除次数
        quint64 totalErrors = 0;              ///< 累计错误次数
        quint64 totalSaveFailures = 0;        ///< 累计保存失败次数
        quint64 totalLoadFailures = 0;        ///< 累计加载失败次数
        quint64 totalProfileSaves = 0;        ///< 累计配置文件保存次数
        quint64 totalProfileLoads = 0;        ///< 累计配置文件加载次数
        quint64 totalExports = 0;             ///< 累计导出JSON次数
        quint64 totalImports = 0;             ///< 累计导入JSON次数
        int maxProfileVersionSaved = 0;       ///< 已保存配置的最高版本号
        int minProfileVersionLoaded = 0;      ///< 已加载配置的最低版本号
        bool hasLoadedVersion = false;        ///< 是否已加载过配置版本
        quint64 totalBytesSerialized = 0;     ///< 累计序列化输出字节数
        quint64 totalBytesDeserialized = 0;   ///< 累计反序列化输入字节数
        quint64 serializationErrors = 0;      ///< 累计序列化错误次数
        quint64 deserializationErrors = 0;    ///< 累计反序列化错误次数
        quint64 totalSerializations = 0;      ///< 累计序列化操作次数
        quint64 totalDeserializations = 0;    ///< 累计反序列化操作次数
        quint64 totalSerializationErrors = 0; ///< 累计序列化错误次数
        quint64 totalBytesWritten = 0;        ///< 累计写入文件字节数
        quint64 totalBytesRead = 0;           ///< 累计读取文件字节数
        quint64 profilesManaged = 0;          ///< 累计管理过的配置文件总数(增/删/重命名)
    };

    /** @brief 获取累计保存次数(文件+配置文件) @return 保存总数 */
    quint64 totalSaves() const;
    /** @brief 获取累计加载次数(文件+配置文件) @return 加载总数 */
    quint64 totalLoads() const;
    /** @brief 获取累计验证次数 @return 验证总数 */
    quint64 totalValidations() const;
    /** @brief 获取累计删除次数 @return 删除总数 */
    quint64 totalDeletes() const;
    /** @brief 获取累计错误次数 @return 错误总数 */
    quint64 totalErrors() const;
    /** @brief 获取累计保存失败次数 @return 失败总数 */
    quint64 totalSaveFailures() const { return m_stats.totalSaveFailures; }
    /** @brief 获取累计加载失败次数 @return 失败总数 */
    quint64 totalLoadFailures() const { return m_stats.totalLoadFailures; }
    /** @brief 获取累计配置文件保存次数 @return 保存计数 */
    quint64 totalProfileSaves() const;
    /** @brief 获取累计配置文件加载次数 @return 加载计数 */
    quint64 totalProfileLoads() const;
    /** @brief 获取累计导出JSON次数 @return 导出计数 */
    quint64 totalExports() const;
    /** @brief 获取累计导入JSON次数 @return 导入计数 */
    quint64 totalImports() const;
    /** @brief 获取已保存配置的最高版本号 @return 版本号 */
    int maxProfileVersionSaved() const;
    /** @brief 获取已加载配置的最低版本号 @return 版本号 */
    int minProfileVersionLoaded() const;
    /** @brief 获取累计序列化输出字节数 @return 字节数 */
    quint64 totalBytesSerialized() const;
    /** @brief 获取累计反序列化输入字节数 @return 字节数 */
    quint64 totalBytesDeserialized() const;
    /** @brief 获取累计序列化错误次数 @return 错误计数 */
    quint64 serializationErrors() const;
    /** @brief 获取累计反序列化错误次数 @return 错误计数 */
    quint64 deserializationErrors() const;
    /** @brief 获取累计序列化操作次数 @return 操作计数 */
    quint64 totalSerializations() const;
    /** @brief 获取累计反序列化操作次数 @return 操作计数 */
    quint64 totalDeserializations() const;
    /** @brief 获取累计序列化错误次数 @return 错误计数 */
    quint64 totalSerializationErrors() const;
    /** @brief 获取累计写入文件字节数 @return 字节数 */
    quint64 totalBytesWritten() const;
    /** @brief 获取累计读取文件字节数 @return 字节数 */
    quint64 totalBytesRead() const;
    /** @brief 获取累计管理过的配置文件总数 @return 管理总数 */
    quint64 profilesManaged() const { return m_stats.profilesManaged; }
    /** @brief 获取统计数据的只读引用 @return Stats常量引用 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置所有统计计数器 */
    void resetSerializerStatistics();

signals:
    /** @brief 布局保存到文件完成 @param filePath 文件路径 */
    void layoutSaved(const QString& filePath);
    /** @brief 布局加载完成 @param name 布局名称 @param itemCount 面板数量 */
    void layoutLoaded(const QString& name, int itemCount);
    /** @brief 布局验证失败 @param errors 错误信息列表 */
    void validationFailed(const QStringList& errors);
    /** @brief 配置文件保存完成 @param profileName 配置名称 */
    void profileSaved(const QString& profileName);
    /** @brief 配置文件加载完成 @param profileName 配置名称 @param itemCount 面板数量 */
    void profileLoaded(const QString& profileName, int itemCount);
    /** @brief 配置文件删除完成 @param profileName 配置名称 */
    void profileDeleted(const QString& profileName);
    /** @brief 当前配置变更 @param profileName 新配置名称 */
    void currentProfileChanged(const QString& profileName);

private:
    QString m_lastError;
    static constexpr int kVersion = 1;
    /** @brief 确保配置列表包含指定名称 @param profileName 配置名称 */
    void ensureProfileListContains(const QString& profileName);
    /** @brief 从配置列表中移除指定名称 @param profileName 配置名称 */
    void removeProfileListEntry(const QString& profileName);
    // ---- 统计计数器 ----
    mutable Stats m_stats;    ///< 聚合统计结构体(mutable因const方法需修改)
};

#endif // DASHBOARDSERIALIZER_H
