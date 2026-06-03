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

/// @brief 仪表盘面板配置项 — 描述单个组件的类型/位置/大小/属性
struct DashboardItemConfig {
    QString widgetType;    ///< 面板类型(gauge/numeric/led/progressbar/chart)
    QString title;         ///< 面板标题
    int row = 0;           ///< 网格行号
    int column = 0;        ///< 网格列号
    int rowSpan = 1;       ///< 行跨度
    int columnSpan = 1;    ///< 列跨度
    QMap<QString, QVariant> properties; ///< 类型特定属性

    QJsonObject toJson() const;                           ///< 转换为JSON对象
    static DashboardItemConfig fromJson(const QJsonObject& obj); ///< 从JSON构建
};

/**
 * @brief 仪表盘布局序列化器
 * @details JSON文件格式: {version, name, columns, items[]}
 * QSettings: DashboardProfiles/<name>/name|columns|items
 */
class DashboardSerializer : public QObject {
    Q_OBJECT

public:
    explicit DashboardSerializer(QObject* parent = nullptr);

    // ---- JSON文件操作 ----
    bool saveToFile(const QString& filePath, const QString& name,   ///< 保存布局到文件
                    int columns, const QList<DashboardItemConfig>& items);
    bool loadFromFile(const QString& filePath, QString& name,       ///< 从文件加载布局
                      int& columns, QList<DashboardItemConfig>& items);
    bool loadFromJson(const QByteArray& jsonData, QString& name,    ///< 从JSON字节加载
                      int& columns, QList<DashboardItemConfig>& items);
    QByteArray toJson(const QString& name, int columns,             ///< 序列化为JSON字节
                      const QList<DashboardItemConfig>& items);
    QString lastError() const;       ///< 获取最后错误信息
    static int currentVersion();     ///< 获取当前文件版本号

    // ---- 验证与文件管理 ----
    QStringList validateLayout(const QList<DashboardItemConfig>& items, int columns) const; ///< 验证布局
    QStringList listLayoutFiles(const QString& dirPath) const;  ///< 列出目录中布局文件
    bool deleteLayout(const QString& filePath);                 ///< 删除布局文件(备份后)

    // ---- QSettings命名配置文件系统 ----
    bool saveToProfile(const QString& profileName, const QString& name, ///< 保存到配置文件
                       int columns, const QList<DashboardItemConfig>& items);
    bool loadFromProfile(const QString& profileName, QString& name,     ///< 从配置文件加载
                         int& columns, QList<DashboardItemConfig>& items);
    bool deleteProfile(const QString& profileName);      ///< 删除配置文件
    bool renameProfile(const QString& oldName, const QString& newName); ///< 重命名
    QStringList listProfiles() const;                    ///< 列出所有配置文件名
    void setCurrentProfile(const QString& profileName);  ///< 设置当前激活配置
    QString currentProfile() const;                      ///< 获取当前激活配置名
    bool hasProfile(const QString& profileName) const;   ///< 检查配置是否存在

    // ---- 统计接口 ----
    quint64 totalSaves() const;        ///< 累计保存次数(文件+配置文件)
    quint64 totalLoads() const;        ///< 累计加载次数(文件+配置文件)
    quint64 totalValidations() const;  ///< 累计验证次数
    quint64 totalDeletes() const;      ///< 累计删除次数
    quint64 totalErrors() const;       ///< 累计错误次数
    quint64 totalProfileSaves() const; ///< 累计配置文件保存次数
    quint64 totalProfileLoads() const; ///< 累计配置文件加载次数
    quint64 totalExports() const;      ///< 累计导出JSON次数
    quint64 totalImports() const;      ///< 累计导入JSON次数

    /** @brief 获取已保存配置文件的最高版本号 @return 最高版本号，无记录时返回0 */
    int maxProfileVersionSaved() const;

    /** @brief 获取已加载配置文件的最低版本号 @return 最低版本号，无记录时返回0 */
    int minProfileVersionLoaded() const;

    /** @brief 获取累计序列化输出字节数(所有saveToFile的JSON字节数之和) @return 字节总数 */
    quint64 totalBytesSerialized() const;

    /** @brief 获取累计反序列化输入字节数(所有loadFromJson的原始字节数之和) @return 字节总数 */
    quint64 totalBytesDeserialized() const;

    /** @brief 获取累计序列化错误次数(saveToFile失败) @return 错误次数 */
    quint64 serializationErrors() const;

    /** @brief 获取累计反序列化错误次数(loadFromJson/loadFromFile失败) @return 错误次数 */
    quint64 deserializationErrors() const;

    void resetSerializerStatistics();  ///< 重置统计计数器

signals:
    void layoutSaved(const QString& filePath);              ///< 布局保存完成
    void layoutLoaded(const QString& name, int itemCount);  ///< 布局加载完成
    void validationFailed(const QStringList& errors);       ///< 验证失败
    void profileSaved(const QString& profileName);          ///< 配置文件保存完成
    void profileLoaded(const QString& profileName, int itemCount); ///< 配置文件加载完成
    void profileDeleted(const QString& profileName);        ///< 配置文件删除完成
    void currentProfileChanged(const QString& profileName); ///< 当前配置变更

private:
    QString m_lastError;
    static constexpr int kVersion = 1;

    void ensureProfileListContains(const QString& profileName); ///< 确保列表包含名称
    void removeProfileListEntry(const QString& profileName);    ///< 从列表移除名称

    // ---- 统计计数器 ----
    mutable quint64 m_totalSaves = 0;
    mutable quint64 m_totalLoads = 0;
    mutable quint64 m_totalValidations = 0;
    mutable quint64 m_totalDeletes = 0;
    mutable quint64 m_totalErrors = 0;
    mutable quint64 m_totalProfileSaves = 0;
    mutable quint64 m_totalProfileLoads = 0;
    mutable quint64 m_totalExports = 0;
    mutable quint64 m_totalImports = 0;

    // ---- 扩展统计计数器 ----
    int m_maxProfileVersionSaved = 0;          ///< 已保存配置的最高版本号
    int m_minProfileVersionLoaded = 0;         ///< 已加载配置的最低版本号
    bool m_hasLoadedVersion = false;           ///< 是否已加载过配置版本（用于初始化最小值）
    mutable quint64 m_totalBytesSerialized = 0;      ///< 累计序列化输出字节数
    mutable quint64 m_totalBytesDeserialized = 0;    ///< 累计反序列化输入字节数
    mutable quint64 m_serializationErrors = 0;       ///< 累计序列化错误次数
    mutable quint64 m_deserializationErrors = 0;     ///< 累计反序列化错误次数
};

#endif // DASHBOARDSERIALIZER_H
