/**
 * @file ProtocolSchemaEditor.cpp
 * @brief 协议帧结构 JSON 编辑器实现
 *
 * 提供 JSON 文本编辑、协议验证和状态反馈功能。
 * 用户可在编辑区输入协议 JSON 定义，通过验证按钮检查格式正确性。
 */

#include "protocol/editor/ProtocolSchemaEditor.h"
#include "protocol/schema/ProtocolSchema.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QStyle>

/** @brief 构造函数(初始化标题+编辑区+按钮+状态标签) @param parent 父控件指针 */
ProtocolSchemaEditor::ProtocolSchemaEditor(QWidget *parent)
    : QWidget(parent)
    , m_jsonEditor(nullptr)
    , m_validateBtn(nullptr)
    , m_statusLabel(nullptr)
    , m_schema(nullptr)
{
    setObjectName(QStringLiteral("ProtocolSchemaEditor"));
    setupUI();
}

/**
 * @brief 析构函数
 */
ProtocolSchemaEditor::~ProtocolSchemaEditor() = default;

/** @brief 初始化界面布局(标题+JSON编辑区+按钮行+状态标签) */
void ProtocolSchemaEditor::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    /* ---- 标题标签 ---- */
    auto *titleLabel = new QLabel(tr("协议模式编辑器"), this);
    titleLabel->setObjectName(QStringLiteral("editorTitleLabel"));
    mainLayout->addWidget(titleLabel);

    /* ---- JSON 文本编辑区 ---- */
    m_jsonEditor = new QTextEdit(this);
    m_jsonEditor->setObjectName(QStringLiteral("jsonEditor"));
    m_jsonEditor->setPlaceholderText(
        tr("在此粘贴或编写JSON协议定义..."));

    QFont monoFont(QStringLiteral("Courier New"), 10);
    monoFont.setStyleHint(QFont::Monospace);
    m_jsonEditor->setFont(monoFont);

    mainLayout->addWidget(m_jsonEditor, /*stretch=*/1);

    /* ---- 操作按钮行 ---- */
    auto *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);

    m_validateBtn = new QPushButton(tr("验证"), this);
    m_validateBtn->setObjectName(QStringLiteral("validateBtn"));
    btnLayout->addWidget(m_validateBtn);

    auto *saveBtn = new QPushButton(tr("保存"), this);
    saveBtn->setObjectName(QStringLiteral("saveBtn"));
    btnLayout->addWidget(saveBtn);

    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    /* ---- 状态提示标签 ---- */
    m_statusLabel = new QLabel(QString(), this);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);

    setLayout(mainLayout);

    /* ---- 信号连接 ---- */
    connect(m_validateBtn, &QPushButton::clicked,
            this, &ProtocolSchemaEditor::validateJson);

    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        ++m_totalSchemasSaved;  ///< 统计: 保存协议
        emit saveRequested(m_jsonEditor->toPlainText());
    });
}

/** @brief 加载协议定义到编辑器(序列化为JSON填入编辑区) @param schema 指向要编辑的ProtocolSchema对象 */
void ProtocolSchemaEditor::loadSchema(ProtocolSchema *schema)
{
    m_schema = schema;
    ++m_totalSchemasLoaded;  ///< 统计: 加载协议

    if (!m_schema) {
        m_statusLabel->setText(tr("无效的协议定义"));
        return;
    }

    if (m_schema->isValid()) {
        const QJsonObject jsonObj = m_schema->toJson();
        const QJsonDocument doc(jsonObj);
        const QString jsonStr = QString::fromUtf8(
            doc.toJson(QJsonDocument::Indented));

        m_jsonEditor->setPlainText(jsonStr);
        m_statusLabel->setText(tr("已加载: %1").arg(m_schema->name()));
    } else {
        m_statusLabel->setText(tr("无效的协议定义"));
    }
}

/** @brief 获取当前编辑的协议定义 @return 当前ProtocolSchema指针，未加载时为nullptr */
ProtocolSchema *ProtocolSchemaEditor::currentSchema() const
{
    return m_schema;
}

/** @brief 验证当前JSON文本是否为合法协议定义(临时ProtocolSchema解析) */
void ProtocolSchemaEditor::validateJson()
{
    ++m_totalValidations;  ///< 统计: 验证操作
    auto *tmpSchema = new ProtocolSchema(this);

    const bool ok = tmpSchema->loadFromJsonData(
        m_jsonEditor->toPlainText().toUtf8());

    if (ok && tmpSchema->isValid()) {
        m_statusLabel->setText(
            tr("✓ 有效协议: %1").arg(tmpSchema->name()));
        m_statusLabel->setProperty("validationState",
                                   QStringLiteral("valid"));
    } else {
        m_statusLabel->setText(
            tr("✗ 错误: %1").arg(tmpSchema->lastError()));
        m_statusLabel->setProperty("validationState",
                                   QStringLiteral("invalid"));
    }

    /* 通知 QSS 刷新属性选择器 */
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);

    delete tmpSchema;
}

/** @brief 设置JSON编辑器文本内容 @param json 要显示的JSON文本 */
void ProtocolSchemaEditor::setJsonText(const QString &json)
{
    m_jsonEditor->setPlainText(json);
}

/** @brief 获取JSON编辑器当前文本内容 @return 编辑器中的JSON文本 */
QString ProtocolSchemaEditor::jsonText() const
{
    return m_jsonEditor->toPlainText();
}

/** @brief 重置协议编辑器统计计数器 */
void ProtocolSchemaEditor::resetSchemaEditorStatistics()
{
    m_totalSchemasLoaded = 0;
    m_totalSchemasSaved = 0;
    m_totalValidations = 0;
}
