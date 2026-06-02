/**
 * @file DirectionFilter.cpp
 * @brief 终端方向过滤器的实现
 *
 * 增量更新逻辑:
 *   1. 环形缓冲区回绕检测: 模型行数减少 -> 旧数据被驱逐 -> 全量重建索引表
 *   2. 增量追加: 只处理 [m_trackedLineCount, modelLineCount) 范围的新行
 *   3. 方向匹配: 新行的方向 == m_direction 时，将其模型行号追加到索引表
 */

#include "terminal/types/DirectionFilter.h"

/** @brief 构造方向过滤器 @param parent 父对象 */
DirectionFilter::DirectionFilter(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置过滤方向并激活过滤 @param direction TX/RX方向枚举 */
void DirectionFilter::setDirection(DataDirection direction)
{
    m_direction = direction;
    m_filtered = true;
}

/** @brief 获取当前过滤方向 @return DataDirection枚举 */
DataDirection DirectionFilter::direction() const
{
    return m_direction;
}

/** @brief 清除方向过滤(显示所有方向) */
void DirectionFilter::clearFilter()
{
    m_filtered = false;
}

/** @brief 查询是否激活了方向过滤 @return true=已激活过滤 */
bool DirectionFilter::isFiltered() const
{
    return m_filtered;
}

/** @brief 模型数据追加回调：增量构建过滤索引表(回绕检测+方向匹配) @param modelLineCount 当前模型总行数 @param lineAt 按索引取TerminalLine的回调 */
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

/** @brief 获取过滤后的行数 @return 过滤索引表大小 */
int DirectionFilter::filteredLineCount() const
{
    return m_filteredIndices.size();
}

/** @brief 过滤索引→模型索引映射(越界返回-1) @param filteredIndex 过滤后索引 @return 模型行索引 */
int DirectionFilter::modelIndex(int filteredIndex) const
{
    // 越界保护: 返回 -1 表示无效索引，调用方已有 if (modelLine >= 0) 的保护
    if (filteredIndex < 0 || filteredIndex >= m_filteredIndices.size()) {
        return -1;
    }
    return m_filteredIndices[filteredIndex];
}

/** @brief 重置过滤状态(清空索引表和跟踪计数) */
void DirectionFilter::reset()
{
    m_trackedLineCount = 0;
    m_filteredIndices.clear();
}
