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

    /** @brief 将配置项转换为JSON对象 @return QJsonObject */
    QJsonObject toJson() const;
    /** @brief 从JSON对象构建配置项 @param obj JSON对象 @return 配置项 */
    static DashboardItemConfig fromJson(const QJsonObject& obj);
};

/** @brief 仪表盘布局序列化器 — JSON文件格式: {version, name, columns, items[]}, QSettings: DashboardProfiles/<name>/name|columns|items */
class DashboardSerializer : public QObject {
    Q_OBJECT

public:
    /** @brief 构造序列化器 @param parent 父对象 */
    explicit DashboardSerializer(QObject* parent = nullptr);

    // ---- JSON文件操作 ----
    /** @brief 保存布局到JSON文件 @param filePath 目标文件路径 @param name 布局名称 @param columns 网格列数 @param items 面板配置列表 @return true=保存成功 */
    bool saveToFile(const QString& filePath, const QString& name,
                    int columns, const QList<DashboardItemConfig>& items);
    /** @brief 从JSON文件加载布局 @param filePath 源文件路径 @param name [out]布局名称 @param columns [out]网格列数 @param items [out]面板配置列表 @return true=加载成功 */
    bool loadFromFile(const QString& filePath, QString& name,
                      int& columns, QList<DashboardItemConfig>& items);
    /** @brief 从JSON字节数据加载布局 @param jsonData JSON字节数据 @param name [out]布局名称 @param columns [out]网格列数 @param items [out]面板配置列表 @return true=加载成功 */
    bool loadFromJson(const QByteArray& jsonData, QString& name,
                      int& columns, QList<DashboardItemConfig>& items);
    /** @brief 将布局序列化为JSON字节数据 @param name 布局名称 @param columns 网格列数 @param items 面板配置列表 @return JSON字节数据 */
    QByteArray toJson(const QString& name, int columns,
                      const QList<DashboardItemConfig>& items);
    /** @brief 获取最后一次操作的错误信息 @return 错误描述字符串 */
    QString lastError() const;
    /** @brief 获取当前序列化文件格式版本号 @return 版本号 */
    static int currentVersion();

    // ---- 验证与文件管理 ----
    /** @brief 验证布局配置合法性 @param items 面板配置列表 @param columns 网格列数 @return 错误信息列表，空表示合法 */
    QStringList validateLayout(const QList<DashboardItemConfig>& items, int columns) const;
    /** @brief 列出指定目录下的布局文件 @param dirPath 目录路径 @return 文件名列表 */
    QStringList listLayoutFiles(const QString& dirPath) const;
    /** @brief 删除布局文件(自动备份) @param filePath 文件路径 @return true=删除成功 */
    bool deleteLayout(const QString& filePath);

    // ---- QSettings命名配置文件系统 ----
    /** @brief 保存布局到QSettings命名配置 @param profileName 配置名称 @param name 布局名称 @param columns 网格列数 @param items 面板配置列表 @return true=保存成功 */
    bool saveToProfile(const QString& profileName, const QString& name,
                       int columns, const QList<DashboardItemConfig>& items);
    /** @brief 从QSettings命名配置加载布局 @param profileName 配置名称 @param name [out]布局名称 @param columns [out]网格列数 @param items [out]面板配置列表 @return true=加载成功 */
    bool loadFromProfile(const QString& profileName, QString& name,
                         int& columns, QList<DashboardItemConfig>& items);
    /** @brief 删除QSettings命名配置 @param profileName 配置名称 @return true=删除成功 */
    bool deleteProfile(const QString& profileName);
    /** @brief 重命名配置文件 @param oldName 原名称 @param newName 新名称 @return true=成功 */
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
    /** @brief 获取累计配置文件保存次数 @return 保存总数 */
    quint64 totalProfileSaves() const;
    /** @brief 获取累计配置文件加载次数 @return 加载总数 */
    quint64 totalProfileLoads() const;
    /** @brief 获取累计导出JSON次数 @return 导出总数 */
    quint64 totalExports() const;
    /** @brief 获取累计导入JSON次数 @return 导入总数 */
    quint64 totalImports() const;

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

    // ---- 序列化生命周期统计 ----

    /** @brief 获取累计序列化操作次数(toJson/saveToFile/saveToProfile成功) @return 序列化总数 */
    quint64 totalSerializations() const;

    /** @brief 获取累计反序列化操作次数(loadFromJson/loadFromFile/loadFromProfile成功) @return 反序列化总数 */
    quint64 totalDeserializations() const;

    /** @brief 获取累计序列化错误次数(所有保存操作失败) @return 错误次数 */
    quint64 totalSerializationErrors() const;

    /** @brief 获取累计写入文件字节数(file.write实际写入字节数) @return 字节总数 */
    quint64 totalBytesWritten() const;

    /** @brief 获取累计读取文件字节数(file.readAll读取字节数) @return 字节总数 */
    quint64 totalBytesRead() const;

    /** @brief 重置所有统计计数器 */
    void resetSerializerStatistics();

signals:
    /** @brief 布局保存到文件完成 @param filePath 保存路径 */
    void layoutSaved(const QString& filePath);
    /** @brief 布局从文件加载完成 @param name 布局名称 @paramItemCount 加载的面板数量 */
    void layoutLoaded(const QString& name, int itemCount);
    /** @brief 布局验证失败 @param errors 错误信息列表 */
    void validationFailed(const QStringList& errors);
    /** @brief 配置文件保存完成 @param profileName 配置名称 */
    void profileSaved(const QString& profileName);
    /** @brief 配置文件加载完成 @param profileName 配置名称 @param itemCount 面板数量 */
    void profileLoaded(const QString& profileName, int itemCount);
    /** @brief 配置文件删除完成 @param profileName 配置名称 */
    void profileDeleted(const QString& profileName);
    /** @brief 当前激活配置变更 @param profileName 新配置名称 */
    void currentProfileChanged(const QString& profileName);

private:
    QString m_lastError;
    static constexpr int kVersion = 1;

    /** @brief 确保配置列表包含指定名称 @param profileName 配置名称 */
    void ensureProfileListContains(const QString& profileName);
    /** @brief 从配置列表中移除指定名称 @param profileName 配置名称 */
    void removeProfileListEntry(const QString& profileName);

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

    // ---- 序列化生命周期统计计数器 ----
    mutable quint64 m_totalSerializations = 0;       ///< 累计序列化操作次数
    mutable quint64 m_totalDeserializations = 0;     ///< 累计反序列化操作次数
    mutable quint64 m_totalSerializationErrors = 0;  ///< 累计序列化错误次数(保存失败)
    mutable quint64 m_totalBytesWritten = 0;         ///< 累计写入文件字节数
    mutable quint64 m_totalBytesRead = 0;            ///< 累计读取文件字节数
};

#endif // DASHBOARDSERIALIZER_H
