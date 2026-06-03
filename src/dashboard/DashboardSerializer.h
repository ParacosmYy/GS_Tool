/**
 * @file DashboardSerializer.h
 * @brief 仪表盘布局序列化器 — JSON格式保存/加载仪表盘配置
 *
 * 将DashboardWidget中的子面板配置(类型/位置/大小/参数)
 * 序列化为JSON文件，支持跨会话持久化。
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

/**
 * @brief 仪表盘面板配置项
 */
struct DashboardItemConfig {
    QString widgetType;    ///< 面板类型(gauge/numeric/led/progressbar/chart)
    QString title;         ///< 面板标题
    int row = 0;           ///< 网格行号
    int column = 0;        ///< 网格列号
    int rowSpan = 1;       ///< 行跨度
    int columnSpan = 1;    ///< 列跨度
    QMap<QString, QVariant> properties; ///< 类型特定属性

    /** @brief 转换为JSON对象 */
    QJsonObject toJson() const;
    /** @brief 从JSON对象构建 */
    static DashboardItemConfig fromJson(const QJsonObject& obj);
};

/**
 * @brief 仪表盘布局序列化器
 *
 * 负责将仪表盘布局保存到/加载从JSON文件。
 * 文件格式:
 * {
 *   "version": 1,
 *   "name": "布局名称",
 *   "columns": 4,
 *   "items": [ DashboardItemConfig... ]
 * }
 */
class DashboardSerializer : public QObject {
    Q_OBJECT

public:
    explicit DashboardSerializer(QObject* parent = nullptr);

    /** @brief 保存布局到文件
     *  @param filePath 目标文件路径
     *  @param name 布局名称
     *  @param columns 网格列数
     *  @param items 面板配置列表
     *  @return true=保存成功
     */
    bool saveToFile(const QString& filePath, const QString& name,
                    int columns, const QList<DashboardItemConfig>& items);

    /** @brief 从文件加载布局
     *  @param filePath 源文件路径
     *  @param name 输出布局名称
     *  @param columns 输出网格列数
     *  @param items 输出面板配置列表
     *  @return true=加载成功
     */
    bool loadFromFile(const QString& filePath, QString& name,
                      int& columns, QList<DashboardItemConfig>& items);

    /** @brief 从JSON字符串加载 */
    bool loadFromJson(const QByteArray& jsonData, QString& name,
                      int& columns, QList<DashboardItemConfig>& items);

    /** @brief 序列化为JSON字节 */
    QByteArray toJson(const QString& name, int columns,
                      const QList<DashboardItemConfig>& items);

    /** @brief 获取最后错误信息 */
    QString lastError() const;

    /** @brief 获取当前文件版本号 */
    static int currentVersion();

    /**
     * @brief 验证布局配置的有效性
     * @param items 面板配置列表
     * @param columns 网格列数
     * @return 错误信息列表，空列表表示验证通过
     */
    QStringList validateLayout(const QList<DashboardItemConfig>& items,
                               int columns) const;

    /**
     * @brief 列出目录中所有布局文件
     * @param dirPath 目录路径
     * @return 文件路径列表
     */
    QStringList listLayoutFiles(const QString& dirPath) const;

    /**
     * @brief 删除指定布局文件（创建备份后删除）
     * @param filePath 布局文件路径
     * @return true 删除成功
     */
    bool deleteLayout(const QString& filePath);

signals:
    /** @brief 布局保存完成信号 */
    void layoutSaved(const QString& filePath);

    /** @brief 布局加载完成信号 */
    void layoutLoaded(const QString& name, int itemCount);

    /** @brief 布局验证失败信号 */
    void validationFailed(const QStringList& errors);

private:
    QString m_lastError;
    static constexpr int kVersion = 1;
};

#endif // DASHBOARDSERIALIZER_H
