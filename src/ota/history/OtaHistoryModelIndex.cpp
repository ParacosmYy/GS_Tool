/**
 * @file OtaHistoryModelIndex.cpp
 * @brief OTA历史记录表格模型 — QAbstractItemModel接口实现
 *
 * 从OtaHistoryModel.cpp拆分，负责:
 *   1. rowCount: 返回记录行数
 *   2. columnCount: 返回列数（固定6列）
 *   3. data: 返回指定单元格数据（显示文本/前景色/工具提示）
 *   4. headerData: 返回水平表头文本
 *
 * 列定义: 时间 | 文件名 | 协议 | 大小 | 耗时 | 结果
 * 特殊渲染: 结果列使用语义色（成功=绿/失败=红），失败行Tooltip显示错误详情
 */

#include "ota/history/OtaHistoryModel.h"
#include "core/theme/ThemeManager.h"

/** @brief 返回记录行数 @param parent 父索引（表格模型中无效） */
int OtaHistoryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_records.size();
}

/** @brief 返回列数（固定为 ColCount=6） @param parent 父索引 */
int OtaHistoryModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return ColCount;
}

/**
 * @brief 返回指定单元格的数据
 *
 * 支持的角色:
 *   - DisplayRole: 格式化显示文本（时间/文件名/协议/大小/耗时/结果）
 *   - ForegroundRole: 结果列使用语义色（成功=Success, 失败=Error）
 *   - ToolTipRole: 失败行显示错误详情
 *
 * @param index 单元格索引 @param role 数据角色 @return 格式化后的数据
 */
QVariant OtaHistoryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_records.size()) return QVariant();

    const OtaRecord& rec = m_records.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColTime:
            return rec.startTime.toString("yyyy-MM-dd HH:mm:ss");
        case ColFileName:
            return rec.fileName;
        case ColProtocol:
            return rec.protocol.toUpper();
        case ColSize: {
            if (rec.fileSize < 1024) return tr("%1 B").arg(rec.fileSize);
            if (rec.fileSize < 1024 * 1024) return tr("%1 KB").arg(rec.fileSize / 1024.0, 0, 'f', 1);
            return tr("%1 MB").arg(rec.fileSize / (1024.0 * 1024.0), 0, 'f', 2);
        }
        case ColDuration:
            if (rec.durationMs < 1000) return tr("%1 ms").arg(rec.durationMs);
            return tr("%1 s").arg(rec.durationMs / 1000.0, 0, 'f', 1);
        case ColResult:
            return rec.success ? tr("成功") : tr("失败");
        default:
            return QVariant();
        }
    }

    if (role == Qt::ForegroundRole) {
        if (index.column() == ColResult) {
            return rec.success
                ? ThemeManager::instance().color(ThemeManager::SemanticColor::Success)
                : ThemeManager::instance().color(ThemeManager::SemanticColor::Error);
        }
    }

    if (role == Qt::ToolTipRole && !rec.success) {
        return rec.errorMessage;
    }

    return QVariant();
}

/**
 * @brief 返回水平表头文本
 * @param section 列号 @param orientation 方向 @param role 数据角色
 * @return 列标题文本（仅支持 Horizontal + DisplayRole）
 */
QVariant OtaHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return QVariant();

    switch (section) {
    case ColTime:     return tr("时间");
    case ColFileName: return tr("文件");
    case ColProtocol: return tr("协议");
    case ColSize:     return tr("大小");
    case ColDuration: return tr("耗时");
    case ColResult:   return tr("结果");
    default:          return QVariant();
    }
}
