/**
 * @file RegisterMapModel.cpp
 * @brief 寄存器地图表格模型实现 -- 核心数据操作与 Model/View 接口
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/regmap/RegisterMapModel.h"

#include <QColor>
#include <QBrush>

// ─────────────────────────── 构造 ───────────────────────────

RegisterMapModel::RegisterMapModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

// ─────────────────────────── 数据加载 ───────────────────────

void RegisterMapModel::loadRegisterMap(const RegisterMap &map)
{
    beginResetModel();
    m_map = map;
    m_currentValues.resize(map.registers.size());
    for (int i = 0; i < map.registers.size(); ++i) {
        m_currentValues[i] = map.registers[i].resetValue;
    }
    rebuildFilteredIndices();
    endResetModel();

    m_stats.totalLoads++;
    m_stats.activeRegisterCount = map.registers.size();
    if (map.registers.size() > m_stats.peakRegisterCount) {
        m_stats.peakRegisterCount = map.registers.size();
    }
    emit registerMapLoaded(map.registers.size());
}

const RegisterMap &RegisterMapModel::registerMap() const
{
    return m_map;
}

RegisterEntry RegisterMapModel::registerAt(int row) const
{
    if (row < 0 || row >= m_filteredRows.size()) {
        return {};
    }
    return m_map.registers[m_filteredRows[row]];
}

quint64 RegisterMapModel::valueAt(int row) const
{
    if (row < 0 || row >= m_filteredRows.size()) {
        return 0;
    }
    return m_currentValues[m_filteredRows[row]];
}

void RegisterMapModel::setValueAt(int row, quint64 val)
{
    if (row < 0 || row >= m_filteredRows.size()) {
        return;
    }
    int srcRow = m_filteredRows[row];
    m_currentValues[srcRow] = val;
    QModelIndex idx = index(row, ColValue);
    emit dataChanged(idx, idx, {Qt::DisplayRole, Qt::EditRole});
    m_stats.totalEdits++;
    emit registerValueChanged(row, val);
}

// ─────────────────────────── 过滤 ───────────────────────────

void RegisterMapModel::setFilter(const QString &keyword)
{
    m_filterKeyword = keyword.trimmed().toLower();
    beginResetModel();
    rebuildFilteredIndices();
    endResetModel();
    m_stats.totalSearches++;
}

void RegisterMapModel::clearFilter()
{
    m_filterKeyword.clear();
    beginResetModel();
    rebuildFilteredIndices();
    endResetModel();
}

void RegisterMapModel::rebuildFilteredIndices()
{
    m_filteredRows.clear();
    for (int i = 0; i < m_map.registers.size(); ++i) {
        const auto &reg = m_map.registers[i];
        if (m_filterKeyword.isEmpty()) {
            m_filteredRows.append(i);
            continue;
        }
        // 匹配名称、地址 hex、分组
        bool match = reg.name.toLower().contains(m_filterKeyword)
                     || QString::number(reg.address, 16).toLower()
                            .contains(m_filterKeyword)
                     || reg.groupName.toLower().contains(m_filterKeyword);
        if (match) {
            m_filteredRows.append(i);
        }
    }
}

// ─────────────────────── QAbstractTableModel ────────────────

int RegisterMapModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_filteredRows.size();
}

int RegisterMapModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColCount;
}

QVariant RegisterMapModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_filteredRows.size()) {
        return {};
    }

    int srcRow = m_filteredRows[index.row()];
    const RegisterEntry &reg = m_map.registers[srcRow];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ColAddress:
            return QStringLiteral("0x%1")
                .arg(static_cast<uint>(reg.address), 0, 16)
                .toUpper();
        case ColName:
            return reg.name;
        case ColValue:
            return QStringLiteral("0x%1")
                .arg(m_currentValues[srcRow], 0, 16)
                .toUpper();
        case ColReset:
            return QStringLiteral("0x%1")
                .arg(reg.resetValue, 0, 16)
                .toUpper();
        case ColGroup:
            return reg.groupName;
        default:
            break;
        }
    }

    // 对齐：地址和值靠右对齐
    if (role == Qt::TextAlignmentRole) {
        if (index.column() == ColAddress || index.column() == ColValue
            || index.column() == ColReset) {
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        }
        return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    }

    // 工具提示：位域描述
    if (role == Qt::ToolTipRole && index.column() == ColName) {
        if (!reg.fields.isEmpty()) {
            QString tip;
            for (const auto &f : reg.fields) {
                tip += QStringLiteral("%1 [%2:%3] = 0x%4")
                           .arg(f.name)
                           .arg(f.bitPos)
                           .arg(f.bitPos + f.bitWidth - 1)
                           .arg(f.resetValue, 0, 16);
                if (!f.description.isEmpty()) {
                    tip += QStringLiteral(" -- %1").arg(f.description);
                }
                tip += QLatin1Char('\n');
            }
            return tip.trimmed();
        }
    }

    return {};
}

QVariant RegisterMapModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case ColAddress:
            return tr("Address");
        case ColName:
            return tr("Name");
        case ColValue:
            return tr("Value");
        case ColReset:
            return tr("Reset");
        case ColGroup:
            return tr("Group");
        default:
            return {};
        }
    }
    return section + 1; // 行号
}

Qt::ItemFlags RegisterMapModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.isValid() && index.column() == ColValue) {
        f |= Qt::ItemIsEditable;
    }
    return f;
}

bool RegisterMapModel::setData(const QModelIndex &index, const QVariant &value,
                               int role)
{
    if (!index.isValid() || role != Qt::EditRole
        || index.column() != ColValue) {
        return false;
    }
    if (index.row() >= m_filteredRows.size()) {
        return false;
    }

    bool ok = false;
    QString text = value.toString().trimmed();
    quint64 newVal = 0;

    // 支持 0x 前缀十六进制和纯十进制
    if (text.startsWith(QLatin1String("0x"), Qt::CaseInsensitive)) {
        newVal = text.toULongLong(&ok, 16);
    } else {
        newVal = text.toULongLong(&ok, 10);
    }

    if (!ok) {
        return false;
    }

    int srcRow = m_filteredRows[index.row()];
    m_currentValues[srcRow] = newVal;
    m_stats.totalEdits++;
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    emit registerValueChanged(index.row(), newVal);
    return true;
}
