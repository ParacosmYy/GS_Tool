/**
 * @file ProtocolSchemaEditor.cpp
 * @brief 协议帧结构 JSON 编辑器实现
 */

#include "protocol/editor/ProtocolSchemaEditor.h"
#include "protocol/schema/ProtocolSchema.h"

#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

/**
 * @brief 构造函数
 *
 * 初始化界面布局：JSON 编辑区、验证按钮和状态标签。
 *
 * @param parent 父控件指针
 */
ProtocolSchemaEditor::ProtocolSchemaEditor(QWidget *parent)
    : QWidget(parent)
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
 * 创建 JSON 文本编辑区、验证按钮和状态提示标签，
 * 使用垂直布局依次排列。
 */
void ProtocolSchemaEditor::setupUI()
{
    auto *layout = new QVBoxLayout(this);

    m_jsonEditor = new QTextEdit(this);
    m_jsonEditor->setPlaceholderText(tr("在此输入协议 JSON 定义..."));
    layout->addWidget(m_jsonEditor);

    m_validateBtn = new QPushButton(tr("验证"), this);
    layout->addWidget(m_validateBtn);

    m_statusLabel = new QLabel(tr("就绪"), this);
    layout->addWidget(m_statusLabel);

    setLayout(layout);
}

/**
 * @brief 加载协议定义到编辑器
 *
 * 将 ProtocolSchema 的内容序列化为 JSON 并显示在编辑区。
 *
 * @param schema 指向要编辑的 ProtocolSchema 对象
 */
void ProtocolSchemaEditor::loadSchema(ProtocolSchema *schema)
{
    m_schema = schema;
    // TODO: 将 schema 序列化为 JSON 并填入 m_jsonEditor
}

/**
 * @brief 获取当前编辑的协议定义
 * @return 当前 ProtocolSchema 指针，未加载时为 nullptr
 */
ProtocolSchema *ProtocolSchemaEditor::currentSchema() const
{
    return m_schema;
}
