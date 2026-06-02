/**
 * @file ProtocolSchemaEditor.h
 * @brief 协议帧结构 JSON 编辑器
 *
 * 提供可视化 JSON 编辑界面，用于创建和修改 ProtocolSchema。
 * 包含 JSON 文本编辑区、验证按钮和状态提示标签。
 */

#ifndef PROTOCOL_SCHEMA_EDITOR_H
#define PROTOCOL_SCHEMA_EDITOR_H

#include <QWidget>

class QTextEdit;
class QPushButton;
class QLabel;
class ProtocolSchema;

/**
 * @class ProtocolSchemaEditor
 * @brief 协议帧结构编辑器控件
 *
 * 以 JSON 文本形式展示和编辑协议帧结构定义，
 * 提供实时校验功能，修改后发射 schemaChanged 信号。
 */
class ProtocolSchemaEditor : public QWidget
{
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit ProtocolSchemaEditor(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~ProtocolSchemaEditor() override;

    /**
     * @brief 加载协议定义到编辑器
     * @param schema 指向要编辑的 ProtocolSchema 对象
     */
    void loadSchema(ProtocolSchema *schema);

    /**
     * @brief 获取当前编辑的协议定义
     * @return 当前 ProtocolSchema 指针，未加载时为 nullptr
     */
    ProtocolSchema *currentSchema() const;

signals:
    /**
     * @brief 协议定义被修改信号
     */
    void schemaChanged();

    /**
     * @brief 用户请求保存信号
     * @param filePath 保存文件路径
     */
    void saveRequested(const QString &filePath);

private:
    /** @brief 初始化界面布局 */
    void setupUI();

    QTextEdit *m_jsonEditor;      ///< JSON 文本编辑区
    QPushButton *m_validateBtn;   ///< 验证按钮
    QLabel *m_statusLabel;        ///< 状态提示标签
    ProtocolSchema *m_schema;     ///< 当前编辑的协议定义
};

#endif // PROTOCOL_SCHEMA_EDITOR_H
