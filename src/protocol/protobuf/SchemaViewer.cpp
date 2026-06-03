/**
 * @file SchemaViewer.cpp
 * @brief 模式查看器实现
 *
 * 加载.proto/.fbs文件并以树形结构展示消息定义。
 * 提供解码消息/字段统计、解析错误跟踪和桥接吞吐量计算。
 */

#include "protocol/protobuf/SchemaViewer.h"

#include <QFile>
#include <QTextStream>
#include <QSplitter>
#include <QHeaderView>

// ============================================================================
// 构造
// ============================================================================

/** @brief 构造函数，初始化模式查看器UI(左树+右详情)和吞吐量计时器 @param parent 父控件 */
SchemaViewer::SchemaViewer(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("SchemaViewer");

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName("schemaSplitter");

    // 模式结构树
    m_schemaTree = new QTreeWidget(this);
    m_schemaTree->setObjectName("schemaTree");
    m_schemaTree->setHeaderLabels({tr("名称"), tr("类型"), tr("编号")});
    m_schemaTree->header()->setStretchLastSection(true);

    // 详细内容视图
    m_detailView = new QTextEdit(this);
    m_detailView->setObjectName("schemaDetailView");
    m_detailView->setReadOnly(true);

    splitter->addWidget(m_schemaTree);
    splitter->addWidget(m_detailView);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    connect(m_schemaTree, &QTreeWidget::itemClicked,
            this, [this](QTreeWidgetItem* item) {
        m_detailView->setPlainText(item->data(0, Qt::UserRole).toString());
        ++m_totalFieldExpansions;
    });

    // 启动吞吐量基准计时器
    m_throughputTimer.start();
}

// ============================================================================
// Schema加载
// ============================================================================

/** @brief 加载并解析Schema文件(.proto/.fbs) @param filePath 文件路径 @param type 文件类型("proto"/"fbs") */
void SchemaViewer::loadSchema(const QString& filePath, const QString& type) {
    ++m_totalSchemasLoaded;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_detailView->setPlainText(
            tr("无法打开文件: %1").arg(filePath));
        ++m_parseErrors;
        return;
    }

    QTextStream stream(&file);
    QString content = stream.readAll();
    file.close();

    m_schemaTree->clear();

    if (type == "proto") {
        parseProtoContent(content);
    } else if (type == "fbs") {
        parseFbsContent(content);
    }

    m_detailView->setPlainText(content);
}

// ============================================================================
// Schema解析
// ============================================================================

/** @brief 解析.proto文件内容为模式树结构(message/enum+字段) @param content 文件文本内容 */
void SchemaViewer::parseProtoContent(const QString& content) {
    /* 按行解析.proto文件: 提取message/enum定义及其字段 */
    auto* rootItem = m_schemaTree->invisibleRootItem();
    QTreeWidgetItem* currentMsg = nullptr;

    for (const QString& line : content.split('\n')) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith("message ")) {
            QString name = trimmed.mid(8).split('{').first().trimmed();
            currentMsg = new QTreeWidgetItem(rootItem);
            currentMsg->setText(0, name);
            currentMsg->setText(1, tr("消息(message)"));
        } else if (trimmed.startsWith("enum ")) {
            QString name = trimmed.mid(5).split('{').first().trimmed();
            auto* enumItem = new QTreeWidgetItem(rootItem);
            enumItem->setText(0, name);
            enumItem->setText(1, tr("枚举(enum)"));
        } else if (currentMsg && trimmed.contains('=')
                   && !trimmed.startsWith("//")) {
            auto* fieldItem = new QTreeWidgetItem(currentMsg);
            QStringList parts = trimmed.split(';').first().split(' ');
            if (parts.size() >= 3) {
                fieldItem->setText(0, parts.last().split('=').first().trimmed());
                fieldItem->setText(1, parts.first());
                fieldItem->setText(2, parts.last().split('=').last().trimmed());
            }
        }
    }
}

/** @brief 解析.fbs文件内容为模式树结构(table/struct+字段) @param content 文件文本内容 */
void SchemaViewer::parseFbsContent(const QString& content) {
    /* 按行解析.fbs文件: 提取table/struct定义及其字段 */
    auto* rootItem = m_schemaTree->invisibleRootItem();
    QTreeWidgetItem* currentTable = nullptr;

    for (const QString& line : content.split('\n')) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith("table ")) {
            QString name = trimmed.mid(6).split('{').first().trimmed();
            currentTable = new QTreeWidgetItem(rootItem);
            currentTable->setText(0, name);
            currentTable->setText(1, tr("表(table)"));
        } else if (trimmed.startsWith("struct ")) {
            QString name = trimmed.mid(7).split('{').first().trimmed();
            auto* structItem = new QTreeWidgetItem(rootItem);
            structItem->setText(0, name);
            structItem->setText(1, tr("结构体(struct)"));
        } else if (currentTable && trimmed.contains(':')
                   && !trimmed.startsWith("//")) {
            auto* fieldItem = new QTreeWidgetItem(currentTable);
            QStringList parts = trimmed.split(':');
            if (parts.size() >= 2) {
                fieldItem->setText(0, parts.first().trimmed());
                fieldItem->setText(1, parts[1].split(';').first().trimmed());
            }
        }
    }
}

// ============================================================================
// Schema统计接口
// ============================================================================

/** @brief 获取累计加载Schema次数 @return 加载次数 */
quint64 SchemaViewer::totalSchemasLoaded() const
{
    return m_totalSchemasLoaded;
}

/** @brief 获取累计字段展开次数 @return 展开次数 */
quint64 SchemaViewer::totalFieldExpansions() const
{
    return m_totalFieldExpansions;
}

// ============================================================================
// 解码统计接口
// ============================================================================

/** @brief 记录一次解码消息(更新吞吐量窗口) @param fieldCount 本消息的字段数 */
void SchemaViewer::recordDecodedMessage(int fieldCount)
{
    ++m_decodedMessages;
    m_decodedFields += static_cast<quint64>(qMax(fieldCount, 0));

    // 更新吞吐量滑动窗口
    ++m_throughputMsgCount;
    m_throughputFieldCount += static_cast<quint64>(qMax(fieldCount, 0));

    // 每隔5秒重置窗口避免速率失真
    if (m_throughputTimer.elapsed() > 5000) {
        m_throughputMsgCount = 0;
        m_throughputFieldCount = 0;
        m_throughputTimer.restart();
    }
}

/** @brief 记录一次解析错误 */
void SchemaViewer::recordParseError()
{
    ++m_parseErrors;
}

/** @brief 获取桥接吞吐量快照 @return BridgeThroughput统计 */
SchemaViewer::BridgeThroughput SchemaViewer::bridgeThroughput() const
{
    qint64 elapsedMs = m_throughputTimer.elapsed();
    if (elapsedMs <= 0) {
        return m_cachedThroughput;
    }

    double elapsedSec = static_cast<double>(elapsedMs) / 1000.0;
    if (elapsedSec < 0.5) {
        return m_cachedThroughput;
    }

    m_cachedThroughput.decodedMessages = m_decodedMessages;
    m_cachedThroughput.decodedFields = m_decodedFields;
    m_cachedThroughput.parseErrors = m_parseErrors;
    m_cachedThroughput.messagesPerSec =
        static_cast<double>(m_throughputMsgCount) / elapsedSec;
    m_cachedThroughput.fieldsPerSec =
        static_cast<double>(m_throughputFieldCount) / elapsedSec;

    return m_cachedThroughput;
}

/** @brief 获取累计解码消息总数 @return 消息数 */
quint64 SchemaViewer::totalDecodedMessages() const
{
    return m_decodedMessages;
}

/** @brief 获取累计解码字段总数 @return 字段数 */
quint64 SchemaViewer::totalDecodedFields() const
{
    return m_decodedFields;
}

/** @brief 获取累计解析错误总数 @return 错误数 */
quint64 SchemaViewer::totalParseErrors() const
{
    return m_parseErrors;
}

/** @brief 重置所有统计计数器 */
void SchemaViewer::resetStatistics()
{
    m_totalSchemasLoaded = 0;
    m_totalFieldExpansions = 0;

    m_decodedMessages = 0;
    m_decodedFields = 0;
    m_parseErrors = 0;

    m_throughputMsgCount = 0;
    m_throughputFieldCount = 0;
    m_throughputTimer.restart();
    m_cachedThroughput = BridgeThroughput();
}
