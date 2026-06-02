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

/**
 * @brief 构造函数
 *
 * 初始化界面布局：标题标签、JSON 编辑区、操作按钮行和状态标签。
 *
 * @param parent 父控件指针
 */
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

/**
 * @brief 初始化界面布局
 *
 * 创建并排列所有 UI 元素：
 * - 标题标签（"Protocol Schema Editor"）
 * - JSON 文本编辑区（等宽字体）
 * - 操作按钮行（验证、保存）
 * - 状态提示标签（自动换行）
 */
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
        emit saveRequested(m_jsonEditor->toPlainText());
    });
}

/**
 * @brief 加载协议定义到编辑器
 *
 * 将 ProtocolSchema 序列化为格式化 JSON 文本并填入编辑区。
 * 如果 schema 无效则显示错误提示。
 *
 * @param schema 指向要编辑的 ProtocolSchema 对象
 */
void ProtocolSchemaEditor::loadSchema(ProtocolSchema *schema)
{
    m_schema = schema;

    if (!m_schema) {
        m_statusLabel->setText(tr("Invalid schema"));
        return;
    }

    if (m_schema->isValid()) {
        const QJsonObject jsonObj = m_schema->toJson();
        const QJsonDocument doc(jsonObj);
        const QString jsonStr = QString::fromUtf8(
            doc.toJson(QJsonDocument::Indented));

        m_jsonEditor->setPlainText(jsonStr);
        m_statusLabel->setText(tr("Loaded: %1").arg(m_schema->name()));
    } else {
        m_statusLabel->setText(tr("Invalid schema"));
    }
}

/**
 * @brief 获取当前编辑的协议定义
 * @return 当前 ProtocolSchema 指针，未加载时为 nullptr
 */
ProtocolSchema *ProtocolSchemaEditor::currentSchema() const
{
    return m_schema;
}

/**
 * @brief 验证当前 JSON 文本是否为合法协议定义
 *
 * 创建临时 ProtocolSchema 对象，尝试从编辑区文本解析。
 * 成功时显示协议名称（绿色），失败时显示错误信息（红色）。
 */
void ProtocolSchemaEditor::validateJson()
{
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

/**
 * @brief 设置 JSON 编辑器文本内容
 * @param json 要显示的 JSON 文本
 */
void ProtocolSchemaEditor::setJsonText(const QString &json)
{
    m_jsonEditor->setPlainText(json);
}

/**
 * @brief 获取 JSON 编辑器当前文本内容
 * @return 编辑器中的 JSON 文本
 */
QString ProtocolSchemaEditor::jsonText() const
{
    return m_jsonEditor->toPlainText();
}
