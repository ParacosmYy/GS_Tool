/**
 * @file ProtocolFieldEditor.cpp
 * @brief 协议字段结构编辑器 — 字段CRUD/数据操作/JSON持久化/解析/工具方法
 *
 * 核心实现:
 *   - addField/removeField/updateField: 字段增删改
 *   - setData/data: 数据读写
 *   - exportToJson/importFromJson: JSON模板持久化
 *   - interpretField(): 按字节序读取整数/浮点/字符串等
 *   - refreshFieldTable/refreshInterpretation: 表格和解析树刷新
 *
 * UI构建/槽函数/对话框见 ProtocolFieldEditorUI.cpp
 * 统计重置见 ProtocolFieldEditorStats.cpp
 */

#include "protocol/field_editor/ProtocolFieldEditor.h"
#include "core/widgets/EdDialog.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <cstring>

// ============================================================
// 构造 / 析构
// ============================================================

ProtocolFieldEditor::ProtocolFieldEditor(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
}

ProtocolFieldEditor::~ProtocolFieldEditor() = default;

// ============================================================
// 字段操作 API
// ============================================================

int ProtocolFieldEditor::addField(const FieldDef& field)
{
    m_fields.append(field);
    ++m_stats.totalFieldAdds;
    refreshFieldTable();
    refreshInterpretation();
    emit fieldsChanged();
    return m_fields.size() - 1;
}

void ProtocolFieldEditor::removeField(int index)
{
    if (index < 0 || index >= m_fields.size()) return;
    m_fields.removeAt(index);
    ++m_stats.totalFieldRemoves;
    refreshFieldTable();
    refreshInterpretation();
    emit fieldsChanged();
}

void ProtocolFieldEditor::updateField(int index, const FieldDef& field)
{
    if (index < 0 || index >= m_fields.size()) return;
    m_fields[index] = field;
    ++m_stats.totalFieldUpdates;
    refreshFieldTable();
    refreshInterpretation();
    emit fieldsChanged();
}

QList<ProtocolFieldEditor::FieldDef> ProtocolFieldEditor::fields() const
{
    return m_fields;
}

// ============================================================
// 数据操作 API
// ============================================================

void ProtocolFieldEditor::setData(const QByteArray& data)
{
    m_data = data;
    ++m_stats.totalDataUpdates;
    m_dataSizeLabel->setText(tr("数据大小: %1 字节").arg(data.size()));
    refreshInterpretation();
    emit dataChanged(data);
}

QByteArray ProtocolFieldEditor::data() const
{
    return m_data;
}

// ============================================================
// JSON 持久化
// ============================================================

bool ProtocolFieldEditor::exportToJson(const QString& path) const
{
    QJsonObject root;
    root["version"] = 1;
    QJsonArray fieldsArr;
    for (const auto& f : m_fields) {
        QJsonObject obj;
        obj["name"] = f.name;
        obj["type"] = fieldTypeToString(f.type);
        obj["bitOffset"] = f.bitOffset;
        obj["bitWidth"] = f.bitWidth;
        obj["enumName"] = f.enumName;
        fieldsArr.append(obj);
    }
    root["fields"] = fieldsArr;

    QJsonDocument doc(root);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        EdDialog::error(const_cast<ProtocolFieldEditor*>(this),
                        tr("导出失败"), tr("无法写入文件: %1").arg(path));
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    ++const_cast<ProtocolFieldEditor*>(this)->m_stats.totalJsonExports;
    return true;
}

bool ProtocolFieldEditor::importFromJson(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        EdDialog::error(this, tr("导入失败"), tr("无法读取文件: %1").arg(path));
        return false;
    }
    const QByteArray raw = file.readAll();
    file.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError) {
        EdDialog::error(this, tr("导入失败"),
                        tr("JSON解析错误: %1").arg(err.errorString()));
        return false;
    }
    const QJsonObject root = doc.object();
    const QJsonArray fieldsArr = root["fields"].toArray();

    m_fields.clear();
    for (const auto& item : fieldsArr) {
        const QJsonObject obj = item.toObject();
        FieldDef f;
        f.name = obj["name"].toString();
        f.type = fieldTypeFromString(obj["type"].toString());
        f.bitOffset = obj["bitOffset"].toInt(0);
        f.bitWidth = obj["bitWidth"].toInt(8);
        f.enumName = obj["enumName"].toString();
        m_fields.append(f);
    }
    ++m_stats.totalJsonImports;
    refreshFieldTable();
    refreshInterpretation();
    emit fieldsChanged();
    return true;
}

// ============================================================
// 统计
// ============================================================

ProtocolFieldEditor::Stats ProtocolFieldEditor::stats() const
{
    return m_stats;
}

// ---- resetStatistics() 见 ProtocolFieldEditorStats.cpp ----

// ============================================================
// 表格 / 解析树刷新
// ============================================================

void ProtocolFieldEditor::refreshFieldTable()
{
    if (m_updating) return;
    m_updating = true;
    m_fieldTable->setRowCount(static_cast<int>(m_fields.size()));
    for (int i = 0; i < m_fields.size(); ++i) {
        const auto& f = m_fields[i];
        auto* nameItem = new QTableWidgetItem(f.name);
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_fieldTable->setItem(i, 0, nameItem);

        auto* typeItem = new QTableWidgetItem(fieldTypeToString(f.type));
        typeItem->setTextAlignment(Qt::AlignCenter);
        m_fieldTable->setItem(i, 1, typeItem);

        auto* offItem = new QTableWidgetItem(QString::number(f.bitOffset));
        offItem->setTextAlignment(Qt::AlignCenter);
        m_fieldTable->setItem(i, 2, offItem);

        auto* widthItem = new QTableWidgetItem(QString::number(f.bitWidth));
        widthItem->setTextAlignment(Qt::AlignCenter);
        m_fieldTable->setItem(i, 3, widthItem);

        auto* enumItem = new QTableWidgetItem(f.enumName);
        enumItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_fieldTable->setItem(i, 4, enumItem);
    }
    m_updating = false;
}

void ProtocolFieldEditor::refreshInterpretation()
{
    m_interpTree->clear();
    ++m_stats.totalInterpretations;

    for (const auto& f : m_fields) {
        auto* item = new QTreeWidgetItem(m_interpTree);
        item->setText(0, f.name);
        item->setText(1, fieldTypeToString(f.type));
        item->setText(2, tr("%1b").arg(f.bitOffset));

        const QVariant value = interpretField(m_data, f);
        if (value.isValid()) {
            switch (f.type) {
            case FieldType::UInt8: case FieldType::UInt16LE: case FieldType::UInt16BE:
            case FieldType::UInt32LE: case FieldType::UInt32BE:
                item->setText(3, value.toString());
                break;
            case FieldType::Int8:
                item->setText(3, QString::number(value.toInt()));
                break;
            case FieldType::Float32: case FieldType::Float64:
                item->setText(3, QString::number(value.toDouble(), 'g', 6));
                break;
            case FieldType::Bool:
                item->setText(3, value.toBool() ? tr("true") : tr("false"));
                break;
            case FieldType::String:
                item->setText(3, value.toString());
                break;
            case FieldType::Bytes:
                item->setText(3, value.toByteArray().toHex(' ').toUpper());
                break;
            }
        } else {
            item->setText(3, tr("—"));
            ++m_stats.interpretationErrors;
        }

        const int byteOff = f.bitOffset / 8;
        const int byteLen = qMax(1, fieldTypeByteSize(f.type, f.bitWidth));
        if (byteOff >= 0 && byteOff + byteLen <= m_data.size()) {
            item->setText(4, m_data.mid(byteOff, byteLen).toHex(' ').toUpper());
        } else {
            item->setText(4, tr("越界"));
        }
    }
    m_interpTree->resizeColumnToContents(0);
    m_interpTree->resizeColumnToContents(1);
    m_interpTree->resizeColumnToContents(2);
}

// ============================================================
// 静态工具方法
// ============================================================

QString ProtocolFieldEditor::fieldTypeToString(FieldType type)
{
    static const QStringList names = {
        "UInt8", "Int8", "UInt16LE", "UInt16BE",
        "UInt32LE", "UInt32BE", "Float32", "Float64",
        "String", "Bytes", "Bool"
    };
    const int idx = static_cast<int>(type);
    return (idx >= 0 && idx < names.size()) ? names[idx] : "Unknown";
}

ProtocolFieldEditor::FieldType ProtocolFieldEditor::fieldTypeFromString(const QString& str)
{
    static const QHash<QString, FieldType> map = {
        {"UInt8", FieldType::UInt8}, {"Int8", FieldType::Int8},
        {"UInt16LE", FieldType::UInt16LE}, {"UInt16BE", FieldType::UInt16BE},
        {"UInt32LE", FieldType::UInt32LE}, {"UInt32BE", FieldType::UInt32BE},
        {"Float32", FieldType::Float32}, {"Float64", FieldType::Float64},
        {"String", FieldType::String}, {"Bytes", FieldType::Bytes},
        {"Bool", FieldType::Bool}
    };
    return map.value(str, FieldType::UInt8);
}

int ProtocolFieldEditor::fieldTypeByteSize(FieldType type, int bitWidth)
{
    switch (type) {
    case FieldType::UInt8: case FieldType::Int8: case FieldType::Bool:
        return 1;
    case FieldType::UInt16LE: case FieldType::UInt16BE:
        return 2;
    case FieldType::UInt32LE: case FieldType::UInt32BE: case FieldType::Float32:
        return 4;
    case FieldType::Float64:
        return 8;
    case FieldType::String: case FieldType::Bytes:
        return qMax(1, bitWidth / 8);
    }
    return 1;
}

// ============================================================
// 核心解析: interpretField
// ============================================================

QVariant ProtocolFieldEditor::interpretField(const QByteArray& data,
                                             const FieldDef& field)
{
    const int byteOffset = field.bitOffset / 8;
    const int byteLen = fieldTypeByteSize(field.type, field.bitWidth);

    if (byteOffset < 0 || byteOffset + byteLen > data.size())
        return {};

    const char* p = data.constData() + byteOffset;

    switch (field.type) {
    case FieldType::UInt8:
        return static_cast<quint8>(p[0]);
    case FieldType::Int8:
        return static_cast<qint8>(p[0]);
    case FieldType::UInt16LE:
        return static_cast<quint16>(
            (static_cast<quint8>(p[1]) << 8) | static_cast<quint8>(p[0]));
    case FieldType::UInt16BE:
        return static_cast<quint16>(
            (static_cast<quint8>(p[0]) << 8) | static_cast<quint8>(p[1]));
    case FieldType::UInt32LE:
        return static_cast<quint32>(
            (static_cast<quint32>(static_cast<quint8>(p[3])) << 24) |
            (static_cast<quint32>(static_cast<quint8>(p[2])) << 16) |
            (static_cast<quint32>(static_cast<quint8>(p[1])) << 8)  |
             static_cast<quint32>(static_cast<quint8>(p[0])));
    case FieldType::UInt32BE:
        return static_cast<quint32>(
            (static_cast<quint32>(static_cast<quint8>(p[0])) << 24) |
            (static_cast<quint32>(static_cast<quint8>(p[1])) << 16) |
            (static_cast<quint32>(static_cast<quint8>(p[2])) << 8)  |
             static_cast<quint32>(static_cast<quint8>(p[3])));
    case FieldType::Float32: {
        float val = 0.0f;
        std::memcpy(&val, p, 4);
        return static_cast<double>(val);
    }
    case FieldType::Float64: {
        double val = 0.0;
        std::memcpy(&val, p, 8);
        return val;
    }
    case FieldType::String:
        return QString::fromUtf8(data.mid(byteOffset, byteLen));
    case FieldType::Bytes:
        return data.mid(byteOffset, byteLen);
    case FieldType::Bool:
        return ((static_cast<quint8>(p[0]) >> (field.bitOffset % 8)) & 0x01) != 0;
    }
    return {};
}
