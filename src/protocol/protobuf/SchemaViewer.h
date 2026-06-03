/**
 * @file SchemaViewer.h
 * @brief 模式查看器 — 可视化显示.proto/.fbs模式定义
 *
 * 以树形结构展示消息类型、字段定义和嵌套关系。
 */
#ifndef SCHEMA_VIEWER_H
#define SCHEMA_VIEWER_H

#include <QWidget>
#include <QTreeWidget>
#include <QTextEdit>

/**
 * @brief 模式定义查看器控件
 * 加载并可视化展示Protobuf或FlatBuffers模式文件的结构定义。
 */
class SchemaViewer : public QWidget {
    Q_OBJECT

public:
    explicit SchemaViewer(QWidget* parent = nullptr);

    /**
     * @brief 加载模式文件
     * @param filePath 模式文件路径
     * @param type 类型 ("proto" 或 "fbs")
     */
    void loadSchema(const QString& filePath, const QString& type);

    /** @brief 获取累计加载Schema次数 */
    quint64 totalSchemasLoaded() const;

    /** @brief 获取累计字段展开次数 */
    quint64 totalFieldExpansions() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

private:
    /**
     * @brief 解析.proto文件内容并填充树
     * @param content 文件内容
     */
    void parseProtoContent(const QString& content);

    /**
     * @brief 解析.fbs文件内容并填充树
     * @param content 文件内容
     */
    void parseFbsContent(const QString& content);

    QTreeWidget* m_schemaTree  = nullptr; ///< 模式结构树
    QTextEdit*   m_detailView  = nullptr; ///< 详细内容视图

    // ---- 统计计数器 ----
    quint64 m_totalSchemasLoaded = 0;      ///< 累计加载Schema次数
    quint64 m_totalFieldExpansions = 0;    ///< 累计字段展开次数
};

#endif // SCHEMA_VIEWER_H
