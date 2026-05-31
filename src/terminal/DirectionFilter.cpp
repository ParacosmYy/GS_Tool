/**
 * @file DirectionFilter.cpp
 * @brief 终端方向过滤器的实现
 *
 * 增量更新逻辑:
 *   1. 环形缓冲区回绕检测: 模型行数减少 -> 旧数据被驱逐 -> 全量重建索引表
 *   2. 增量追加: 只处理 [m_trackedLineCount, modelLineCount) 范围的新行
 *   3. 方向匹配: 新行的方向 == m_direction 时，将其模型行号追加到索引表
 */

#include "DirectionFilter.h"

DirectionFilter::DirectionFilter(QObject* parent)
    : QObject(parent)
{
}

void DirectionFilter::setDirection(DataDirection direction)
{
    m_direction = direction;
    m_filtered = true;
}

DataDirection DirectionFilter::direction() const
{
    return m_direction;
}

void DirectionFilter::clearFilter()
{
    m_filtered = false;
}

bool DirectionFilter::isFiltered() const
{
    return m_filtered;
}

void DirectionFilter::onDataAppended(int modelLineCount,
                                     const std::function<TerminalLine(int)>& lineAt)
{
    if (!m_filtered) return;

    // 环形缓冲区回绕检测: 模型行数减少说明旧数据被驱逐，需全量重建
    if (m_trackedLineCount > modelLineCount) {
        m_trackedLineCount = 0;
        m_filteredIndices.clear();
    }

    // 增量构建: 只处理新增的模型行
    for (int i = m_trackedLineCount; i < modelLineCount; ++i) {
        TerminalLine line = lineAt(i);
        // 只将匹配过滤方向的行号加入索引表
        if (line.direction == m_direction) {
            m_filteredIndices.append(i);
        }
    }
    m_trackedLineCount = modelLineCount;
}

int DirectionFilter::filteredLineCount() const
{
    return m_filteredIndices.size();
}

int DirectionFilter::modelIndex(int filteredIndex) const
{
    // 越界保护: 返回 -1 表示无效索引，调用方已有 if (modelLine >= 0) 的保护
    if (filteredIndex < 0 || filteredIndex >= m_filteredIndices.size()) {
        return -1;
    }
    return m_filteredIndices[filteredIndex];
}

void DirectionFilter::reset()
{
    m_trackedLineCount = 0;
    m_filteredIndices.clear();
}
