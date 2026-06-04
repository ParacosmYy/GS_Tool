/**
 * @file DataMaskEditorIo.cpp
 * @brief 数据掩码编辑器 JSON 导入/导出实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 从 DataMaskEditor.cpp 拆分而来，包含 exportToJson() 和 importFromJson()。
 */

#include "utils/bitmask/DataMaskEditor.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

/**
 * @brief 导出位域定义到JSON文件
 * @param filePath 目标文件路径
 * @return true导出成功
 *
 * JSON 格式: { "bitWidth": int, "maskValue": hexString, "fields": [...] }
 * 每个字段: { "name": string, "startBit": int, "endBit": int, "color": hexString }
 */
bool DataMaskEditor::exportToJson(const QString &filePath)
{
    QJsonArray fieldsArray;
    for (const auto &f : m_fields) {
        QJsonObject obj;
        obj["name"] = f.name;
        obj["startBit"] = f.startBit;
        obj["endBit"] = f.endBit;
        obj["color"] = f.color.name();
        fieldsArray.append(obj);
    }

    QJsonObject root;
    root["bitWidth"] = m_bitWidth;
    root["maskValue"] = QString::number(m_maskValue, 16);
    root["fields"] = fieldsArray;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

/**
 * @brief 从JSON文件导入位域定义
 * @param filePath 源文件路径
 * @return true导入成功
 *
 * 恢复位宽、掩码值和所有字段定义，同步 UI 控件。
 */
bool DataMaskEditor::importFromJson(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    file.close();
    if (err.error != QJsonParseError::NoError) return false;

    const QJsonObject root = doc.object();

    // 恢复位宽
    if (root.contains("bitWidth")) {
        const int bw = root["bitWidth"].toInt();
        setBitWidth(bw);
        // 同步下拉框
        for (int i = 0; i < m_bitWidthCombo->count(); ++i) {
            if (m_bitWidthCombo->itemData(i).toInt() == bw) {
                m_bitWidthCombo->setCurrentIndex(i);
                break;
            }
        }
    }

    // 恢复掩码值
    if (root.contains("maskValue")) {
        bool ok = false;
        const uint64_t mv = root["maskValue"].toString().toULongLong(&ok, 16);
        if (ok) {
            m_maskValue = mv;
            refreshBitDisplay();
            refreshValueDisplay();
        }
    }

    // 恢复字段列表
    m_fields.clear();
    const QJsonArray arr = root["fields"].toArray();
    for (const auto &val : arr) {
        const QJsonObject obj = val.toObject();
        BitField f;
        f.name = obj["name"].toString();
        f.startBit = obj["startBit"].toInt();
        f.endBit = obj["endBit"].toInt();
        f.color = QColor(obj["color"].toString());
        m_fields.append(f);
    }

    m_stats.activeFields = m_fields.size();
    if (m_stats.activeFields > m_stats.peakFields) {
        m_stats.peakFields = m_stats.activeFields;
    }

    colorBitButtons();
    refreshFieldTable();
    return true;
}
