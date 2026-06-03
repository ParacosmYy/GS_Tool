/**
 * @file SchemaViewer.h
 * @brief 模式查看器 — 可视化显示.proto/.fbs模式定义
 *
 * 以树形结构展示消息类型、字段定义和嵌套关系。
 * 提供解码消息/字段统计、解析错误跟踪和桥接吞吐量统计。
 */
#ifndef SCHEMA_VIEWER_H
#define SCHEMA_VIEWER_H

#include <QWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QElapsedTimer>

/**
 * @brief 模式定义查看器控件
 * 加载并可视化展示Protobuf或FlatBuffers模式文件的结构定义。
 * 同时追踪解码统计和桥接吞吐量信息。
 */
class SchemaViewer : public QWidget {
    Q_OBJECT

public:
    /** @brief 桥接吞吐量统计快照 */
    struct BridgeThroughput {
        quint64 decodedMessages = 0;    ///< 累计解码消息总数
        quint64 decodedFields = 0;      ///< 累计解码字段总数
        quint64 parseErrors = 0;        ///< 累计解析错误次数
        double messagesPerSec = 0.0;    ///< 消息解码速率(条/秒)
        double fieldsPerSec = 0.0;      ///< 字段解码速率(字段/秒)
    };

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

    // ---- 解码统计接口 ----

    /** @brief 记录一次解码消息(用于桥接吞吐量计算) @param fieldCount 本消息的字段数 */
    void recordDecodedMessage(int fieldCount);

    /** @brief 记录一次解析错误 */
    void recordParseError();

    /** @brief 获取桥接吞吐量快照 @return BridgeThroughput统计 */
    BridgeThroughput bridgeThroughput() const;

    /** @brief 获取累计解码消息总数 */
    quint64 totalDecodedMessages() const;

    /** @brief 获取累计解码字段总数 */
    quint64 totalDecodedFields() const;

    /** @brief 获取累计解析错误总数 */
    quint64 totalParseErrors() const;

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

    // ---- Schema统计 ----
    quint64 m_totalSchemasLoaded = 0;      ///< 累计加载Schema次数
    quint64 m_totalFieldExpansions = 0;    ///< 累计字段展开次数

    // ---- 解码统计 ----
    quint64 m_decodedMessages = 0;         ///< 累计解码消息总数
    quint64 m_decodedFields = 0;           ///< 累计解码字段总数
    quint64 m_parseErrors = 0;             ///< 累计解析错误次数

    // ---- 吞吐量计算 ----
    QElapsedTimer m_throughputTimer;       ///< 吞吐量基准计时器
    quint64 m_throughputMsgCount = 0;      ///< 滑动窗口内消息计数
    quint64 m_throughputFieldCount = 0;    ///< 滑动窗口内字段计数
    mutable BridgeThroughput m_cachedThroughput; ///< 缓存的吞吐量快照
};

#endif // SCHEMA_VIEWER_H
