/**
 * @file ProtocolFieldEditor.h
 * @brief 协议字段结构编辑器 — 可视化定义协议包字段结构并实时解析数据
 *
 * 字段增删改(名称/类型/位偏移/位宽/枚举映射) + 实时解析 + JSON导入导出
 * 外部通过 setData() 输入字节，编辑器按字段定义解释并展示结果。
 * 设计模式: 观察者模式(Qt信号/槽)
 */
#ifndef PROTOCOL_FIELD_EDITOR_H
#define PROTOCOL_FIELD_EDITOR_H

#include <QWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QByteArray>
#include <QList>
#include <QString>
#include <QVariant>

/** @brief 协议字段结构编辑器 — QTableWidget编辑字段, QTreeWidget展示解析结果, 支持位级偏移 */
class ProtocolFieldEditor : public QWidget {
    Q_OBJECT

public:
    /** @brief 字段数据类型 — 覆盖嵌入式协议常用原始类型 */
    enum class FieldType {
        UInt8, Int8, UInt16LE, UInt16BE, UInt32LE, UInt32BE,
        Float32, Float64, String, Bytes, Bool
    };
    Q_ENUM(FieldType)

    /** @brief 单个字段定义 — 名称/类型/位偏移/位宽/枚举映射 */
    struct FieldDef {
        QString name;       ///< 字段名称
        FieldType type;     ///< 数据类型
        int bitOffset;      ///< 位偏移(从报文起始)
        int bitWidth;       ///< 位宽(bit)
        QString enumName;   ///< 枚举映射名(可选)
        FieldDef() : type(FieldType::UInt8), bitOffset(0), bitWidth(8) {}
        FieldDef(const QString& n, FieldType t, int off, int w, const QString& en = {})
            : name(n), type(t), bitOffset(off), bitWidth(w), enumName(en) {}
    };

    /** @brief 运行统计 — 操作计数供诊断 */
    struct Stats {
        quint64 totalFieldAdds = 0, totalFieldRemoves = 0, totalFieldUpdates = 0;
        quint64 totalDataUpdates = 0, totalJsonImports = 0, totalJsonExports = 0;
        quint64 totalInterpretations = 0, interpretationErrors = 0;
    };

    explicit ProtocolFieldEditor(QWidget* parent = nullptr); ///< 构造
    ~ProtocolFieldEditor() override;                         ///< 析构
    ProtocolFieldEditor(const ProtocolFieldEditor&) = delete;
    ProtocolFieldEditor& operator=(const ProtocolFieldEditor&) = delete;

    // ---- 字段 API ----
    int addField(const FieldDef& field);             ///< 追加字段, 返回索引
    void removeField(int index);                     ///< 移除字段
    void updateField(int index, const FieldDef& f);  ///< 更新字段
    QList<FieldDef> fields() const;                  ///< 获取全部字段

    // ---- 数据 API ----
    void setData(const QByteArray& data);            ///< 设置待解析数据
    QByteArray data() const;                         ///< 获取当前数据

    // ---- JSON 持久化 ----
    bool exportToJson(const QString& path) const;    ///< 导出字段定义到JSON
    bool importFromJson(const QString& path);        ///< 从JSON导入字段定义

    // ---- 统计 ----
    Stats stats() const;                             ///< 获取统计
    void resetStatistics();                          ///< 重置统计(见Stats.cpp)

signals:
    void fieldsChanged();                          ///< 字段增/删/改
    void dataChanged(const QByteArray& data);      ///< 数据更新

private slots:
    void onAddFieldClicked();       ///< 添加字段按钮
    void onRemoveFieldClicked();    ///< 删除字段按钮
    void onEditFieldClicked();      ///< 编辑字段按钮
    void onFieldSelectionChanged(); ///< 选中行变更
    void onFieldDoubleClicked(int row, int col); ///< 双击编辑

private:
    void setupUI();                 ///< 构建UI控件和布局
    void setupConnections();        ///< 连接信号/槽
    void refreshFieldTable();       ///< 刷新字段表格
    void refreshInterpretation();   ///< 刷新解析结果树

    /** @brief 字段编辑对话框(新增/编辑) @return 用户编辑后的字段定义 */
    FieldDef showFieldDialog(const FieldDef& field, const QString& title, bool* ok);

    static QString fieldTypeToString(FieldType type);   ///< 类型枚举→字符串
    static FieldType fieldTypeFromString(const QString& str); ///< 字符串→类型枚举
    static int fieldTypeByteSize(FieldType type, int bitWidth); ///< 类型占用字节数

    /** @brief 从数据中按字段定义提取值 @return 解析值(无效=越界/失败) */
    static QVariant interpretField(const QByteArray& data, const FieldDef& field);

    QTableWidget* m_fieldTable = nullptr;  ///< 字段定义表格
    QTreeWidget* m_interpTree = nullptr;   ///< 解析结果树
    QPushButton* m_addBtn = nullptr;       ///< 添加字段按钮
    QPushButton* m_removeBtn = nullptr;    ///< 删除字段按钮
    QPushButton* m_editBtn = nullptr;      ///< 编辑字段按钮
    QPushButton* m_importBtn = nullptr;    ///< JSON导入按钮
    QPushButton* m_exportBtn = nullptr;    ///< JSON导出按钮
    QLabel* m_dataSizeLabel = nullptr;     ///< 数据大小标签

    QList<FieldDef> m_fields;              ///< 字段定义列表
    QByteArray m_data;                     ///< 待解析原始数据
    bool m_updating = false;               ///< 防循环更新标志
    Stats m_stats;                         ///< 运行统计
    static constexpr int kDefaultBitWidth = 8; ///< 默认位宽
};

#endif // PROTOCOL_FIELD_EDITOR_H
