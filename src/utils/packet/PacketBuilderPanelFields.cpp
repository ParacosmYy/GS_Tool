/**
 * @file PacketBuilderPanelFields.cpp
 * @brief 数据包构建面板 — 字段管理与统计方法实现
 * @author Serial Tool Team
 * @date 2026-06-04
 *
 * 本文件从 PacketBuilderPanel.cpp 拆分而来，集中管理：
 * - 字段增删 / 上下移动 / 全部清除
 * - 表格刷新 / hex dump 格式化
 * - 统计计数器重置
 */

#include "utils/packet/PacketBuilderPanel.h"

#include <QHeaderView>

/**
 * @brief 添加新字段行
 *
 * 创建一个默认 uint8 字段并追加到构建器末尾，
 * 同时刷新表格显示。
 */
void PacketBuilderPanel::onAddField()
{
    if (!m_builder) { return; }

    ++m_totalFieldEdits;
    PacketField field;
    field.name = tr("字段%1").arg(m_builder->fields().size() + 1);
    field.offset = 0;
    field.size = 1;
    field.dataType = QStringLiteral("uint8");
    field.value = 0;

    m_builder->addField(field);
    refreshTable();
}

/**
 * @brief 删除选中行对应的字段
 *
 * 根据当前选中行索引移除对应字段，并刷新表格。
 */
void PacketBuilderPanel::onRemoveField()
{
    if (!m_builder) { return; }

    int row = m_fieldTable->currentRow();
    if (row >= 0) {
        ++m_totalFieldEdits;
        m_builder->removeField(row);
        refreshTable();
    }
}

/**
 * @brief 刷新表格以同步构建器字段
 *
 * 从构建器读取当前字段列表，逐行填充名称、偏移、大小、类型、值。
 */
void PacketBuilderPanel::refreshTable()
{
    if (!m_builder) { return; }

    auto fields = m_builder->fields();
    m_fieldTable->setRowCount(fields.size());

    for (int i = 0; i < fields.size(); ++i) {
        const auto &f = fields[i];
        m_fieldTable->setItem(i, 0, new QTableWidgetItem(f.name));
        m_fieldTable->setItem(i, 1, new QTableWidgetItem(
            QString::number(f.offset)));
        m_fieldTable->setItem(i, 2, new QTableWidgetItem(
            QString::number(f.size)));
        m_fieldTable->setItem(i, 3, new QTableWidgetItem(f.dataType));
        m_fieldTable->setItem(i, 4, new QTableWidgetItem(
            f.value.toString()));
    }
}

/**
 * @brief 格式化hex dump输出
 *
 * 将原始字节按每16字节换行，以大写十六进制形式展示，
 * 末尾附带总字节数。
 *
 * @param data 原始数据
 * @return 格式化的hex dump字符串
 */
QString PacketBuilderPanel::formatHexDump(const QByteArray &data) const
{
    if (data.isEmpty()) { return tr("(空数据包)"); }

    QString result;
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0) {
            result += (i % 16 == 0) ? "\n" : " ";
        }
        result += QString("%1").arg(
            static_cast<quint8>(data[i]), 2, 16, QChar('0')).toUpper();
    }

    result += tr("\n\n%1 字节").arg(data.size());
    return result;
}

/**
 * @brief 清除所有字段
 *
 * 从后向前逐个移除字段，清空预览区。
 */
void PacketBuilderPanel::onClearAll()
{
    if (!m_builder) { return; }

    auto fields = m_builder->fields();
    for (int i = fields.size() - 1; i >= 0; --i) {
        m_builder->removeField(i);
    }
    refreshTable();
    m_hexPreview->clear();
}

/**
 * @brief 上移选中字段
 *
 * 将当前选中行与上一行交换，通过移除后重新添加重建字段顺序，
 * 完成后选中上移后的行。
 */
void PacketBuilderPanel::onMoveUp()
{
    if (!m_builder) { return; }

    int row = m_fieldTable->currentRow();
    if (row <= 0) { return; }

    auto fields = m_builder->fields();
    std::swap(fields[row], fields[row - 1]);

    /* 重建字段列表 */
    for (int i = fields.size() - 1; i >= 0; --i) {
        m_builder->removeField(i);
    }
    for (auto& f : fields) {
        m_builder->addField(f);
    }

    refreshTable();
    m_fieldTable->selectRow(row - 1);
}

/**
 * @brief 下移选中字段
 *
 * 将当前选中行与下一行交换，通过移除后重新添加重建字段顺序，
 * 完成后选中下移后的行。
 */
void PacketBuilderPanel::onMoveDown()
{
    if (!m_builder) { return; }

    int row = m_fieldTable->currentRow();
    auto fields = m_builder->fields();
    if (row < 0 || row >= fields.size() - 1) { return; }

    std::swap(fields[row], fields[row + 1]);

    /* 重建字段列表 */
    for (int i = fields.size() - 1; i >= 0; --i) {
        m_builder->removeField(i);
    }
    for (auto& f : fields) {
        m_builder->addField(f);
    }

    refreshTable();
    m_fieldTable->selectRow(row + 1);
}

/**
 * @brief 重置所有统计计数器
 *
 * 将构建次数、发送次数、字段编辑次数、模板加载次数全部归零。
 */
void PacketBuilderPanel::resetStatistics()
{
    m_totalPacketsBuilt = 0;
    m_totalSends = 0;
    m_totalFieldEdits = 0;
    m_totalTemplateLoads = 0;
}
