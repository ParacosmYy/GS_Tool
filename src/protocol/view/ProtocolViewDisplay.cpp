/**
 * @file ProtocolViewDisplay.cpp
 * @brief ProtocolView 显示与渲染辅助方法
 *
 * 从 ProtocolView.cpp 拆分而来，包含:
 *   - 动态列头管理: 新字段自动追加表格列
 *   - 数值着色逻辑: 根据字段值范围设置正常/警告/错误前景色
 *   - 列宽自适应: 按内容调整数据列宽度并限制上下界
 */

#include "protocol/view/ProtocolView.h"
#include "core/theme/ThemeManager.h"

// ============================================================
// 动态列管理
// ============================================================

/** @brief 动态更新列头(新字段自动追加列，忽略下划线内部字段) @param fields 当前帧的字段映射 */
void ProtocolView::updateColumnHeaders(const QVariantMap& fields)
{
    for (auto it = fields.constBegin(); it != fields.constEnd(); ++it) {
        if (it.key().startsWith('_')) continue;
        if (!m_fieldNames.contains(it.key())) {
            m_fieldNames.append(it.key());
            int col = kFixedColumns + m_fieldNames.size() - 1;
            m_model->setHorizontalHeaderItem(col, new QStandardItem(it.key()));
            m_table->setColumnWidth(col, 100);
        }
    }
}

// ============================================================
// 着色逻辑
// ============================================================

/** @brief 根据字段值创建着色的QStandardItem(正常绿/警告黄/错误红) @param fieldName 字段名 @param value 字段值字符串 @return 带前景色的QStandardItem */
QStandardItem* ProtocolView::createColoredItem(const QString& fieldName,
                                                const QString& value)
{
    auto* item = new QStandardItem(value);
    auto it = m_colorRanges.find(fieldName);
    if (it == m_colorRanges.end()) return item;

    bool ok = false;
    double numVal = value.toDouble(&ok);
    if (!ok) return item;

    ++m_totalColorRangeMatches;  ///< 累计数值着色匹配次数
    const FieldColorRange& range = it.value();
    QColor warnColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Warning);
    QColor errColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Error);

    if (numVal >= range.normalLow && numVal <= range.normalHigh) {
        // 正常区间: 默认前景
    } else if ((numVal >= range.warnLow && numVal < range.normalLow) ||
               (numVal > range.normalHigh && numVal <= range.warnHigh)) {
        item->setForeground(warnColor);  // 警告区间
    } else {
        item->setForeground(errColor);   // 错误区间(超出警告范围)
    }
    return item;
}

/** @brief 自动调整所有列宽(序号50px/时间100px/数据列60~200px自适应) */
void ProtocolView::autoResizeColumns()
{
    m_table->setColumnWidth(0, 50);
    m_table->setColumnWidth(1, 100);
    for (int i = 0; i < m_fieldNames.size(); ++i) {
        int col = kFixedColumns + i;
        m_table->resizeColumnToContents(col);
        int w = m_table->columnWidth(col);
        if (w > 200) m_table->setColumnWidth(col, 200);
        else if (w < 60) m_table->setColumnWidth(col, 60);
    }
}
